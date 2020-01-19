#include "math.h"
#define READ_PIN A0
#define GROUND_PIN A1
#define LED_PIN D7

#define NBR_OF_READINGS 500
#define READING_DELAY 1
#define LOOP_DELAY 4000

#define UPPER_TRESHOLD 1000
#define MIDDLE_TRESHOLD 500
#define LOWER_TRESHOLD 50

#define AUTO_TURNOFF_TIME 30 * 60 * 1000

#define THINGSPEAK HIGH

SYSTEM_MODE(SEMI_AUTOMATIC);

int state = 0;
int positiveReadings[NBR_OF_READINGS];
int maxReading;
unsigned long timer;
int elapsed;


void setup() {
  pinMode(READ_PIN, INPUT);
  pinMode(GROUND_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  WiFi.on();
  /* WiFi.setCredentials("ssid", "password"); */
  Particle.connect();

  Particle.variable("max", maxReading);
}

void loop() {
  int reading = readMany("");
  if (THINGSPEAK) {
    Particle.publish("tsReading", String(reading));
  }

  // see if it started yet
  if (state == 0 && reading > UPPER_TRESHOLD) {
    // Boiling has started! Do nothing
    state = 1;
    timer = millis();
    started();
    return;
  }

  // see if it finished boiling
  if (state == 1 && reading < MIDDLE_TRESHOLD && reading > LOWER_TRESHOLD) {
    // Boiling is done, so coffee is (almost) done
    state = 2;
    elapsed = millis() - timer;

    // TODO: how many cups? based on time stored in elapse
    // TODO: maybe add some kind of delay here
    done();
    return;
  }

  // see if heat pad is turned off yet
  if (state != 0 && reading < LOWER_TRESHOLD) {
    // The coffee brewer was turned off, no more coffee
    elapsed = millis() - timer;
    timer = millis();

    // was it turned off manually or automatically? based on time
    if (elapsed <= AUTO_TURNOFF_TIME) {
      finished(false);
    } else {
      finished(true);
    }
    return;
  }

  delay(LOOP_DELAY);
}

// Do NBR_OF_READINGS readings, return RMS value
int readMany(String command) {
  digitalWrite(LED_PIN, HIGH);

  // make reads as ints and store them away because it's fast ⚡️
  // subtract virtual ground to keep things smooth and accurate 😎
  for (int i = 0; i < NBR_OF_READINGS; i++) {
    positiveReadings[i] = analogRead(READ_PIN) - analogRead(GROUND_PIN);
    delay(READING_DELAY);
  }

  // Calculate root mean squared value 📈
  // Also store the maximum value, used for fine tuning shunt resistor value
  float sumSquared = 0;
  for (int i = 0; i < NBR_OF_READINGS; i++) {
    if (positiveReadings[i] > maxReading) {
      maxReading = positiveReadings[i];
    }
    sumSquared = sumSquared + pow(positiveReadings[i], 2);
  }

  // cast it as an int because it's convenient to handle in the particle cloud,
  // and because it feels cheesy to store int readings in a float just because
  // we did some fancy math on it 🧀
  int rms = sqrt(sumSquared / NBR_OF_READINGS);

  digitalWrite(LED_PIN, LOW);
  return rms;
}

/* event handlers -------------------- */

void started() {
  Particle.publish("started", "Coffee is on it's way! Sit tight! :rocket:", PUBLIC);
  return;
}

void done() {
  Particle.publish("done", "Coffee is served! :coffee:", PUBLIC);
  return;
}

void finished(bool timeout) {
  if (timeout) {
    Particle.publish("finished", "The coffee is getting cold, hurry! :snowflake:", PUBLIC);
  } else {
    Particle.publish("finished", "No more coffee :frowning:", PUBLIC);
  }
  return;
}
