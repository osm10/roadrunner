#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "color.h"
#include "edges.h"
#include "myvc.h"
#include "sign.h"
#include "utils.h"

#define MAXIMAGES 50

int process_file(char *path) {
  IVC *src = vc_read_image(path);
  IVC *gray = vc_grayscale_new(src->width, src->height);
  IVC *edge = vc_grayscale_new(src->width, src->height);
  IVC *dst = vc_rgb_new(src->width, src->height);
  const char *filename = get_filename_no_ext(basename(path));

  printf("A identificar `%s'\n", path);

  Color color = vc_find_color(src, dst);
#ifdef DEBUG
  printf("Cor: %s\n", vc_color_name(color));
  if (!vc_write_image_info("out/color_segm.ppm", dst)) {
    error("process_file: vc_write_image_info failed\n");
  }
#endif

  if (!vc_rgb_to_gray(dst, gray)) {
    error("process_file: convertion to grayscale failed\n");
  }
#ifdef DEBUG
  if (!vc_write_image_info("out/color_segm.pgm", gray)) {
    error("process_file: vc_write_image_info failed\n");
  }
#endif

  if (!vc_gray_edge_prewitt(gray, edge, 40)) {
    error("process_file: sobel edge detection failed\n");
  }
#ifdef DEBUG
  if (!vc_write_image_info("out/edge.pgm", edge)) {
    error("process_file: vc_write_image_info failed\n");
  }
#endif

  Shape shape = vc_find_shape(edge, filename);
#ifdef DEBUG
  printf("Forma: %s\n", vc_shape_name(shape));
#endif

  Sign sign = vc_identify_sign(color, shape);
  if (strncmp(sign.name, "UnknownSign", 51) == 0) {
    printf("\nSinal não reconhecido\n");
  } else {
    printf("\nSinal reconhecido:\n");
    vc_sign_print(&sign);
  }

  vc_image_free(src);
  vc_image_free(gray);
  vc_image_free(edge);
  vc_image_free(dst);

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

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Error: not enough arguments\n");
    usage(argv[0]);
    return 0;
  }

  // caminho do ficheiro ou pasta dado pelo utilizador
  char *path = argv[1];

  // testamos se o argumento é um ficheiro
  if (is_regular_file(path)) {
    if (!process_file(path)) {
      fprintf(stderr, "main: process_file failed\n");
      return 0;
    }
    return 1;
  }

  // alocação de memória para MAXIMAGES com 100 caracteres cada
  char **images = alloc_images(MAXIMAGES, 100);
  size_t nimages = 0; // número de imagens encontradas

  // testamos se o argumento é um diretório
  if (is_directory(path)) {
    nimages = get_images_from_dir(path, images, MAXIMAGES);
    printf("Number of images to process: %ld\n", nimages);
    if (nimages < 1) {
      fprintf(stderr, "main: get_images_from_dir found no images\n");
      free_images(images, MAXIMAGES);
      return 0;
    }
    for (size_t i = 0; i < nimages; i++) {
      if (!process_file(images[i])) {
        fprintf(stderr, "main: process_file on image `%s'\n", path);
      }
      getchar();
    }
  }

  // libertar o espaço
  free_images(images, MAXIMAGES);

  return 1;
}
