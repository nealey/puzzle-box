#include <Arduino.h>
#line 1 "/home/neale/Arduino/puzzle_box/puzzle_box.ino"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <FastLED.h>
#include "morse.h"
#include "pulse.h"
#include "riddler.h"

// WS2812 LEDs
#define LEDS_PIN 1
#define NUM_LEDS 2
CRGB leds[NUM_LEDS];

// Laser
#define LASER_PIN 0

// Photoresistor
#define PHOTO_PIN A0

// Piezo buzzer
#define BUZZER_PIN 11

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

  // Turn on backlight
  pinMode(DISPLAY_LED, OUTPUT);
  analogWrite(9, 64);
  
  // Turn on display
  display.begin();
  display.setContrast(30);
  display.clearDisplay();
  display.drawBitmap(
    (DISPLAY_WIDTH - riddler_width)/2, 
    (DISPLAY_HEIGHT - riddler_height)/2, 
    riddler_bits, 
    riddler_width, 
    riddler_height, 
    0xffff
  );
  display.display();

  pinMode(LASER_PIN, OUTPUT);
  pinMode(PHOTO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  tone(BUZZER_PIN, 110);

}

CHSV loop_morse(Adafruit_PCD8544 *display) {
  static Pulse pulse = Pulse(DIT_DURATION);
  static MorseEncoder enc = MorseEncoder("CQ CQ KD7OQI");
  static uint16_t errors = 0;
  bool solved = false;
  int recv = analogRead(PHOTO_PIN);
  bool error = ((recv >= 512) != enc.Transmitting);
  CHSV color;
  
  if (solved) {
    color = CHSV(128, 40, 64);
  } else if (error) {
    ++errors;
    color = CHSV(0, 255, recv>>2);
  } else {
    color = CHSV(128, 255, recv>>2);
  }

  if (! pulse.Tick()) {
    return color;
  }
  
  if (display) {
    display->clearDisplay();
    display->setFont();
    display->setCursor(0, 0);
    display->print("The Morse One");
    display->display();
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
}

void messageHappy() {
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

void loop() {
  leds[0] = loop_morse(NULL);
  
  FastLED.show();
}

