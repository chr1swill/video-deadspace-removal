#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <assert.h>
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

int filter_for_only_files(const struct dirent *entry) {
  if (entry->d_type != DT_REG) {
    return 0;
  }
  if ((strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)) {
    return 0;
  }
  return 1;
};

int main(int argc, char **argv) {
   if (argc != 2) {
       fprintf(stderr, "Usage: %s <dir_with_images_files>\n", argv[0]);
       return -1;
   }

   char *img_dir = argv[1];

   struct stat st = {0};
   int exist = stat(img_dir, &st);
   if (exist != 0) {
     fprintf(stderr, "Error getting stats of dir: %s\n", strerror(errno));
     return -1;
   }

   DIR *dir = opendir(img_dir);
   if (dir == NULL) {
     fprintf(stderr, "Error opening dir: %s\n", strerror(errno));
     return -1;
   }

   int number_of_files = 0;
   struct dirent *entry = {0};
   while ((entry = readdir(dir)) != NULL) {
     if (filter_for_only_files(entry) == 0) continue;
     number_of_files++;
   }

   if (number_of_files == 0) {
     fprintf(stderr, "The are no files in the provided directory: %s\n", img_dir);
     closedir(dir);
     return -1;
   } 
  
   struct dirent **namelist = malloc(number_of_files * sizeof(struct dirent));
   if (namelist == NULL) {
     fprintf(stderr, "Failed to allocate memory for name list.\n");
     closedir(dir);
     return -1;
   }

   int sorted_entries = scandir(img_dir, &namelist, filter_for_only_files, alphasort);
   if (sorted_entries == -1) {
     fprintf(stderr, "Error scanning dir: %s\n", strerror(errno));
     free(namelist);
     closedir(dir);
     return -1;
   }
   assert(sorted_entries == number_of_files);

  // for (int i = 0; i < sorted_entries; i++) {
  //   printf("entry = (%s)\n", namelist[i]->d_name);
  // }

   for (int i = 1; i < sorted_entries; i++) {

     int img1_buff_len = strlen(img_dir) + strlen(namelist[i-1]->d_name) + 2;
     char *img1_path = malloc(img1_buff_len * sizeof(char));
     if (img1_path == NULL) {
       fprintf(stderr, "Error allocating buffer for image paths\n");
       free(namelist);
       closedir(dir);
       return -1;
     }

     int img2_buff_len = strlen(img_dir) + strlen(namelist[i]->d_name) + 2;
     char *img2_path = malloc(img2_buff_len * sizeof(char));
     if (img2_path == NULL) {
       fprintf(stderr, "Error allocating buffer for image paths\n");
       free(namelist);
       closedir(dir);
       free(img1_path);
       return -1;
     }

     int bytes_wrote1 = snprintf(img1_path, img1_buff_len, "%s/%s", img_dir, namelist[i-1]->d_name);
     if (bytes_wrote1 < 0) {
       fprintf(stderr, "Failed to load path form image into buffer image 1.\n");
       free(namelist);
       closedir(dir);
       free(img1_path);
       free(img2_path);
       return -1;
     }

     int bytes_wrote2 = snprintf(img2_path, img2_buff_len, "%s/%s", img_dir, namelist[i]->d_name);
     if (bytes_wrote2 < 0) {
       fprintf(stderr, "Failed to load path form image into buffer for image 2.\n");
       free(namelist);
       closedir(dir);
       free(img1_path);
       free(img2_path);
       return -1;
     }

     printf("img1 path = (%s), img2 path = (%s)\n", img1_path, img2_path);
     //unsigned char *img1 = load_png(img1_path);
     //unsigned char *img2 = load_png(img2_path);

     //if (img1 == NULL || img2 == NULL) {
     //  fprintf(stderr, "Error loading images.\n");

     //  for (int i = 0; i < sorted_entries; i++) {
     //    free(namelist[i]);
     //  }
     //  free(namelist);
     //  closedir(dir);
     //  return 1;
     //}


     free(img1_path);
     free(img2_path);
   }

   for (int i = 0; i < sorted_entries; i++) {
     free(namelist[i]);
   }
   free(namelist);
   closedir(dir);
   return 0;

   unsigned char *img1 = load_png(img_dir);
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
