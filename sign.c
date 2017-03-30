#include "sign.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

// Sinais de transito conhecidos.
Sign known_signs[] = {
    {"Arrow Left", Circle, Blue}, {"Arrow Right", Circle, Blue},
    {"Arrow Up", Circle, Blue},   {"Highway", Square, Blue},
    {"Car", Square, Blue},        {"Forbidden", Circle, Red},
    {"Stop", Octagon, Red},
};

// Identifica um sinal dos sinais conhecidos.
Sign vc_identify_sign(Color color, Shape shape) {
  Sign sign = {"Unknown", UnknownShape, UnknownColor};
  size_t nsigns = sizeof(known_signs) / sizeof(known_signs[0]);

  for (size_t i = 0; i < nsigns; i++) {
    sign = known_signs[i];
    if ((sign.shape == shape) && (sign.color == color)) {
      return sign;
    }
  }
  return sign;
}

// Determina a cor dominante de uma dada imagem RGB.
// - dst: imagem de saída segmentada para a cor dominante
Color vc_find_color(IVC *src, IVC *dst) {
  Color color = UnknownColor;
  // Imagens temporárias
  IVC *red = vc_rgb_new(src->width, src->height);
  IVC *blue = vc_rgb_new(src->width, src->height);
  IVC *yellow = vc_rgb_new(src->width, src->height);
  size_t size = src->bytesperline * src->height;

  // sinais vermelhos
  long nred = vc_rgb_to_hsv_red(src, red);
#ifdef DEBUG
  if (!vc_write_image_info("out/red.ppm", red)) {
    error("color_segmentation: vc_write_image_info failed\n");
  }
#endif
  // sinais azuis
  long nblue = vc_rgb_to_hsv_blue(src, blue);
#ifdef DEBUG
  if (!vc_write_image_info("out/blue.ppm", blue)) {
    error("color_segmentation: vc_write_image_info failed\n");
  }
#endif
  // sinais amarelos
  long nyellow = vc_rgb_to_hsv_yellow(src, yellow);
#ifdef DEBUG
  if (!vc_write_image_info("out/yellow.ppm", yellow)) {
    error("color_segmentation: vc_write_image_info failed\n");
  }
#endif

#ifdef DEBUG
  printf("Quantidade de pixeis\nVermelho: %ld\tAzul: %ld\tAmarelo: %ld\n", nred,
         nblue, nyellow);
#endif

  if (nred > nblue) {
    if (nred > nyellow) {
      color = Red;
      memcpy(dst->data, red->data, size);
    } else {
      color = Yellow;
      memcpy(dst->data, yellow->data, size);
    }
  } else {
    if (nblue > nyellow) {
      color = Blue;
      memcpy(dst->data, blue->data, size);
    } else {
      color = Yellow;
      memcpy(dst->data, yellow->data, size);
    }
  }

  // Apagar imagens temporárias
  vc_image_free(blue);
  vc_image_free(red);
  vc_image_free(yellow);

  return color;
}

// Determina a forma do sinal.
Shape vc_find_shape(IVC *src) {
  if (!src) {
    fatal("vc_find_shape: src == null\n");
  }

  int i;
  int ntemp_imgs = 2; // número de imagens temporárias
  IVC *tmp[ntemp_imgs];
  for (i = 0; i < ntemp_imgs; i++) {
    tmp[i] = vc_grayscale_new(src->width, src->height);
  }

  /*
    if (!vc_gray_to_binary_otsu(src, tmp[0])) {
      fatal("vc_find_shape: vc_gray_to_binary_otsu failed\n");
    }
  */

  int nblobs = 0; // número de blobs identificados, inicialmente a zero
  OVC *blobs = vc_binary_blob_labelling(src, tmp[0], &nblobs);
  if (!blobs) {
    fatal("vc_find_shape: vc_binary_blob_labelling failed\n");
  }

#ifdef DEBUG
  printf("\nNumber of labels (before filtering): %d\n", nblobs);
#endif

  if (!vc_binary_blob_info(tmp[0], blobs, nblobs)) {
    fatal("vc_find_shape: vc_binary_blob_info failed\n");
  }

/*
  nblobs = vc_binary_blob_filter(&blobs, nblobs, 30);
  if (nblobs == -1) {
    fatal("vc_find_shape: vc_binary_blob_filter failed\n");
  }
*/

#ifdef DEBUG
  printf("Number of labels (after filtering): %d\n", nblobs);
#endif

  for (i = 0; i < nblobs; i++) {
#ifdef DEBUG
    vc_binary_blob_print(&blobs[i]);
    printf("blob %d is probably a ", blobs[i].label);
    vc_shape_print(vc_identify_shape(&blobs[i], 0.2f));
    printf("\n");
#endif
    vc_draw_mass_center(tmp[0], blobs[i].xc, blobs[i].yc, 255);
    vc_draw_boundary_box(tmp[0], blobs[i].x, blobs[i].x + blobs[i].width,
                         blobs[i].y, blobs[i].y + blobs[i].height, 255);
  }

  vc_write_image_info("out/blobbed.pgm", tmp[0]);

  free(blobs);
  for (i = 0; i < ntemp_imgs; i++) {
    vc_image_free(tmp[i]);
  }

  return UnknownShape;
}
