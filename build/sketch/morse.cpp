#include "morse.h"

// Each morse dit/dah is stored as a nybble

#define dit 1
#define dah 3

#define Pack(a, b, c, d) ((a << 0) | (b << 2) | (c << 4) | (d << 6))
#define Unpack(m, pos) ((m >> (pos * 2)) & 0b11)

const uint8_t MorseLetters[] = {
    Pack(dit, dah, 0, 0),      // A
    Pack(dah, dit, dit, dit),  // B
    Pack(dah, dit, dah, dit),  // C
    Pack(dah, dit, dit, 0),    // D
    Pack(dit, 0, 0, 0),        // E
    Pack(dit, dit, dah, dit),  // F
    Pack(dah, dah, dit, 0),    // G
    Pack(dit, dit, dit, dit),  // H
    Pack(dit, dit, 0, 0),      // I
    Pack(dit, dah, dah, dah),  // J
    Pack(dah, dit, dah, 0),    // K
    Pack(dit, dah, dit, dit),  // L
    Pack(dah, dah, 0, 0),      // M
    Pack(dah, dit, 0, 0),      // N
    Pack(dah, dah, dah, 0),    // O
    Pack(dit, dah, dah, dit),  // P
    Pack(dah, dah, dit, dah),  // Q
    Pack(dit, dah, dit, 0),    // R
    Pack(dit, dit, dit, 0),    // S
    Pack(dah, 0, 0, 0),        // T
    Pack(dit, dit, dah, 0),    // U
    Pack(dit, dit, dit, dah),  // V
    Pack(dit, dah, dah, 0),    // W
    Pack(dah, dit, dit, dah),  // X
    Pack(dah, dit, dah, dah),  // Y
    Pack(dah, dah, dit, dit),  // Z
};

MorseEncoder::MorseEncoder() {
  SetText("");
}
MorseEncoder::MorseEncoder(const char *s) {
  SetText(s);
}

void MorseEncoder::SetText(const char *s) {
  p = s;
  crumb = 0;
  ticksLeft = 0;
  Transmitting = false;
}

void MorseEncoder::Quiet(int ticks) {
  Transmitting = false;
  ticksLeft = ticks;
}

bool MorseEncoder::Tick() {
  --ticksLeft;
  if (ticksLeft > 0) {
    return true;
  }

  // We're out of ticks

  if (!p || !*p) {
    return false;
  }

  // If we were just transmitting, we have to stop for at least one dit
  if (Transmitting) {
    Transmitting = false;
    ticksLeft = MORSE_DIT;
    return true;
  }

  // If that was the end of the letter, we have to pause more
  if (crumb == 4) {
    crumb = 0;
    ++p;
    ticksLeft = MORSE_PAUSE_LETTER - MORSE_DIT;
    return true;
  }

  switch (*p) {
    case '\0':
      return false;
    case 'a' ... 'z':
      Transmitting = true;
      ticksLeft = Unpack(MorseLetters[*p - 'a'], crumb++);
      break;
    case 'A' ... 'Z':
      Transmitting = true;
      ticksLeft = Unpack(MorseLetters[*p - 'A'], crumb++);
      break;
    case ' ':
      crumb = 0;
      ++p;
      Transmitting = false;
      ticksLeft = MORSE_PAUSE_WORD - MORSE_DIT;
      break;
    default:  // this should never happen! Transmit for a word pause to indicate weirdness.
      crumb = 0;
      ++p;
      Transmitting = true;
      ticksLeft = MORSE_PAUSE_WORD;
      break;
  }
  if (0 == ticksLeft) {
    // Unpack can return 0 if there are fewer than 4 emissions for a letter.
    // In that case, we 're done with the letter.
    crumb = 0;
    ++p;
    Transmitting = false;
    ticksLeft = MORSE_PAUSE_LETTER - MORSE_DIT;
  }

  return true;
}
