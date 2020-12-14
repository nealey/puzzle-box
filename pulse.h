#pragma once

#define MILLISECOND 1L
#define SECOND (1000 * MILLISECOND)
#define MINUTE (60 * SECOND)
#define HOUR (60 * MINUTE)
#define DAY (24 * HOUR)

class Pulse {
public:
  Pulse(unsigned long period);
  
  /** Tick tells you if a period has elapsed. */
  bool Tick();

  /** Until sets the duration of the next period. */
  void Until(unsigned long next);
  void Until(unsigned long next, unsigned long now);

  unsigned long period;
  unsigned long nextEventMillis;
};
