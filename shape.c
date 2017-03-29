#include "shape.h"
#include "myvc.h"

float vc_calculate_rectangle_area(OVC *blob) {
  if (!blob)
    return 0;

  return (float)(blob->width * blob->height);
}

float vc_calculate_circle_area(OVC *blob) {
  if (!blob)
    return 0.0;

  float radius = (float)(blob->width) / 2.0;

  return 2 * 3.14159265 * (radius * radius);
}

Shape vc_identify_shape(OVC *blob, float tolerance) {
  Shape obj = UnknownShape;

  if (!blob)
    return obj;
  if (tolerance < 0.0 || tolerance > 1.0)
    return obj;

  float area_circle = vc_calculate_circle_area(blob);
  float area_rectangle = vc_calculate_rectangle_area(blob);

  float upper_circ = area_circle + (area_circle * tolerance);
  float lower_circ = area_circle - (area_circle * tolerance);

  float upper_rect = area_rectangle + (area_rectangle * tolerance);
  float lower_rect = area_rectangle - (area_rectangle * tolerance);

  if (blob->area >= lower_circ && blob->area <= upper_circ)
    obj = Circle;
  else if (blob->area >= lower_rect && blob->area <= upper_rect)
    obj = Rectangle;

  return obj;
}

void vc_shape_print(Shape shape) {
  switch (shape) {
  case Circle:
    printf("Circle\n");
    break;
  case Square:
    printf("Square\n");
    break;
  case Rectangle:
    printf("Rectangle\n");
    break;
  case Octagon:
    printf("Octagon\n");
    break;
  default:
    printf("Unknown shape\n");
  }
}
