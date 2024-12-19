
#include <FastLED.h>

#define LED_PIN 38
#define NUM_LEDS 1

CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
}

void loop() {
  static uint8_t hue = 0;
  fill_rainbow(leds, NUM_LEDS, hue, 10);
  FastLED.show();
  hue++;
  delay(20);
}
