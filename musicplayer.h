#pragma once

#include <stdint.h>

class MusicPlayer {
 public:
  MusicPlayer(uint8_t pin);

  /** Play begins playing a tune */
  void Play(int bpm, const char *tune);

  /** KeepPlaying should be called once per loop.
   * It returns true if a tune is currently being played.
   */
  bool KeepPlaying();

  /** Tone plays a certain tone */
  void Tone(int note);

  /** NoTone stops playing anything */
  void NoTone();

private:
  uint8_t pin;
  char *tune;
  unsigned long nextNoteMillis;
  uint16_t baseDuration;
};

#define TUNE_STAYIN_ALIVE "E- F P a- P2 E-2 P2 C P <B- C E- P <B- C P E- P2 <B- P C P E- P F P a- P"

#define TUNE_JINGLEBELLS_CHORUS1 "eee2eee2egc.d/e4"
#define TUNE_JINGLEBELLS_CHORUS2 "fff.f/feee/e/edded2g2"
#define TUNE_JINGLEBELLS_CHORUS3 "fff.f/feee/e/ggfdc2._"
#define TUNE_JINGLEBELLS_MELODY1 "GedcG2.G/G/GedcA4"
#define TUNE_JINGLEBELLS_MELODY2 "AfedB2._ ggfde2._"
#define TUNE_JINGLEBELLS_MELODY3 "Afedgggg agfdc2g2"
#define TUNE_JINGLEBELLS_CHORUS TUNE_JINGLEBELLS_CHORUS1 TUNE_JINGLEBELLS_CHORUS2 TUNE_JINGLEBELLS_CHORUS1 TUNE_JINGLEBELLS_CHORUS3
#define TUNE_JINGLEBELLS_MELODY TUNE_JINGLEBELLS_MELODY1 TUNE_JINGLEBELLS_MELODY2 TUNE_JINGLEBELLS_MELODY1 TUNE_JINGLEBELLS_MELODY3
#define TUNE_JINGLEBELLS TUNE_JINGLEBELLS_MELODY TUNE_JINGLEBELLS_CHORUS

#define TUNE_YAY "C_CCG4"
#define TUNE_BOO "C2<F#2"
