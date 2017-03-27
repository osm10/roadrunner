#ifndef COLOR_H
#define COLOR_H

#include "myvc.h"

#define MAX3(a, b, c) (a > b ? (a > c ? a : c) : (b > c ? b : c))
#define MIN3(a, b, c) (a < b ? (a < c ? a : c) : (b < c ? b : c))

// Cores básicas
typedef enum {
  UnknownColor,
  Red,
  Blue,
  Yellow,
  Green,
} Color;

void vc_color_print(Color color);
long vc_rgb_to_hsv_by_color(IVC *src, IVC *dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax);
long vc_rgb_to_hsv_blue(IVC *src, IVC *dst);
long vc_rgb_to_hsv_red(IVC *src, IVC *dst);
long vc_rgb_to_hsv_yellow(IVC *src, IVC *dst);

#endif /* COLOR_H */
