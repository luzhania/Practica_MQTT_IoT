#include <Arduino.h>
#pragma once

class Observer {
public:
  virtual void update(const String& message) = 0;
};