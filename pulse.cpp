#include "pulse.h"

#include <Arduino.h>

Pulse::Pulse(unsigned long period) {
  this->period = period;
  this->nextEventMillis = 0;
}

bool Pulse::Tick() {
  unsigned long now = millis();
  
  if (now >= nextEventMillis) {
    nextEventMillis = now + period;
    return true;
  }
  
  return false;
}
