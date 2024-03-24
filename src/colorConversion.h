#ifndef COLOR_CONVERSION
#define COLOR_CONVERSION

#include <raylib.h>

Vector3 rgb2hsl(float r, float g, float b);
Color hsl2rgb(float h, float s, float l);

#endif
