#include <dirent.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXIMAGES 10

static const char *const extensions[] = {"pbm", "pgm", "ppm", "pnm"};

// TODO: move to utils.c
char *concat(int count, ...) {
  va_list ap;
  int i;

  // Find required length to store merged string
  int len = 1; // room for NULL
  va_start(ap, count);
  for (i = 0; i < count; i++)
    len += strlen(va_arg(ap, char *));
  va_end(ap);

  // Allocate memory to concat strings
  char *merged = calloc(sizeof(char), len);
  int null_pos = 0;

  // Actually concatenate strings
  va_start(ap, count);
  for (i = 0; i < count; i++) {
    char *s = va_arg(ap, char *);
    strcpy(merged + null_pos, s);
    null_pos += strlen(s);
  }
  va_end(ap);

  return merged;
}

// Imprime informação de utilização
void usage(char *program) {
  fprintf(stderr, "Usage: %s [file or directory]\n", program);
}

// Verifa se um determinado caminho corresponde a um ficheiro normal.
int is_regular_file(const char *path) {
  struct stat path_stat;
  if (stat(path, &path_stat) != 0)
    return 0;
  return S_ISREG(path_stat.st_mode);
}

// Verifa se um determinado caminho corresponde a uma directório.
int is_directory(const char *path) {
  struct stat path_stat;
  if (stat(path, &path_stat) != 0)
    return 0;
  return S_ISDIR(path_stat.st_mode);
}

const char *get_filename_ext(const char *filename) {
  const char *dot = strrchr(filename, '.');
  if (!dot || dot == filename)
    return NULL;
  return dot + 1;
}

int has_netpbm_file_extension(const char *filename) {
  if (!filename)
    return 0;
  const char *extension = get_filename_ext(filename);
  if (!extension)
    return 0;
  // printf("extension: %s\n", extension);
  size_t nextensions = sizeof(extensions) / sizeof(const char *const);
  // para cada extensão de imagem do formato Netpbm
  for (size_t i = 0; i < nextensions; i++) {
    // ver se a extensão do ficheiro é igual a uma extensão Netpbm
    // printf("extensions[i]: %s\n", extensions[i]);
    if (strncmp(extension, extensions[i], 3) == 0)
      return 1;
  }
  return 0;
}

// Retorna o número de imagens encontradas em path
size_t get_images_from_dir(const char *path, char **images, size_t nimages) {
  struct dirent *dir;
  DIR *d = opendir(path);
  if (!d)
    return -1;

  char *filename, *filepath;
  size_t i = 0;

  while ((dir = readdir(d)) != NULL) {
    filename = dir->d_name;
    if (has_netpbm_file_extension(filename) == 1) {
      // printf("VALID: %s\n", filename);
      filepath = concat(3, path, "/", filename);
      // printf("filepath: %s\n", filepath);
      strncpy(images[i], filepath, strlen(filepath));
      free(filepath);
      i++;
    }
  }

  closedir(d);

  return i;
}

char** alloc_images(size_t nimages, size_t nchar) {
  char **images = calloc(nimages, sizeof(char *));
  if (!images) return NULL;
  for (size_t i = 0; i < nimages; i++) {
    images[i] = malloc((nchar+1) * sizeof(char));
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
    return EXIT_FAILURE;
  }

  // caminho do ficheiro ou pasta dado pelo utilizador
  const char *path = argv[1];

  // testamos se o argumento é um ficheiro
  if (is_regular_file(path)) {
    // TODO: process_file
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
