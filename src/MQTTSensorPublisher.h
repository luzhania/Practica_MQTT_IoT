#include "Observer.h"
#include "MQTTHandler.h"
#pragma once

class MQTTSensorPublisher : public Observer {
private:
  MQTTHandler& mqttHandler;

public:
  MQTTSensorPublisher(MQTTHandler& mqttHandler)
      : mqttHandler(mqttHandler) {}

  void update(const String& message) override {
    mqttHandler.publish(message, "titos/place/sound");
  }

  void connect() {
    mqttHandler.connect();
  }

  void loop() {
    mqttHandler.loop();
  }
};