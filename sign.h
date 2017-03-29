#ifndef SIGN_H
#define SIGN_H

#include "color.h"
#include "shape.h"

// Um sinal de transito é constituido por:
//  - nome: o nome do sinal de transíto. eg: "Stop"
//  - forma: o forma do sinal. eg: Octagon
//  - cor: a cor principal do sinal. eg: Red
typedef struct {
  const char *name;
  Shape shape;
  Color color;
} Sign;

// Sinais de transito conhecidos
extern Sign known_signs[];

Sign vc_identify_sign(Color color, Shape shape);
Color vc_find_color(IVC *src, IVC* dst);
Shape vc_find_shape(IVC *src);

#endif /* SIGN_H */
