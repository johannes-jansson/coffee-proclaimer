#define READ_PIN A0
#define LED_PIN D7
#define NBR_OF_READINGS 3
#define READING_DELAY 100
#define LOOP_DELAY 5000
#define UPPER_TRESHOLD 3000
#define MIDDLE_TRESHOLD 1000
#define LOWER_TRESHOLD 500
#define THINGSPEAK HIGH

SYSTEM_MODE(SEMI_AUTOMATIC);

int state = 0;

void setup() {
  pinMode(READ_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  WiFi.on();
  WiFi.setCredentials("SSID", "password");
  Particle.connect();

  Particle.function("read", readVoltage);
  Particle.function("started", started);
  Particle.function("done", done);
  Particle.function("finished", finished);
  /* Particle.variable("reading", reading) */
  digitalWrite(LED_PIN, HIGH);
}

void loop() {
  int reading = readMany("");
  if (THINGSPEAK) {
    Particle.publish("tsReading", String(reading));
  }

  // waiting for it to start
  if (state == 0 && reading > UPPER_TRESHOLD) {
    // Boiling has started! Do nothing
    state = 1;
    started("");
    return;
  }

  // waiting for it to finish boiling
  if (state == 1 && reading < MIDDLE_TRESHOLD) {
    // Boiling is done, so coffee is (almost) done
    state = 2;

    // TODO: maybe add some kind of delay here
    done("");
    return;
  }

  // waiting for it to finish boiling
  if (state == 2 && reading < LOWER_TRESHOLD) {
    // The coffee brewer was turned off, no more coffee
    state = 0;
    finished("");
    return;
  }

  delay(LOOP_DELAY);
}

// Do three readings, take some kind of average
int readMany(String command) {
  int reading = 0;
  int newReading = 0;
  for (int i = 0; i < NBR_OF_READINGS; i++) {
    newReading = readVoltage("");
    reading = reading + newReading;
    delay(READING_DELAY);
  }
  return reading / NBR_OF_READINGS;
}

int readVoltage(String command) {
  return analogRead(READ_PIN);
}

int started(String command) {
  if (THINGSPEAK) {
    Particle.publish("tsStarted", "1", PRIVATE);
  }
  return 1;
}

int done(String command) {
  if (THINGSPEAK) {
    Particle.publish("tsDone", "1", PRIVATE);
  }
  return 1;
}

int finished(String command) {
  if (THINGSPEAK) {
    Particle.publish("tsFinished", "1", PRIVATE);
  }
  return 1;
}
