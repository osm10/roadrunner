#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "color.h"
#include "edges.h"
#include "myvc.h"
#include "sign.h"
#include "utils.h"

#define MAXIMAGES 50

// Função auxiliar para comparar dois blobs por area.
int compare_area(const void *a, const void *b) {
  return (((OVC *)b)->area - (((OVC *)a)->area));
}

int shape_segmentation(IVC *src) {
  IVC *gray = vc_grayscale_new(src->width, src->height);
  IVC *segm = vc_grayscale_new(src->width, src->height);
  IVC *dst = vc_grayscale_new(src->width, src->height);

  if (!vc_rgb_to_gray(src, gray))
    return 0;
  if (!vc_write_image_info("out/gray.pgm", gray)) {
    error("process_file2: vc_write_image_info failed\n");
  }

  if (!vc_gray_to_binary_otsu(gray, segm))
    return 0;
  if (!vc_write_image_info("out/segm.pgm", segm)) {
    error("process_file2: vc_write_image_info failed\n");
  }

  int nblobs = 0;
  OVC *blobs = vc_binary_blob_labelling(segm, dst, &nblobs);
  if (!blobs)
    return 0;

  vc_binary_blob_info(dst, blobs, nblobs);

  printf("nblobs: %d\n", nblobs);

  if (!vc_write_image_info("out/blobbed.pgm", dst)) {
    error("process_file2: vc_write_image_info failed\n");
  }

  qsort(blobs, nblobs, sizeof(OVC),
        compare_area); // filtrar por area decrescente

  // saltamos o maior (i=1)
  for (int i = 0; i < nblobs; i++) {
    OVC *blob = &blobs[i];
    vc_binary_blob_print(blob);
    printf("Circularity: %.2f\n\n", blob->circularity);
    if (!vc_draw_boundary_box(dst, blob->x, blob->x + blob->width, blob->y,
                              blob->y + blob->height, 255))
      return 0;
    if (!vc_draw_mass_center(dst, blob->xc, blob->yc, 255))
      return 0;
  }

  if (!vc_write_image_info("out/bounded.pgm", dst)) {
    error("process_file2: vc_write_image_info failed\n");
  }

  // nblobs = vc_binary_blob_filter(&blobs, nblobs, 30);

  vc_image_free(src);
  vc_image_free(gray);
  vc_image_free(segm);
  vc_image_free(dst);
  free(blobs);

  return 1;
}

int process_file(const char *path) {
  IVC *src = vc_read_image((char *)path);
  IVC *gray = vc_grayscale_new(src->width, src->height);
  IVC *edge = vc_grayscale_new(src->width, src->height);
  IVC *dst = vc_rgb_new(src->width, src->height);

  Color color = vc_find_color(src, dst);
#ifdef DEBUG
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

  if (!vc_gray_edge_sobel(gray, edge, 40)) {
    error("process_file: sobel edge detection failed\n");
  }
#ifdef DEBUG
  if (!vc_write_image_info("out/edge.pgm", edge)) {
    error("process_file: vc_write_image_info failed\n");
  }
#endif

  Shape shape = vc_find_shape(edge);
#ifdef DEBUG
  vc_shape_print(shape);
#endif

  Sign sign = vc_identify_sign(color, shape);
  if (strncmp(sign.name, "UnknownSign", 51) == 0) {
    printf("\nSinal não reconhecido\n");
  } else {
    printf("\nSinal reconhecido: %s\n", sign.name);
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
  const char *path = argv[1];

  // testamos se o argumento é um ficheiro
  if (is_regular_file(path)) {
    printf("Processing: %s\n", path);
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
      printf("Processing: %s\n", images[i]);
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
