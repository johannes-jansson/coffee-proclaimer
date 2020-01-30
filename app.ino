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
#define K2 34000
#define M2 11600

#define AUTO_TURNOFF_TIME 29 * 60 * 1000
#define BOIL_DELAY 300 * 1000
#define DRIP_DELAY 150 * 1000

#define THINGSPEAK HIGH

int state = 0;
int positiveReadings[NBR_OF_READINGS];
unsigned long timer;
unsigned long boiler_timer;
int elapsed;
float cups = -1;

void setup() {
  pinMode(READ_PIN, INPUT);
  pinMode(GROUND_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Particle.function("setState", setState);
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
    boiler_timer = millis();
    Particle.publish("tsBoilTimer", String(boiler_timer - timer), PUBLIC);
    cups = K1 * (boiler_timer - timer) / 1000.0 + M1;
    return;
  }

  // check if the coffee is ready (based on time since boiler done)
  if (
    state == 2 &&
    reading < MIDDLE_TRESHOLD && reading > LOWER_TRESHOLD &&
    millis() - boiler_timer > DRIP_DELAY
  ) {
    state = 3;
    done();
  }

  // check if heat pad is turned off
  if (state != 0 && reading < LOWER_TRESHOLD) {
    state = 0;
    elapsed = millis() - timer;
    Particle.publish("tsHeatTimer", String(elapsed), PUBLIC);
    finished(elapsed);
    return;
  }

  // If it's 3 AM and the device has been running for more than 23 hours
  if (Time.hour() == 3 && millis() > 23 * 60 * 60 * 1000) {
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

int setState(String newState) {
  state = newState.toInt();
  return state;
}

/* event handlers -------------------- */

void started() {
  Particle.publish("started", "Coffee is on it's way! Sit tight! :rocket:", PUBLIC);
  return;
}

void done() {
  Particle.publish("done", String((int) round(cups)) + " cups of coffee served! :coffee:", PUBLIC);
  return;
}

void finished(int elapsed) {
  if (elapsed >= AUTO_TURNOFF_TIME) {
    Particle.publish("finished", "The coffee is getting cold, hurry! :snowflake:", PUBLIC);
  } else {
    Particle.publish("finished", "No more coffee :frowning:", PUBLIC);
  }
  return;
}
