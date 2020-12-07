//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// The following functions are related to Beegees Easter Egg only

const unsigned int freqs[] = {
  3520, // A
  3729, // A#
  3951, // B
  2093, // C
  2217, // C#
  2349, // D
  2489, // D#
  2637, // E
  2794, // F
  2960, // F#
  3136, // G
  3322, // G#
};

const int scale[] = {
  0, 2, 3, 5, 7, 8, 10
};

Player::Player() {
  
}


Player::PlayNote(int note, int octave, int duration)
void
playNote(int octave, int note, int duration)
{
  unsigned int freq;
  int i;

  if (note >= 0) {
    freq = freqs[note];
    for (i = octave; i < 7; i += 1) {
      freq /= 2;
    }
    
    tone(BUZZER2, freq, duration);
    setLEDs(note + 1);
  }
  delay(duration);
  setLEDs(0);
}

void
play(int bpm, char *tune)
{
  unsigned int baseDuration = 60000 / bpm;
  int duration = baseDuration;
  int baseOctave = 4;
  int octave = baseOctave;
  
  int note = -2;

  char *p = tune;
  
  for (; *p; p += 1) {
    boolean playNow = false;
    
    switch (*p) {
    case '>':
      octave = baseOctave + 1;
      break;
    case '<':
      octave = baseOctave - 1;
      break;
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
      playNow = true;
      note = scale[*p - 'A'];
      break;
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
      playNow = true;
      octave += 1;
      note = scale[*p - 'a'];
      break;
    case '_':
      playNow = true;
      note = -1;
      break;
    }
    
    // Check for sharps or flats
    switch (*(p+1)) {
    case '#':
    case '+':
      note += 1;
      if (note == 12) {
        octave += 1;
        note = 0;
      }
      p += 1;
      break;
    case '-':
      note -= 1;
      if (note == -1) {
        octave -= 1;
        note = 11;
      }
      p += 1;
      break;
    }
    
    // Check for octave
    switch (*(p+1)) {
    case ',':
      octave -= 1;
      p += 1;
      break;
    case '\'':
      octave += 1;
      p += 1;
      break;
    }
    
    // Check for duration
    switch (*(p+1)) {
    case '2':
      duration *= 2;
      p += 1;
      break;
    case '4':
      duration *= 4;
      p += 1;
      break;
    case '.':
      duration += duration / 2;
      p += 1;
      break;
    }
    
    if (playNow) {
      playNote(octave, note, duration);
      note = -1;
      octave = baseOctave;
      duration = baseDuration;
    }
    
    if (checkButton()) {
      noTone(BUZZER2);
      return;
    }
  }
  if (note >= 0) {
    playNote(octave, note, duration);
  }
  noTone(BUZZER2);
}

// Do nothing but play bad beegees music
// This function is activated when user holds bottom right button during power up
void play_beegees()
{
  while(checkButton() == CHOICE_NONE) {
    play(104 * 4, "E-F_a-__E-2__C_B-,CE-_B-,C_E-__B-,_C_E-_F_a-_");
  }
}

void lplay(unsigned int tempo, char *tune, int count)
{
  int i;
  
  for (i = 0; i < count; i += 1) {
    play(tempo * 3, tune);
  }
}


void play_march()
{
  const int tempo = 80;
  
  lplay(tempo * 4, "GB-,D", 6);
  lplay(tempo * 4, "GB-,D", 6);
  lplay(tempo * 4, "GB-,D", 6);
  lplay(tempo * 4, "E-G-,B-,", 5);
  play(tempo * 4, "B-");
  lplay(tempo * 4, "GB-,D", 6);
  lplay(tempo * 4, "E-G-,B-,", 5);
  play(tempo * 4, "B-");
  lplay(tempo * 4, "GB-,D", 6);
  lplay(tempo * 4, "GB-,D", 6);
  
  lplay(tempo * 4, "dGB-", 6);
  lplay(tempo * 4, "dGB-", 6);
  lplay(tempo * 4, "dGB-", 6);
  lplay(tempo * 4, "e-G-B-", 5);
  play(tempo * 4, "B-");
  lplay(tempo * 4, "G-B-,F", 6);
  lplay(tempo * 4, "E-G-,B-,", 5);
  play(tempo * 4, "B-");
  lplay(tempo * 4, "GB-,D", 6);
  lplay(tempo * 4, "GB-,D", 6);
  
  return;
}
