#ifndef UTILS
#define UTILS

static const char *const extensions[] = {"pbm", "pgm", "ppm", "pnm"};

char *concat(int count, ...);
int is_regular_file(const char *path);
int is_directory(const char *path);
const char *get_filename_ext(const char *filename);
int has_netpbm_file_extension(const char *filename);
size_t get_images_from_dir(const char *path, char **images, size_t nimages);

#endif /* UTILS */
