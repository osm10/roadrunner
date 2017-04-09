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

int super_oliver(char *path) {
  IVC *src = vc_read_image(path);
  IVC *tmp = vc_grayscale_new(src->width,src->height);
  char *filename = "yeah";
  int i;

 int nblobs = 0; // número de blobs identificados, inicialmente a zero
  OVC *blobs = vc_binary_blob_labelling(src, tmp, &nblobs);
  if (!blobs) {
    fatal("vc_find_shape: vc_binary_blob_labelling failed\n");
  }

#ifdef DEBUG
  printf("\nFiltering labels by area.\n");
  printf("Number of labels (before filtering): %d\n", nblobs);
#endif

  if (!vc_binary_blob_info(tmp, blobs, nblobs)) {
    fatal("vc_find_shape: vc_binary_blob_info failed\n");
  }

  nblobs = vc_binary_blob_filter(&blobs, nblobs, 400);
  if (nblobs == -1) {
    fatal("vc_find_shape: vc_binary_blob_filter failed\n");
  }

#ifdef DEBUG
  printf("Number of labels (after filtering): %d\n", nblobs);
#endif

  for (i = 0; i < nblobs; i++) {
#ifdef DEBUG
    vc_binary_blob_print(&blobs[i]);
    printf("blob %d is probably a ", blobs[i].label);
    printf("%s\n", vc_shape_name(vc_identify_shape(&blobs[i], 0.2f)));
    printf("\n");
#endif
    vc_draw_mass_center(tmp, blobs[i].xc, blobs[i].yc, 255);
    vc_draw_boundary_box(tmp, blobs[i].x, blobs[i].x + blobs[i].width,
                         blobs[i].y, blobs[i].y + blobs[i].height, 255);
  }

  vc_write_image_info(concat(4, "out/", "blobbed_", filename, ".pgm"), tmp);


  free(blobs);
      vc_image_free(tmp);
  return 1;
}

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
  free((char*)filename);

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
    if (!super_oliver(path)) {
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
      if (!super_oliver(images[i])) {
        fprintf(stderr, "main: process_file on image `%s'\n", path);
      }
      getchar();
    }
  }

  // libertar o espaço
  free_images(images, MAXIMAGES);

  return 1;
}
