#include "libz.h"

unsigned char* load_png_v2(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return NULL;
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);
    png_init_io(png, fp);
    png_read_info(png, info);

    //int height = png_get_image_height(png, info);

    //png_bytep *rows = malloc(sizeof(png_bytep) * height);
    png_bytep rows[HEIGHT];

    for (int y = 0; y < HEIGHT; y++) {
        rows[y] = malloc(png_get_rowbytes(png, info));
    }
    png_read_image(png, rows);
    fclose(fp);

    unsigned char *data = malloc(WIDTH * HEIGHT * CHANNELS);
    for (int y = 0; y < HEIGHT; y++) {
        memcpy(data + y * WIDTH * CHANNELS, rows[y], WIDTH * CHANNELS);
        //free(rows[y]);
    }
    //free(rows);
    png_destroy_read_struct(&png, &info, NULL);

    return data;
}

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

    int height = png_get_image_height(png, info);

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
      if (diff < TOO_DIFF) {
        diff += abs(img1[i] - img2[i]);
        continue;
      } else {
        break;
      }
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

char *get_img_path(char *img_dir, char *d_name) {
  int img1_buff_len = strlen(img_dir) + strlen(d_name) + 2;
  char *img1_path = malloc(img1_buff_len * sizeof(char));
  if (img1_path == NULL) {
    fprintf(stderr, "Error allocating buffer for image paths\n");
    return NULL;
  }

  int bytes_wrote1 = snprintf(img1_path, img1_buff_len, "%s/%s", img_dir, d_name);
  if (bytes_wrote1 < 0) {
    fprintf(stderr, "Error form image path by joining %s + / + %s.\n", img_dir, d_name);
    return NULL;
  }

  return img1_path;
}

void free_namelist(struct dirent **namelist, int length) {
  for (int i = 0; i < length; i++) {
    free(namelist[i]);
  }
  free(namelist);
}

VideoFileFormatExtensions video_file_extensions = {
    ".mp4",
    ".mkv",
    ".avi",
    ".mov",
    ".flv",
    ".wmv",
    ".mpeg",
    ".webm",
    ".m4v",
    ".ts",
    ".mts",
    ".vob",
    ".ogv",
    ".gif",
    ".f4v",
    ".rm",
    ".divx",
    ".xvid",
    ".asf",
    DELIMITER
};

void display_allowed_file_ext(void){
    printf("Allow input file extentions:\n");
    for (int i = 0; ((strcmp(video_file_extensions[i], DELIMITER))) != 0; i++) {
      printf("\t.%s\n", video_file_extensions[i]);
    }
}

