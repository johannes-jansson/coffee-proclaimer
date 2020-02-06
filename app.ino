#include "math.h"
#define READ_PIN A5
#define GROUND_PIN A4
#define LED_PIN D7

#define NBR_OF_READINGS 500
#define READING_DELAY 1
#define LOOP_DELAY 2000

#define UPPER_TRESHOLD 1000
#define MIDDLE_TRESHOLD 200
#define LOWER_TRESHOLD 30

#define K1 0.03650164
#define M1 -0.49918924
#define K2 11.6600
#define M2 34

#define AUTO_TURNOFF_TIME 29 * 60 * 1000

SYSTEM_MODE(SEMI_AUTOMATIC);

int state = 0;
int positiveReadings[NBR_OF_READINGS];
unsigned long timer;
unsigned long boilerTimer;
int elapsed;
float cups = -1;
int nbrOfCups = 0;
int dripDelay = 0;

void setup() {
  pinMode(READ_PIN, INPUT);
  pinMode(GROUND_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  WiFi.on();
  WiFi.connect();
  Particle.connect();
  Particle.function("setState", setState);
  Particle.function("getCups", getCups);
  Particle.function("setCups", setCups);
}

void loop() {
  int reading = readMany("");

  // check if boiling has started
  if (state == 0 && reading > UPPER_TRESHOLD) {
    state = 1;
    timer = millis();
    started();
    return;
  }

  // check if boiling has finished and heating has started
  if (state == 1 && reading < MIDDLE_TRESHOLD && reading > LOWER_TRESHOLD) {
    state = 2;
    boilerTimer = millis();
    Particle.publish("tsBoilTimer", String(boilerTimer - timer), PUBLIC);
    cups = K1 * (boilerTimer - timer) / 1000.0 + M1;
    dripDelay = (K2 * cups + M2) * 1000;
    return;
  }

  // check if the coffee is ready (based on time since boiler done)
  if (
    state == 2 &&
    reading < MIDDLE_TRESHOLD && reading > LOWER_TRESHOLD &&
    millis() - boilerTimer > dripDelay
  ) {
    state = 3;
    done();
  }

  // check if heat pad is turned off
  if (state != 0 && reading < LOWER_TRESHOLD) {
    state = 0;
    elapsed = millis() - timer;
    finished(elapsed);
    return;
  }

  // Make sure we have wifi connection
  if (!Particle.connected()) Particle.connect();
  if (!WiFi.ready()) {
    WiFi.on();
    WiFi.connect();
  }

  // If it's 3 AM and the device has been running for more than 23 hours
  if (Time.hour() == 3 && millis() > 23 * 60 * 60 * 1000) {
    Particle.publish("dev_slack", Time.timeStr() + ", I'm resetting", PUBLIC);
    System.reset();
  }

  delay(LOOP_DELAY);
}

// Do NBR_OF_READINGS readings, return RMS value 🔬
int readMany(String command) {
  digitalWrite(LED_PIN, HIGH);

  // make reads as ints and store them away because it's fast ⚡️
  // subtract virtual ground to keep things smooth and accurate 😎
  for (int i = 0; i < NBR_OF_READINGS; i++) {
    positiveReadings[i] = analogRead(READ_PIN) - analogRead(GROUND_PIN);
    delay(READING_DELAY);
  }

  // Calculate root mean squared value 📈
  float sumSquared = 0;
  for (int i = 0; i < NBR_OF_READINGS; i++) {
    sumSquared = sumSquared + pow(positiveReadings[i], 2);
  }

  // cast it as an int because it's convenient to handle in the particle cloud,
  // and because it feels cheesy to store int readings in a float just because
  // we did some fancy math on it 🧀
  int rms = sqrt(sumSquared / NBR_OF_READINGS);

  digitalWrite(LED_PIN, LOW);
  return rms;
}

/* cloud functions -------------------- */

int setState(String newState) {
  state = newState.toInt();
  return state;
}

int getCups(String s) {
  return nbrOfCups;
}

int setCups(String newCups) {
  nbrOfCups = newCups.toInt();
  return nbrOfCups;
}

/* event handlers -------------------- */

void started() {
  bool first = nbrOfCups == 0;
  String outstring = "";
  if (Time.hour() < 8 && first) outstring += "Good morning <@UJ67H58GG|gordon>!\n";
  else if (Time.hour() < 10 && first) outstring += "Good morning!\n";
  outstring += "Brain juice is on it's way! Sit tight! :rocket:";

  Particle.publish("slack", outstring, PUBLIC);
  return;
}

void done() {
  bool first = nbrOfCups == 0;
  nbrOfCups += (int) round(cups);
  String outstring = "";

  // Always present number of coffee cups served
  outstring += String((int) round(cups)) + " cups of brain juice served! ";

  // Random emoji means awesome emoji!
  int r = random(50);
  if (Time.weekday() == 6) outstring += ":coffee_parrot:";
  else if (r == 0) outstring += ":aw_yeah:";
  else if (r == 1) outstring += ":carlton:";
  else if (r == 2) outstring += ":thumbsup_all:";
  else if (r == 3) outstring += ":the_horns:";
  else if (r == 4) outstring += ":raised_hands:";
  else outstring += ":coffee:";
  
  // Give the people what the people want: fun stats!
  outstring += "\nThat's " + String(nbrOfCups) + " in total today.";

  // Optional comment based on coffee amount
  if (nbrOfCups > 60 && random(2) == 0) {
    outstring += " Pretty impressive!";
  } else if (nbrOfCups < 21 && !first && Time.hour() > 10 && random(2) == 0) {
    outstring += " Those are rookie numbers!";
  }

  Particle.publish("slack", outstring, PUBLIC);
  Particle.publish("dev_slack", outstring, PUBLIC);
  return;
}

void finished(int elapsed) {
  if (elapsed >= AUTO_TURNOFF_TIME) {
    Particle.publish("slack", "The brain juice is getting cold, hurry! :snowflake:", PUBLIC);
  } else {
    Particle.publish("slack", "No more brain juice :frowning:", PUBLIC);
  }
  return;
}
