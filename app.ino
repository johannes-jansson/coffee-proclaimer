#include "math.h"
#define READ_PIN A0
#define GROUND_PIN A1
#define LED_PIN D7

#define NBR_OF_READINGS 500
#define READING_DELAY 1
#define LOOP_DELAY 4000

#define UPPER_TRESHOLD 3000
#define MIDDLE_TRESHOLD 1000
#define LOWER_TRESHOLD 500

#define THINGSPEAK HIGH

SYSTEM_MODE(SEMI_AUTOMATIC);

int state = 0;
int positiveReadings[NBR_OF_READINGS];
int maxReading = -1;

void setup() {
  pinMode(READ_PIN, INPUT);
  pinMode(GROUND_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  WiFi.on();
  /* WiFi.setCredentials("insert-SSID-here", "insert-password-here"); */
  Particle.connect();

  Particle.function("started", started);
  Particle.function("done", done);
  Particle.function("finished", finished);
  Particle.variable("max", maxReading);
}

void loop() {
  int reading = readMany("");
  if (THINGSPEAK) {
    Particle.publish("tsReading", String(reading));
  }

  /* // waiting for it to start */
  /* if (state == 0 && reading > UPPER_TRESHOLD) { */
  /*   // Boiling has started! Do nothing */
  /*   state = 1; */
  /*   started(""); */
  /*   return; */
  /* } */

  /* // waiting for it to finish boiling */
  /* if (state == 1 && reading < MIDDLE_TRESHOLD) { */
  /*   // Boiling is done, so coffee is (almost) done */
  /*   state = 2; */

  /*   // TODO: maybe add some kind of delay here */
  /*   done(""); */
  /*   return; */
  /* } */

  /* // waiting for it to finish boiling */
  /* if (state == 2 && reading < LOWER_TRESHOLD) { */
  /*   // The coffee brewer was turned off, no more coffee */
  /*   state = 0; */
  /*   finished(""); */
  /*   return; */
  /* } */

  delay(LOOP_DELAY);
}

// Do NBR_OF_READINGS readings, return RMS
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

int started(String command) {
  if (THINGSPEAK) {
    Particle.publish("tsStarted", "1", PUBLIC);
  }
  return 1;
}

int done(String command) {
  if (THINGSPEAK) {
    Particle.publish("tsDone", "1", PUBLIC);
  }
  return 1;
}

int finished(String command) {
  if (THINGSPEAK) {
    Particle.publish("tsFinished", "1", PUBLIC);
  }
  return 1;
}
