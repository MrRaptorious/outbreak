#include "colorConversion.h"
#include <raymath.h>

Vector3 rgb2hsl(float r, float g, float b) {
  r /= 255.0f;
  g /= 255.0f;
  b /= 255.0f;

  float max = fmaxf(fmaxf(r, g), b);
  float min = fminf(fminf(r, g), b);
  float delta = max - min;

  float h, s, l;

  if (delta == 0) {
    h = 0; // undefined, it's a shade of gray
  } else if (max == r) {
    h = 60 * fmodf(((g - b) / delta), 6);
  } else if (max == g) {
    h = 60 * (((b - r) / delta) + 2);
  } else if (max == b) {
    h = 60 * (((r - g) / delta) + 4);
  }

  if (h < 0) {
    h += 360;
  }

  l = (max + min) / 2;

  if (delta == 0) {
    s = 0;
  } else {
    s = (delta / (1 - fabsf(2 * l - 1)));
  }

  return (Vector3){h, s, l};
}

Color hsl2rgb(float h, float s, float l) {
  float c = (1 - fabs(2 * l - 1)) * s;
  float x = c * (1 - fabs(fmod((h / 60), 2) - 1));
  float m = l - c / 2;

  float r1, g1, b1;

  if (h >= 0 && h < 60) {
    r1 = c;
    g1 = x;
    b1 = 0;
  } else if (h >= 60 && h < 120) {
    r1 = x;
    g1 = c;
    b1 = 0;
  } else if (h >= 120 && h < 180) {
    r1 = 0;
    g1 = c;
    b1 = x;
  } else if (h >= 180 && h < 240) {
    r1 = 0;
    g1 = x;
    b1 = c;
  } else if (h >= 240 && h < 300) {
    r1 = x;
    g1 = 0;
    b1 = c;
  } else {
    r1 = c;
    g1 = 0;
    b1 = x;
  }

  unsigned char r = (r1 + m) * 255;
  unsigned char g = (g1 + m) * 255;
  unsigned char b = (b1 + m) * 255;

  return (Color){r, g, b, 255};
}
