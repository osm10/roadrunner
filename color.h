#ifndef COLOR_H
#define COLOR_H

#include "myvc.h"

// Cores básicas
typedef enum {
  UnknownColor,
  Red,
  Blue,
  Yellow,
  Green,
} Color;

void vc_color_print(Color color);

#endif /* COLOR_H */
