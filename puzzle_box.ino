#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <FastLED.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <SPI.h>
#include "morse.h"
#include "musicplayer.h"
#include "paj7620.h"
#include "pulse.h"
#include "riddler.h"

#define NUM_PUZZLES 6

// WS2812 LEDs
#define LEDS_PIN 1
#define NUM_LEDS NUM_PUZZLES
CRGB leds[NUM_LEDS];
CHSV ColorSolved = CHSV(32, 200, 40);

// Laser
#define LASER_PIN 0

// Photoresistor
#define PHOTO_PIN A0

// Piezo buzzer
#define BUZZER_PIN 11
MusicPlayer mp = MusicPlayer(BUZZER_PIN);

// Display pin connections. LED needs to be PWM capable.
#define DISPLAY_WIDTH 84
#define DISPLAY_HEIGHT 48
#define DISPLAY_SCE 4
#define DISPLAY_RST 7
#define DISPLAY_DC 8
#define DISPLAY_LED 9
Adafruit_PCD8544 display = Adafruit_PCD8544(DISPLAY_DC, DISPLAY_SCE, DISPLAY_RST);

// Morse code stuff
#define DIT_DURATION 100

void setup() {
  FastLED.addLeds<WS2812, LEDS_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(96);

  // Turn on backlight
  pinMode(DISPLAY_LED, OUTPUT);
  analogWrite(9, 64);

  // Turn on display
  display.begin();
  display.setContrast(50);

  // Gesture sensor
  while (paj7620Init()) {
    display.clearDisplay();
    display.print("PAJ7260?");
    display.display();
  }

  pinMode(LASER_PIN, OUTPUT);
  pinMode(PHOTO_PIN, INPUT);

  // Riddler symbol
  display.clearDisplay();
  display.drawBitmap(
      (DISPLAY_WIDTH - riddler_width) / 2,
      (DISPLAY_HEIGHT - riddler_height) / 2,
      riddler_bits,
      riddler_width,
      riddler_height,
      0xffff);
  display.display();

  // Hello!
  mp.Play(120 * 4, TUNE_YAY);
}

CHSV loop_morse(bool fg) {
  static Pulse pulse = Pulse(DIT_DURATION);
  static MorseEncoder enc = MorseEncoder("CQ CQ KD7OQI");
  static uint16_t errors = 0;
  bool solved = false;
  int recv = analogRead(PHOTO_PIN);
  bool error = ((recv >= 512) != enc.Transmitting);
  CHSV color;

  if (solved) {
    return ColorSolved;
  } else if (error) {
    ++errors;
    color = CHSV(0, 255, recv >> 2);
  } else {
    color = CHSV(128, 255, recv >> 2);
  }

  if (!pulse.Tick()) {
    return color;
  }

  if (fg) {
    display.clearDisplay();
    display.setFont();
    display.setCursor(0, 0);
    display.print("The Morse One");
    display.display();
  }

  if (enc.Tick()) {
    digitalWrite(LASER_PIN, enc.Transmitting);
  } else {
    // We've sent the whole thing
    if (errors < 500) {
      solved = true;
    }
    enc.SetText("HO HO HO ARK");
    enc.Quiet(30);
    errors = 0;
  }

  return color;
}

CHSV loop_konami(bool fg) {
  static Pulse pulse = Pulse(100);
  CHSV color;
  uint8_t gesture;

  uint8_t prox = 0;
  if (!paj7620ReadReg(0x6c, 1, &prox)) {
    display.fillRect(0, 0, 84, 4, 0);
    display.fillRect(0, 0, min(prox / 3, 84), 4, 1);
  }

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
        out = 26;  // right
        break;
      case GES_DOWN_FLAG:
        out = 27;  // left
        break;
      case GES_RIGHT_FLAG:
        out = 25;  // down
        break;
      case GES_LEFT_FLAG:
        out = 24;  // up
        break;
      case GES_FORWARD_FLAG:
        out = 15;
        break;
      case GES_BACKWARD_FLAG:
        out = 9;
        break;
      default:
        out = '?';
        display.fillRect(0, 32, 15, 8, 0);
        display.setCursor(0, 32);
        display.print(gesture);
        break;
    }
    if (out) {
      display.fillRect(0, 40, 5, 8, 0);
      display.setCursor(0, 40);
      display.print(out);
    }
  }
  display.display();

  return color;
}

void beHappy() {
  if (!mp.KeepPlaying()) {
    mp.Play(76 * 4, TUNE_JINGLEBELLS);

    display.clearDisplay();
    display.setFont(&FreeSerif9pt7b);
    display.setCursor(0, 12);
    display.print("Happy");
    display.setCursor(0, 29);
    display.print("Holiday,");
    display.setCursor(0, 46);
    display.print("Martin!");
    display.display();
  }
}

void loop() {
  static int current = -1;
  static bool solved[NUM_PUZZLES] = {0};
  static CHSV lastColors[NUM_PUZZLES];
  bool writeLEDs = false;
  bool allSolved = true;

  mp.KeepPlaying();

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

  if (allSolved) {
    beHappy();
  }

  if (writeLEDs) {
    FastLED.show();
  }
}
