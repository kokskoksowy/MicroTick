#include <Arduino.h>
#define RGB_PIN 48       // pin danych dla wbudowanej diody RGB

void setRGB(int r, int g, int b, int brightness) {
  // ograniczenie wartości 0-255
  r = constrain(r, 0, 255);
  g = constrain(g, 0, 255);
  b = constrain(b, 0, 255);

  // ograniczenie brightness do zakresu 0-255
  brightness = constrain(brightness, 0, 255);

  // zastosowanie jasności
  r = (r * brightness) / 255;
  g = (g * brightness) / 255;
  b = (b * brightness) / 255;

  neopixelWrite(RGB_PIN, r, g, b);
}



