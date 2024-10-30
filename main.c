#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <png.h>

#define WIDTH 1920
#define HEIGHT 1080
#define CHANNELS 3  // RGB

// Function to load a PNG into memory
unsigned char* load_png(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return NULL;
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);
    png_init_io(png, fp);
    png_read_info(png, info);

    int width = png_get_image_width(png, info);
    int height = png_get_image_height(png, info);
    int color_type = png_get_color_type(png, info);

    png_bytep *rows = malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; y++) {
        rows[y] = malloc(png_get_rowbytes(png, info));
    }
    png_read_image(png, rows);
    fclose(fp);

    unsigned char *data = malloc(WIDTH * HEIGHT * CHANNELS);
    for (int y = 0; y < height; y++) {
        memcpy(data + y * WIDTH * CHANNELS, rows[y], WIDTH * CHANNELS);
        free(rows[y]);
    }
    free(rows);
    png_destroy_read_struct(&png, &info, NULL);

    return data;
}

// Function to compare two images
int compare_images(unsigned char *img1, unsigned char *img2) {
    int diff = 0;
    for (int i = 0; i < WIDTH * HEIGHT * CHANNELS; i++) {
        diff += abs(img1[i] - img2[i]);
    }
    return diff;
}

int main(int argc, char **argv) {
   if (argc != 2) {
       fprintf(stderr, "Usage: %s <dir_with_images_files>\n", argv[0]);
       return 1;
   }

   struct stat st = {0};
   int exist = stat(argv[1], &st);
   if (exist != 0) {
     fprintf(stderr, "Error getting stats of dir: %s\n", strerror(errno));
     return -1;
   }
   printf("Valid dir %s\n", argv[1]);

   DIR *dir = opendir(argv[1]);
   if (dir == NULL) {
     fprintf(stderr, "Error opening dir: %s\n", strerror(errno));
     return -1;
   }

   struct dirent *current_entry = {0};
   while ((current_entry = readdir(dir)) != NULL) {
     // skip directories
     if (current_entry->d_type != DT_REG) continue;
     // skip current direct and parent directory
     if ((strcmp(current_entry->d_name, ".")) == 0) continue;
     if ((strcmp(current_entry->d_name, "..")) == 0) continue;

     DIR *hold_dir_p = dir;
     struct dirent *next_entry = {0};
     if ((next_entry = readdir(dir)) != NULL) {
       // skip directories
       if (current_entry->d_type != DT_REG) {
         dir = hold_dir_p;
         continue;
       }
       
       // skip current direct and parent directory
       if ((strcmp(current_entry->d_name, ".")) == 0) {
         dir = hold_dir_p;
         continue;
       }

       if ((strcmp(current_entry->d_name, "..")) == 0) {
         dir = hold_dir_p;
         continue;
       }

       printf("comparing current dirname = (%s) + next dirname = (%s)\n", current_entry->d_name, next_entry->d_name);
     }

     // reset dir for next iteration to prevent skipping dir entries for comparing
     dir = hold_dir_p;
   }

   if ((closedir(dir)) != 0) {
     fprintf(stderr, "Error closing dir: %s\n", strerror(errno));
     return -1;
   }

   return 0;

   unsigned char *img1 = load_png(argv[1]);
   unsigned char *img2 = load_png(argv[2]);

   if (!img1 || !img2) {
     fprintf(stderr, "Error loading images.\n");
     return 1;
   }

   int diff = compare_images(img1, img2);
   printf("Difference: %d\n", diff);

   free(img1);
   free(img2);
   return 0;
}
