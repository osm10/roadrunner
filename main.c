#include "myvc.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

#define MAXIMAGES 10

sign_color_e vc_find_color(IVC *src) {
  if (!src) return unknown_color;
  if (!vc_is_rgb(src)) return unknown_color;
 
  IVC *hsv = vc_rgb_new(src->width, src->height);
  if (!hsv) {
    return unknown_color;
  }

  // converter para hsv
  if (vc_rgb_to_hsv(src, hsv) != EXIT_SUCCESS) {
    fprintf(stderr, "vc_find_color: vc_rgb_to_hsv failed!\n");
    return unknown_color;
  }
#ifdef DEBUG
  vc_write_image_info("out/hsv.ppm", hsv);
#endif

  //calcular o histograma do canal H
  long pos, hue_hist[256] = { 0 };
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

int process_file(const char *path) {
  IVC *src = vc_read_image((char *)path);
  //sign_color_e color = vc_find_color(src);
  //sign_shape_e shape = vc_find_shape(src);
  vc_image_free(src);
  return 1;
}

// Imprime informação de utilização
void usage(char *program) {
  fprintf(stderr, "Usage: %s [file or directory]\n", program);
}

char **alloc_images(size_t nimages, size_t nchar) {
  char **images = calloc(nimages, sizeof(char *));
  if (!images)
    return NULL;
  for (size_t i = 0; i < nimages; i++) {
    images[i] = malloc((nchar + 1) * sizeof(char));
    // TODO: treat malloc failure
  }
  return images;
}

void free_images(char *images[], size_t nimages) {
  for (size_t i = 0; i < nimages; i++) {
    free(images[i]);
  }
  free(images);
}
/*
int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Error: not enough arguments\n");
    usage(argv[0]);
    return EXIT_FAILURE;
  }

  // caminho do ficheiro ou pasta dado pelo utilizador
  const char *path = argv[1];

  // testamos se o argumento é um ficheiro
  if (is_regular_file(path)) {
    if (!process_file(path)) {
      fprintf(stderr, "main: process_file failed\n");
      return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
  }

  // alocação de memória para MAXIMAGES com 100 caracteres cada
  char **images = alloc_images(MAXIMAGES, 100);
  size_t nimages = 0; // número de imagens encontradas

  // testamos se o argumento é um diretório
  if (is_directory(path)) {
    nimages = get_images_from_dir(path, images, MAXIMAGES);
    printf("nimages: %ld\n", nimages);
    if (nimages < 1) {
      fprintf(stderr, "main: get_images_from_dir found no images\n");
      free_images(images, MAXIMAGES);
      return EXIT_FAILURE;
    }
    for (size_t i = 0; i < nimages; i++) {
      printf("image: %s\n", images[i]);
    }
  }

  // libertar o espaço
  free_images(images, MAXIMAGES);

  return EXIT_SUCCESS;
}
*/
int main(void) {
  IVC *image[3];
  int i, nblobs;
  OVC *blobs;

  image[0] = vc_read_image("samples/others/flir-01.pgm");
  if (image[0] == NULL) {
    printf("ERROR -> vc_read_image():\n\tFile not found!\n");
    getchar();
    return 0;
  }

  image[1] = vc_image_new(image[0]->width, image[0]->height, 1, 255);
  image[2] = vc_image_new(image[0]->width, image[0]->height, 1, 255);
  if (image[1] == NULL) {
    printf("ERROR -> vc_image_new():\n\tOut of memory!\n");
    getchar();
    return 0;
  }

  vc_gray_to_binary_fixed(image[0], image[1], 127);
  blobs = vc_binary_blob_labelling(image[1], image[2], &nblobs);

  if (blobs != NULL) {
    printf("\nNumber of labels (before filter): %d\n", nblobs);
    vc_binary_blob_info(image[2], blobs, nblobs);
    nblobs = vc_binary_blob_filter(&blobs, nblobs, 30);
    printf("Number of labels (after filter): %d\n", nblobs);
    for (i = 0; i < nblobs; i++) {
      vc_binary_blob_print(&blobs[i]);
      printf("\n");
      vc_draw_mass_center(image[2], blobs[i].xc, blobs[i].yc, 255);
      vc_draw_boundary_box(image[2], blobs[i].x, blobs[i].x + blobs[i].width,
                           blobs[i].y, blobs[i].y + blobs[i].height, 255);
    }

    free(blobs);
  }

  vc_write_image("out/vc0022.pgm", image[2]);

  vc_image_free(image[0]);
  vc_image_free(image[1]);
  vc_image_free(image[2]);

  return 0;
}
