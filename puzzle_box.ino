#include <Adafruit_GFX.h>
#include <Adafruit_MPR121.h>
#include <Adafruit_PCD8544.h>
#include <FastLED.h>
#include <Keyboard.h>
#include <SPI.h>
#include "morse.h"
#include "musicplayer.h"
#include "paj7620.h"
#include "pulse.h"

#define NUM_PUZZLES 4

// WS2812 LEDs
#define LEDS_PIN 13
#define NUM_LEDS NUM_PUZZLES
CRGB leds[NUM_LEDS];
CHSV ColorSolved = CHSV(128, 200, 40);
CHSV ColorUnsolved = CHSV(32, 200, 40);
CHSV ColorBlack = CHSV(0, 0, 0);

// Laser
#define LASER_PIN 7

// Photoresistor
#define PHOTO_PIN A0

// Piezo buzzer
#define BUZZER_PIN 12
MusicPlayer mp = MusicPlayer(BUZZER_PIN);

// Display pin connections. LED needs to be PWM capable.
#define DISPLAY_WIDTH 84
#define DISPLAY_HEIGHT 48
#define DISPLAY_SCE 4
#define DISPLAY_RST 6
#define DISPLAY_DC 5
#define DISPLAY_LED 9
Adafruit_PCD8544 display = Adafruit_PCD8544(DISPLAY_DC, DISPLAY_SCE, DISPLAY_RST);

// Morse code stuff
#define DIT_DURATION (100 * MILLISECOND)

// Touch sensor
Adafruit_MPR121 cap;

void yay() {
  mp.Play(120 * 4, TUNE_YAY);
}

void boo() {
  mp.Play(120 * 4, TUNE_BOO);
}

void setup() {
  // Turn on backlight
  pinMode(DISPLAY_LED, OUTPUT);
  analogWrite(DISPLAY_LED, 64);

  // Turn on display
  display.begin();
  display.setContrast(50);
  display.display();

  FastLED.addLeds<WS2812, LEDS_PIN, GRB>(leds, NUM_LEDS);
  //FastLED.setBrightness(96);

  // Gesture sensor
  while (paj7620Init()) {
    display.clearDisplay();
    display.print("PAJ7260?");
    display.display();
  }

  while (!cap.begin()) {
    display.clearDisplay();
    display.print("MPR121?");
    display.display();
  }

  pinMode(LASER_PIN, OUTPUT);
  pinMode(PHOTO_PIN, INPUT);

  // Riddler symbol
  poem("Touch White", "Wire", "", "Caution: Laser");
  display.setTextSize(2);
  display.setCursor(70, 32);
  display.print('\x19');
  display.display();

  randomSeed(analogRead(PHOTO_PIN));

  // Be a USB keyboard
  Keyboard.begin();

  // Hello!
  yay();
}

// returns true if it actually printed the poem
// in other words, if we just got foreground
bool poem(const char *s1, const char *s2, const char *s3, const char *s4) {
  static char *last_s1 = NULL;

  if (last_s1 != s1) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(s1);
    display.setCursor(0, 8);
    display.print(s2);
    display.setCursor(0, 16);
    display.print(s3);
    display.setCursor(0, 24);
    display.print(s4);
    display.display();
    last_s1 = s1;
    return true;
  }
  return false;
}

CHSV solvedPoem(bool fg) {
  if (fg) {
    poem("Solved!", "", "", "");
  }
  return ColorSolved;
}

CHSV loop_morse(bool fg) {
  static Pulse pulse = Pulse(DIT_DURATION);
  static MorseEncoder enc = MorseEncoder("CQ");
  static uint16_t errors = 0;
  static bool solved = false;
  int recv = analogRead(PHOTO_PIN);
  bool error = ((recv >= 512) != enc.Transmitting);
  CHSV color;

  if (solved) {
    return solvedPoem(fg);
  } else if (!fg) {
    digitalWrite(LASER_PIN, false);
    return ColorUnsolved;
  }

  poem(
      "Help my sensor",
      "See the light;",
      "Got to get the",
      "Timing right!");

  color = CHSV(0, 255, recv >> 2);
  if (error) {
    ++errors;
  }

  if (!pulse.Tick()) {
    return color;
  }

  if (enc.Tick()) {
    digitalWrite(LASER_PIN, enc.Transmitting);
  } else {
    // We've sent the whole thing
    if (errors < 500) {
      solved = true;
      yay();
      return solvedPoem(fg);
    }
    boo();
    enc.SetText("HO HO HO");
    enc.Quiet(30);
    errors = 0;
  }

  return color;
}

#define UP 24
#define DOWN 25
#define RIGHT 26
#define LEFT 27
#define IN 'B'
#define OUT 'A'
const char KonamiCode[] = {
    UP,
    UP,
    DOWN,
    DOWN,
    LEFT,
    RIGHT,
    LEFT,
    RIGHT,
    IN,
    OUT,
    0,
};
CHSV loop_konami(bool fg) {
  static bool solved = false;
  static int pos = 0;
  static Pulse pulse = Pulse(100 * MILLISECOND);
  CHSV color;
  uint8_t gesture;

  if (solved) {
    return solvedPoem(fg);
  } else if (!fg) {
    return ColorUnsolved;
  }

  poem(
      "wave it in:",
      "it codifies",
      "beefy men with",
      "30 lives");

  uint8_t prox = 0;
  paj7620ReadReg(0x6c, 1, &prox);
  color = CHSV(0, 255, prox);

  if (!pulse.Tick()) {
    return color;
  }

  if (!paj7620ReadReg(0x43, 1, &gesture)) {
    char out = 0;

    switch (gesture) {
      case 0:
        break;
      case GES_UP_FLAG:
        out = RIGHT;
        break;
      case GES_DOWN_FLAG:
        out = LEFT;
        break;
      case GES_RIGHT_FLAG:
        out = DOWN;
        break;
      case GES_LEFT_FLAG:
        out = UP;
        break;
      case GES_FORWARD_FLAG:
        out = IN;
        break;
      case GES_BACKWARD_FLAG:
        out = OUT;
        break;
      default:
        // out = '?';
        break;
    }
    if (out) {
      if (out == KonamiCode[pos]) {
        display.fillRect(pos * 6, 40, 6, 8, 0);
        display.setCursor(pos * 6, 40);
        ++pos;
      } else {
        display.fillRect(0, 40, 84, 8, 0);
        display.setCursor(0, 40);
        pos = 0;
        boo();
      }
      display.print(out);
    }
  }

  if (KonamiCode[pos] == 0) {
    solved = true;
    yay();
    return solvedPoem(fg);
  }
  display.display();

  return color;
}

#define ROUNDS_TO_WIN 5
const char *inputNames[] = {
    "yellow",
    "green",
    "red",
    "blue",
};
const uint8_t numInputNames = sizeof(inputNames) / sizeof(*inputNames);

CHSV simonSound(int num) {
  if (-1 == num) {
    mp.NoTone();
    return ColorBlack;
  } else {
    uint16_t hue;
    switch (num) {
      case 0:
        mp.Tone(60);
        hue = HUE_YELLOW;
        break;
      case 1:
        mp.Tone(55);
        hue = HUE_GREEN;
        break;
      case 2:
        mp.Tone(50);
        hue = HUE_RED;
        break;
      case 3:
        mp.Tone(45);
        hue = HUE_BLUE;
        break;
    }
    return CHSV(hue, 255, 64);
  }
}

#define SIMON_TONE_DURATION (300 * MILLISECOND)
#define SIMON_QUIET_DURATION (SIMON_TONE_DURATION * 2)
#define SIMON_INPUT_TIMEOUT (4 * SECOND)
CHSV loop_simon(bool fg, uint16_t touched, uint16_t justTouched) {
  static uint8_t round = 0;
  static uint8_t sequencePosition = 0;
  static bool solved = false;
  static uint8_t sequence[ROUNDS_TO_WIN] = {0};
  static uint8_t state = 0;
  static Pulse pulse = Pulse(0);
  static CHSV color = ColorBlack;
  bool ticked = pulse.Tick();

  if (solved) {
    return solvedPoem(fg);
  } else if (!fg) {
    return ColorUnsolved;
  }

  bool reset = poem(
      "For a fun game",
      "we can play:",
      "You repeat all",
      "that I say!");
  if (reset) {
    state = 0;
  }

  uint8_t input = sequence[sequencePosition];
  switch (state) {
    case 0: {  // quiet
      // Beginning of a round: be quiet for a bit
      color = simonSound(-1);
      pulse.Until(SIMON_QUIET_DURATION);
      // Pick an input at random for the next sequence item
      sequence[round] = random(numInputNames);
      // Start the sequencePosition loop at position 0
      sequencePosition = 0;
      state = 1;
      break;
    }
    case 1: {  // Wait for a tick
      if (!ticked) {
        break;
      }
      state = 2;
      break;
    }
    case 2: {  // beep
      // set the buzzer and color
      color = simonSound(input);
      pulse.Until(SIMON_TONE_DURATION);
      state = 3;
      break;
    }
    case 3: {  // Wait for a tick
      if (!ticked) {
        break;
      }
      state = 4;
      break;
    }
    case 4: {  // no beep
      // Stop the tone and color
      color = simonSound(-1);
      // Is that the last one this round?
      if (sequencePosition == round) {
        // Start listening for input
        pulse.Until(SIMON_INPUT_TIMEOUT);
        sequencePosition = 0;
        state = 5;
      } else {
        // We're ready to play the next tone
        ++sequencePosition;
        pulse.Until(SIMON_TONE_DURATION);
        state = 1;
      }
      break;
    }
    case 5: {  // Listen for input
      if (ticked) {
        // Time's up!
        state = 7;
      }
      if (justTouched) {
        if (justTouched == bit(input)) {
          // That's right!
          color = simonSound(input);
          state = 6;
        } else {
          // Wrong!
          state = 7;
        }
      }
      break;
    }
    case 6: {  // We're playing the right tone, keep doing that until they let go
      if (!touched) {
        color = simonSound(-1);
        if (sequencePosition == round) {
          // They got everything this round, increase difficulty
          state = 8;
        } else {
          // Listen for the next one
          ++sequencePosition;
          pulse.Until(SIMON_INPUT_TIMEOUT);
          state = 5;
        }
      }
      break;
    }
    case 7: {  // Wrong!
      boo();
      pulse.Until(1 * SECOND);
      round = 0;
      sequencePosition = 0;
      state = 9;
      break;
    }
    case 8: {  // Increase difficulty!
      ++round;
      if (round == ROUNDS_TO_WIN) {
        yay();
        solved = true;
        return solvedPoem(true);
      }
      sequencePosition = 0;
      state = 9;
      break;
    }
    case 9: {  // wait for tick
      if (!ticked) {
        break;
      }
      sequencePosition = 0;
      state = 0;
      break;
    }
    default:
      // This should never happen...
      state = 0;
      break;
  }

  return color;
}

#define KEYBOARD_INPUT_ROUNDS 6
CHSV loop_keyboard(bool fg, uint16_t justTouched) {
  static bool solved = false;
  static uint8_t remaining = KEYBOARD_INPUT_ROUNDS;
  static uint8_t inputName = 0;
  static Pulse pulse = Pulse(6 * SECOND);

  if (solved) {
    return solvedPoem(fg);
  } else if (!fg) {
    return ColorUnsolved;
  }

  poem(
      "Let me talk to",
      "a computer,",
      "You will see I",
      "get much cuter");

  if (pulse.Tick()) {
    inputName = random(numInputNames);
    Keyboard.print("\nNow touch ");
    Keyboard.print(inputNames[inputName]);
    Keyboard.print("...");
  }

  if (justTouched) {
    Keyboard.print(" ");
    if (justTouched == bit(inputName)) {
      Keyboard.print("correct.");
      --remaining;
    } else {
      Keyboard.print("wrong.");
      boo();
      remaining = KEYBOARD_INPUT_ROUNDS;
    }
    if (remaining == 0) {
      Keyboard.print("\nGOOD JOB, HUMAN\n");
      yay();
      solved = true;
      return solvedPoem(true);
    }
    pulse.Until(0);  // Pick another wire immediately
  }

  return ColorBlack;
}

void beHappy() {
  while (true) {
    if (!mp.KeepPlaying()) {
      mp.Play(76 * 4, TUNE_JINGLEBELLS);

      poem(
          "The final act",
          "enjoy you must",
          "I type for you",
          "Some C++!");

      Keyboard.print("Happy Holidy, Martin! https://github.com/nealey/puzzle-box\n");
    }
  }
}

void loop() {
  static int current = -1;
  static bool solved[NUM_PUZZLES] = {0};
  static CHSV lastColors[NUM_PUZZLES];
  bool writeLEDs = false;
  bool allSolved = true;

  mp.KeepPlaying();

  // Read capacative touch sensors
  static uint16_t lastTouched = 0;
  uint16_t touched = cap.touched();
  uint16_t justTouched = (lastTouched ^ touched) & touched;

  if (bitRead(justTouched, 4)) {
    current = (current + 1) % NUM_PUZZLES;
    tone(BUZZER_PIN, 220 * (current + 1), 220);
  }

  for (int i = 0; i < NUM_PUZZLES; ++i) {
    CHSV color = CHSV(0, 0, 0);
    bool fg = (current == i);

    switch (i) {
      case 0:
        color = loop_morse(fg);
        break;
      case 1:
        color = loop_konami(fg);
        break;
      case 2:
        color = loop_simon(fg, touched, justTouched);
        break;
      case 3:
        color = loop_keyboard(fg, justTouched);
        break;
    }
    if (color != lastColors[i]) {
      lastColors[i] = color;
      writeLEDs = true;
    }
    if (color != ColorSolved) {
      allSolved = false;
    }
    leds[i] = color;
  }

  if (writeLEDs) {
    FastLED.show();
  }
  lastTouched = touched;

  if (allSolved) {
    beHappy();
  }
}
