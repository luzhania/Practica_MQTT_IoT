#include "Observer.h"
#pragma once

class RelayObserver : public Observer {
private:
  unsigned int relayPin;

public:
  RelayObserver(unsigned int relayPin) : relayPin(relayPin) {
    pinMode(relayPin, OUTPUT);
    turnOn();
  }

  void update(const String& message) override {
    if (message == "ON") {
      turnOn();
    } else if (message == "OFF") {
      turnOff();
    }
  }

  void turnOn() {
    digitalWrite(relayPin, HIGH);
  }

  void turnOff() {
    digitalWrite(relayPin, LOW);
  }
};