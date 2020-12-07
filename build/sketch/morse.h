#pragma once

#include <stdint.h>

#define MORSE_DIT 1
#define MORSE_DAH 3
#define MORSE_PAUSE_LETTER 3
#define MORSE_PAUSE_WORD 6

class MorseEncoder {
 public:
  MorseEncoder();
  MorseEncoder(const char *s);

  /** SetText resets state with new text.
   */
  void SetText(const char *s);

  /** Tick tells the encoder that a dit has elapsed.
    * 
    * Returns true if there's data left to transmit.
    * If it returns false, you need to feed it more data.
    * 
    * You should call this every time your dit duration ends.
    */
  bool Tick();

  /** Quiet stops transmitting for this many ticks.  
   */
  void Quiet(int ticks);

  /** Transmitting is true if you should be transmitting right now.  
   */
  bool Transmitting;

 private:
  const char *p;
  uint8_t crumb;
  int ticksLeft;
};
