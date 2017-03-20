#include "color.h"

void vc_color_print(Color color) {
  switch (color) {
  case Red:
    printf("Red");
    break;
  case Blue:
    printf("Blue");
    break;
  case Yellow:
    printf("Yellow");
    break;
  case Green:
    printf("Green");
    break;
  default:
    printf("Unknown color");
  }
}
