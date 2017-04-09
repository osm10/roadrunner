#ifndef SHAPE_H
#define SHAPE_H

#include "myvc.h"

// Formas de sinais de transíto
typedef enum {
  UnknownShape,
  Circle,
  Square,
  Rectangle,
  Octagon,
} Shape;

char* vc_shape_name(Shape shape);
float vc_calculate_circle_area(OVC *blob);
float vc_calculate_rectange_area(OVC *blob);
Shape vc_identify_shape(OVC *blob, float tolerance);

#endif /* SHAPE_H */
