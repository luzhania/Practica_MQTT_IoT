#include "Subject.h"
#pragma once

class SoundSensor : public Subject {
private:
  unsigned int pin;

public:
  SoundSensor(unsigned int pin) : pin(pin) {}

  void checkSoundLevel() {
    unsigned int sensorValue = analogRead(pin);
    Serial.print("Sound level: ");
    Serial.println(sensorValue);
    notifyObservers(String(sensorValue));
  }
};