#define READ_PIN A0
#define LED_PIN D7
#define NBR_OF_READINGS 3
#define READING_DELAY 100
#define LOOP_DELAY 5000
#define UPPER_TRESHOLD 3000
#define MIDDLE_TRESHOLD 1000
#define LOWER_TRESHOLD 500

SYSTEM_MODE(SEMI_AUTOMATIC);

int state = 0;

void setup() {
  pinMode(READ_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  WiFi.on();
  WiFi.setCredentials("SSID", "password");
  Particle.connect();

  Particle.function("read", read);
  Particle.function("started", read);
  Particle.function("done", read);
  Particle.function("finished", read);
  /* Particle.variable("reading", reading) */
}

void loop() {
  int reading = readMany("");

  // waiting for it to start
  if (state == 0 && reading > UPPER_TRESHOLD) {
    // Boiling has started! Do nothing
    state = 1;
    started("");
    break;
  }

  // waiting for it to finish boiling
  if (state == 1 && reading < MIDDLE_TRESHOLD) {
    // Boiling is done, so coffee is (almost) done
    state = 2;

    // TODO: maybe add some kind of delay here
    done("");
    break;
  }

  // waiting for it to finish boiling
  if (state == 2 && reading < LOWER_TRESHOLD) {
    // The coffee brewer was turned off, no more coffee
    state = 0;
    finished("");
    break;
  }

  delay(LOOP_DELAY);
}

// Do three readings, take some kind of average
int readMany(String command) {
  int reading = 0;
  int newReading = 0;
  for (int i = 0; i < NBR_OF_READINGS; i++) {
    newReading = read("");
    reading = reading + newReading;
    delay(READING_DELAY);
  }
  return reading / NBR_OF_READINGS;
}

int read(String command) {
  return analogRead(LED_PIN);
}

void started(String command) {
  Particle.publish("started");
  return;
}

void done(String command) {
  Particle.publish("done");
  return;
}

void finished(String command) {
  Particle.publish("finished");
  return;
}
