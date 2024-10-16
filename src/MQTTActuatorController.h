#include "subject.h"
#include "MQTTHandler.h"
#pragma once

class MQTTActuatorController : public Subject {
private:
  MQTTHandler& mqttHandler;
  const char* TOPIC;

public:
  MQTTActuatorController(MQTTHandler& mqttHandler, const char* topic) 
      : mqttHandler(mqttHandler), TOPIC(topic) {}

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

  void subscribe() {
    mqttHandler.subscribe(TOPIC);
  }
  
  void connect() {
    mqttHandler.connect();
  }

  void loop() {
    mqttHandler.loop();
  }
};