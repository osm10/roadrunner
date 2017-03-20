#include "sign.h"
#include "stdlib.h"
#include "utils.h"

// Sinais de transito conhecidos
Sign known_signs[7] = {
    {"Arrow Left", Circle, Blue}, {"Arrow Right", Circle, Blue},
    {"Arrow Up", Circle, Blue},   {"Highway", Square, Blue},
    {"Car", Square, Blue},        {"Forbidden", Circle, Red},
    {"Stop", Octagon, Red},
};

Color vc_find_color(IVC *src) {
  if (!src)
    fatal("vc_find_color: src == null\n");
  if (!vc_is_rgb(src))
    fatal("vc_find_color: vc_is_rgb(src) failed\n");

  IVC *hsv = vc_rgb_new(src->width, src->height);
  if (!hsv) {
    fatal("vc_find_color: allocation of new RGB image failed\n");
  }

  // converter a imagem de entrada para HSV
  if (vc_rgb_to_hsv(src, hsv) != EXIT_SUCCESS) {
    fatal("vc_find_color: vc_rgb_to_hsv failed!\n");
  }

#ifdef DEBUG
  vc_write_image_info("out/hsv.ppm", hsv);
#endif

  // calcular o histograma do canal H
  long pos, hue_hist[256] = {0};
  for (int x = 0; x < hsv->width; x++) {
    for (int y = 0; y < hsv->height; y++) {
      pos = y * hsv->bytesperline + x * hsv->channels;
      hue_hist[hsv->data[pos]]++;
    }
  }

  // identificar a cor dominante no histograma
  for (int x = 0; x < 256; x++) {
    // TODO:
  }

  return UnknownColor;
}

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

  if (!vc_gray_to_binary_otsu(src, tmp[0])) {
    fatal("vc_find_shape: vc_gray_to_binary_otsu failed\n");
  }

  int nblobs = 0; // número de blobs identificados, inicialmente a zero
  OVC *blobs = vc_binary_blob_labelling(tmp[0], tmp[1], &nblobs);
  if (!blobs) {
    fatal("vc_find_shape: vc_binary_blob_labelling failed\n");
  }

#ifdef DEBUG
  printf("\nNumber of labels (before filtering): %d\n", nblobs);
#endif

  if (!vc_binary_blob_info(tmp[1], blobs, nblobs)) {
    fatal("vc_find_shape: vc_binary_blob_info failed\n");
  }

  nblobs = vc_binary_blob_filter(&blobs, nblobs, 30);
  if (nblobs == -1) {
    fatal("vc_find_shape: vc_binary_blob_filter failed\n");
  }

#ifdef DEBUG
  printf("Number of labels (after filtering): %d\n", nblobs);
#endif

  for (i = 0; i < nblobs; i++) {
    vc_binary_blob_print(&blobs[i]);
    printf("\n");
    printf("blob %d is probably a %d\n", blobs[i].label,
           vc_identify_shape(&blobs[i], 0.2f));
    // vc_draw_mass_center(tmp[1], blobs[i].xc, blobs[i].yc, 255);
    // vc_draw_boundary_box(tmp[1], blobs[i].x, blobs[i].x + blobs[i].width,
    // blobs[i].y, blobs[i].y + blobs[i].height, 255);
  }

  free(blobs);

  return UnknownShape;
}
