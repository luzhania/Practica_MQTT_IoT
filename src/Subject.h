#include "Observer.h"
#include <vector>
#pragma once

using namespace std;

class Subject {
private:
  vector<Observer*> observers;

public:
  void attach(Observer* observer) {
    observers.push_back(observer);
  }

  void detach(Observer* observer) {
    observers.erase(remove(observers.begin(), observers.end(), observer), observers.end());
  }

  void notifyObservers(const String& message) {
    for (Observer* observer : observers) {
      observer->update(message);
    }
  }
};