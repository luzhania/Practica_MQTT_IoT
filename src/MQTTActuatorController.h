#include "subject.h"
#include "MQTTHandler.h"
#pragma once

class MQTTActuatorController : public Subject {
private:
  MQTTHandler& mqttHandler;

public:
  MQTTActuatorController(MQTTHandler& mqttHandler) 
      : mqttHandler(mqttHandler) {}

  void setCallback() {
    mqttHandler.setCallback([this](char* topic, byte* payload, unsigned int length) {
      String message;
      for (int i = 0; i < length; i++) {
        message += (char)payload[i];
      }
      Serial.print("Message received: ");
      Serial.println(message);
      notifyObservers(message);
    });
  }

  void connect() {
    mqttHandler.connect();
  }

  void loop() {
    mqttHandler.loop();
  }
};