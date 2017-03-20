#include "sign.h"
#include "stdlib.h"

// Sinais de transito conhecidos
Sign known_signs[7] = {
    {"Arrow Left", Circle, Blue}, {"Arrow Right", Circle, Blue},
    {"Arrow Up", Circle, Blue},   {"Highway", Square, Blue},
    {"Car", Square, Blue},        {"Forbidden", Circle, Red},
    {"Stop", Octagon, Red},
};

Color vc_find_color(IVC *src) {
  if (!src)
    return UnknownColor;
  if (!vc_is_rgb(src))
    return UnknownColor;

  IVC *hsv = vc_rgb_new(src->width, src->height);
  if (!hsv) {
    return UnknownColor;
  }

  // converter para hsv
  if (vc_rgb_to_hsv(src, hsv) != EXIT_SUCCESS) {
    fprintf(stderr, "vc_find_color: vc_rgb_to_hsv failed!\n");
    return UnknownColor;
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
  }

  return 1;
}

Shape vc_find_shape(IVC *src) {
  IVC *tmp[2];
  tmp[0] = vc_grayscale_new(src->width, src->height);
  tmp[1] = vc_grayscale_new(src->width, src->height);

  vc_gray_to_binary_otsu(src, tmp[0]);

  int nblobs = 0;
  OVC *blobs = vc_binary_blob_labelling(tmp[0], tmp[1], &nblobs);

  if (blobs != NULL) {
    printf("\nNumber of labels (before filter): %d\n", nblobs);
    vc_binary_blob_info(tmp[1], blobs, nblobs);
    nblobs = vc_binary_blob_filter(&blobs, nblobs, 30);
    printf("Number of labels (after filter): %d\n", nblobs);
    for (int i = 0; i < nblobs; i++) {
      vc_binary_blob_print(&blobs[i]);
      printf("\n");
      printf("blob %d is probably a %d\n", blobs[i].label,
             vc_identify_shape(&blobs[i], 0.2f));
      // vc_draw_mass_center(tmp[1], blobs[i].xc, blobs[i].yc, 255);
      // vc_draw_boundary_box(tmp[1], blobs[i].x, blobs[i].x + blobs[i].width,
      // blobs[i].y, blobs[i].y + blobs[i].height, 255);
    }

    free(blobs);
  }

  return UnknownShape;
}
