#include "coffee-messages.h"

Coffeemessages::Coffeemessages() {
}

void Coffeemessages::proclaim(String message) {
    Particle.publish("dev_slack", message, PUBLIC);
}

// Example private method
void Coffeemessages::secret() {
    Serial.println("called doit");
}
