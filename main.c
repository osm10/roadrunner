#include "myvc.h"
#include "sign.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

#define MAXIMAGES 50

#define MAX3(a, b, c) (a > b ? (a > c ? a : c) : (b > c ? b : c))
#define MIN3(a, b, c) (a < b ? (a < c ? a : c) : (b < c ? b : c))

// hue:0-360; saturation:0-100: value:0-360
int vc_rgb_to_hsv_by_color(IVC *src, IVC *dst, int hmin, int hmax, int smin,
                           int smax, int vmin, int vmax) {
  int x, y;
  long pos;
  float rf, gf, bf;
  float hue, saturation, value;
  float max, min;

  // verificação de erros
  if (src->width <= 0 || src->height <= 0 || src->data == NULL)
    return 0;
  if (dst->width != src->width || dst->height != src->height)
    return 0;
  if (src->channels != 3 || dst->channels != 3)
    return 0;

  for (x = 0; x < src->width; x++) {
    for (y = 0; y < src->height; y++) {
      pos = y * src->bytesperline + x * src->channels;

      rf = (float)src->data[pos];
      gf = (float)src->data[pos + 1];
      bf = (float)src->data[pos + 2];

      max = MAX3(rf, gf, bf);
      min = MIN3(rf, gf, bf);

      value = max;
      if (value == 0.0) {
        saturation = 0.0;
        hue = 0.0;
      } else {
        saturation = ((value - min) / value) * 255.0f;
        if (saturation == 0.0) {
          hue = 0.0;
        } else {
          if ((max == rf) && (gf >= bf))
            hue = 60.0f * (gf - bf) / (max - min);
          else if ((max == rf) && (gf < bf))
            hue = 360.0f + 60.0f * (gf - bf) / (max - min);
          else if (max == gf)
            hue = 120.0f + 60.0f * (bf - rf) / (max - min);
          else
            hue = 240.0f + 60.0f * (rf - gf) / (max - min); // max == b
        }
      }

      dst->data[pos] = 0;
      dst->data[pos + 1] = 0;
      dst->data[pos + 2] = 0;

      if (hmin < hmax) {
        // segmentar apenas os pixeis
        if ((hue >= hmin && hue <= hmax) && (saturation * 100 / 255 >= smin &&
                                             saturation * 100 / 255 <= smax) &&
            (value * 100 / 255 >= vmin && value * 100 / 255 <= vmax)) {
          dst->data[pos] = 255;
          dst->data[pos + 1] = 255;
          dst->data[pos + 2] = 255;
        }
      } else {
        if ((hue >= hmin || hue <= hmax) && (saturation * 100 / 255 >= smin &&
                                             saturation * 100 / 255 <= smax) &&
            (value * 100 / 255 >= vmin && value * 100 / 255 <= vmax)) {
          dst->data[pos] = 255;
          dst->data[pos + 1] = 255;
          dst->data[pos + 2] = 255;
        }
      }
    }
  }
  return 1;
}

int vc_rgb_to_hsv_b(IVC *src, IVC *dst) {
  return vc_rgb_to_hsv_by_color(src, dst, 220, 230, 70, 100, 40, 260);
}

int vc_rgb_to_hsv_r(IVC *src, IVC *dst) {
  return vc_rgb_to_hsv_by_color(src, dst, 350, 10, 60, 100, 50, 360);
}

int vc_rgb_to_hsv_y(IVC *src, IVC *dst) {
  return vc_rgb_to_hsv_by_color(src, dst, 50, 60, 90, 100, 90, 360);
}

// Função auxiliar para comparar dois blobs por area.
int compare_area(const void *a, const void *b) {
  return (((OVC *)b)->area - (((OVC *)a)->area));
}

int color_segmentation(IVC *src) {
  IVC *dst = vc_image_new(src->width, src->height, src->channels, src->levels);
  // sinais vermelhos
  vc_rgb_to_hsv_r(src, dst);
  if (!vc_write_image_info("out/red.ppm", dst)) {
    error("color_segmentation: vc_write_image_info failed\n");
  }
  return 1;
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
  // Color color = vc_find_color(src);
  // vc_color_print(color);
  // Shape shape = vc_find_shape(src);
  color_segmentation(src);
  shape_segmentation(src);
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
