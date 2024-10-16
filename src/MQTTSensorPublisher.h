#include "Observer.h"
#include "MQTTHandler.h"
#pragma once

class MQTTSensorPublisher : public Observer {
private:
  MQTTHandler& mqttHandler;
  const char* TOPIC;

public:
  MQTTSensorPublisher(MQTTHandler& mqttHandler, const char* TOPIC)
      : mqttHandler(mqttHandler), TOPIC(TOPIC) {}

  void update(const String& message) override {
    mqttHandler.publish(message, TOPIC);
  }

  void connect() {
    mqttHandler.connect();
  }

  void loop() {
    mqttHandler.loop();
  }
};