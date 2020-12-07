#pragma once

class Pulse {
public:
  Pulse(unsigned long period);
  
  /** Tick tells you if a period has elapsed. */
  bool Tick();

  unsigned long period;
  unsigned long nextEventMillis;
};
