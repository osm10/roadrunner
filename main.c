#include "myvc.h"
#include "sign.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

#define MAXIMAGES 50

int vc_rgb_to_hsv_b(IVC *srcdst)
{
	unsigned char *data = (unsigned char *) srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	float r, g, b, hue, saturation, value;
	float rgb_max, rgb_min;
	int i, size;

	// Verifica��o de erros
	if((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if(channels != 3) return 0;
	
	size = width * height * channels;
	
	for(i=0; i<size; i=i+channels)
	{
		r = (float) data[i];
		g = (float) data[i + 1];
		b = (float) data[i + 2];
		
		// Calcula valores máximo e mínimo dos canais de cor R, G e B
		rgb_max = (r > g ? (r > b ? r : b) : (g > b ? g : b));
		rgb_min = (r < g ? (r < b ? r : b) : (g < b ? g : b));
		
		// Value toma valores entre [0,255]
		value = rgb_max;
		if(value == 0.0)
		{
			hue = 0.0;
			saturation = 0.0;
		}
		else
		{
			// Saturation toma valores entre [0,255]
			saturation = ((rgb_max - rgb_min) / rgb_max) * (float) 255.0;

			if(saturation == 0.0)
			{
				hue = 0.0;
			}
			else
			{
				// Hue toma valores entre [0,360]
				if((rgb_max == r) && (g >= b))
				{
					hue = 60.0f * (g - b) / (rgb_max - rgb_min);
				}
				else if((rgb_max == r) && (b > g))
				{
					hue = 360.0f + 60.0f * (g - b) / (rgb_max - rgb_min);
				}
				else if(rgb_max == g)
				{
					hue = 120 + 60 * (b - r) / (rgb_max - rgb_min);
				}
				else  // rgb_max == b
				{
					hue = 240.0f + 60.0f * (r - g) / (rgb_max - rgb_min);
				}
			}
		}
		
		// Atribui valores entre [0,255]
		/*data[i] = (unsigned char) (hue / 360.0 * 255.0);
		data[i + 1] = (unsigned char) (saturation);
		data[i + 2] = (unsigned char) (value);*/

		data[i]=0.0;
		data[i+1] =0.0;
		data[i+2]=0.0;
		
		//segmentar apenas os pixeis
		//H:60 S:100 V:100
		//H:43 S:100 V:90
		// para encontar os bonecos amarelados
		if((hue > 220) && (hue<=230) && (saturation/255.0*100.0 >=80) && (value/255.0*100.0 >=49))
		{
			data[i]=255; data[i+1]=255; data[i+2] =255;
		}
	}
	return 1;
}

// Calcula a circularidade de um blob.
float circularity(OVC *blob) {
  if (!blob)
    return -1.0f;
  return (12.5663706 * blob->area) / (blob->perimeter * blob->perimeter);
}

// Função auxiliar para comparar dois blobs por area.
int compare_area(const void *a, const void *b) {
  return (((OVC *)b)->area - (((OVC *)a)->area));
}

int color_segmentation(IVC *src) {
  vc_rgb_to_hsv_b(src);
  if (!vc_write_image_info("out/blue.ppm", src)) {
    error("process_file2: vc_write_image_info failed\n");
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
    printf("Circularity: %.2f\n\n", circularity(blob));
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
  //Color color = vc_find_color(src);
  // vc_color_print(color);
  //Shape shape = vc_find_shape(src);
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
