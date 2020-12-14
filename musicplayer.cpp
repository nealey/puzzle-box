#include "musicplayer.h"
#include <Arduino.h>

#define MILLISECOND 1L
#define SECOND (1000 * MILLISECOND)
#define MINUTE (60 * SECOND)
#define REST -1
#define OCTAVE 7

// Frequencies in octave 7
const uint16_t freqs[] = {
    2093,  // C
    2217,  // C#
    2349,  // D
    2489,  // D#
    2637,  // E
    2794,  // F
    2960,  // F#
    3136,  // G
    3322,  // G#
    3520,  // A
    3729,  // A#
    3951,  // B
};

const int scale[] = {
    0,
    2,
    3,
    5,
    7,
    8,
    10,
};

MusicPlayer::MusicPlayer(uint8_t pin) {
  this->pin = pin;
  this->tune = NULL;
  pinMode(pin, OUTPUT);
}

void MusicPlayer::Tone(int note) {
  if (REST == note) {
    noTone(pin);
  } else {
    uint16_t freq = freqs[note % 12];

    for (int o = OCTAVE; note < o * 12; --o) {
      freq /= 2;
    }
    tone(pin, freq);
  }
}

void MusicPlayer::NoTone() {
  Tone(REST);
}

bool MusicPlayer::KeepPlaying() {
  unsigned long now = millis();
  uint16_t duration = baseDuration;
  int octave = 4;
  int note = REST;

  if (!tune) {
    return false;
  }
  if (now < nextNoteMillis) {
  // Turn it off for a second in case the note is repeating.
    if (now + 20 > nextNoteMillis) {
      NoTone();
    }
    return true;
  }
  
  // Ignore spaces
  while (*tune == ' ') {
    ++tune;
  }

  for (;;) {
    switch (*tune) {
      case '>':
        ++octave;
        ++tune;
        break;
      case '<':
        --octave;
        ++tune;
        break;
      default:
        goto OCTAVE_DONE;
    }
  }
OCTAVE_DONE:

  switch (*tune) {
    case '\0':
      tune = NULL;
      NoTone();
      return false;
    case 'a' ... 'b':
      ++octave;
    case 'c' ... 'g':
      ++octave;
      note = scale[*tune - 'a'] + (12 * octave);
      break;
    case 'A' ... 'B':
      ++octave;
    case 'C' ... 'G':
      note = scale[*tune - 'A'] + (12 * octave);
      break;
    case '_':
    case 'P':
    default:
      note = REST;
      break;
  }
  ++tune;

  // Check for sharps or flats
  switch (*tune) {
    case '#':
    case '+':
      ++note;
      ++tune;
      break;
    case '-':
      --note;
      ++tune;
      break;
  }

  // Check for duration
  for (;;) {
    switch (*tune) {
      case '/':
        duration /= 2;
        ++tune;
        break;
      case '2':
        duration *= 2;
        ++tune;
        break;
      case '4':
        duration *= 4;
        ++tune;
        break;
      case '.':
        duration += duration / 2;
        ++tune;
        break;
      default:
        goto DURATION_DONE;
    }
  }
DURATION_DONE:

  Tone(note);
  nextNoteMillis = now + duration;

  return true;
}

void MusicPlayer::Play(int bpm, const char *tune) {
  this->tune = tune;
  this->baseDuration = MINUTE / bpm;
  this->nextNoteMillis = millis();
}
