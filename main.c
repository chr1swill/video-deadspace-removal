#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <assert.h>
#include <unistd.h>
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

static inline void free_namelist(struct dirent **namelist, int length) {
  for (int i = 0; i < length; i++) {
    free(namelist[i]);
  }
  free(namelist);
}

#define MAX_INPUT_LEN 256

typedef enum {
    MP4,
    MKV,
    AVI,
    MOV,
    FLV,
    WMV,
    MPEG,
    WEBM,
    M4V,
    TS,
    MTS,
    VOB,
    OGV,
    GIF,
    F4V,
    RM,
    DIVX,
    XVID,
    ASF
} VideoFileFormat;

#define DELIMITER "DELIMITER"

const char *VideoFileFormatExtensions[] = {
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

static inline void display_allowed_file_ext(void){
    printf("Allow input file extentions:\n");
    for (int i = 0; ((strcmp(VideoFileFormatExtensions[i], DELIMITER))) != 0; i++) {
      printf("\t.%s\n", VideoFileFormatExtensions[i]);
    }
}

int main(int argc, char **argv) {
  // first read the name of the vide file from the command line

  char *input_file_path = (char *)malloc(MAX_INPUT_LEN * sizeof(char));
  if (input_file_path == NULL) {
    fprintf(stderr, "Error allocating memory for the input file path\n");
    return -1;
  }

  printf("Enter in you file name:\n");
  fflush(STDIN_FILENO);

  ssize_t bytes_read;
  size_t i = 0; 
  while (i < (MAX_INPUT_LEN -1 ) && 
        (bytes_read = read(STDIN_FILENO, &input_file_path[i], 1)) > 0) {
    assert(i < (MAX_INPUT_LEN - 1));
    if (input_file_path[i] == '\n') break; 
    i++;
  }

  input_file_path[i] = '\0';

  struct stat st = {0};
  int real_file = stat(input_file_path, &st);
  if (real_file != 0) {
    fprintf(stderr, "Could not find file: (%s)\n", input_file_path);
    free(input_file_path);
    return -1;
  }

  char *file_ext = strstr(input_file_path, ".");
  if (file_ext == NULL) {
    fprintf(stderr, "Error provided file is an unsupported file type: %s\n", input_file_path);

    display_allowed_file_ext();
    free(input_file_path);
    return -1;
  }

  printf("%s\n", file_ext);

  int found = -1;
  for (int i = 0; ((strcmp(VideoFileFormatExtensions[i], DELIMITER)) != 0) ; i++) {
    if ((strcmp(VideoFileFormatExtensions[i], file_ext) == 0)) {
      found = 0;
      break;
    }
  } 

  if (found != 0) {
    fprintf(stderr, "Error provided file is an unsupported file type: %s\n", input_file_path);

    display_allowed_file_ext();
    free(input_file_path);
    return -1;
  }

  free(input_file_path);
  return 0;


  // then fork that off into ffmpeg
  // if it is successfuly you can run the app but you choose the dir that it output too instead of the user choosing
  // we make the images into the dir we created
  // after that we use ffmpeg to create a new new ouptut video file, called output or something, ensureing first that there is no file with that name already

   if (argc != 2) {
       fprintf(stderr, "Usage: %s <dir_with_images_files>\n", argv[0]);
       return -1;
   }

   char *img_dir = argv[1];

   //struct stat st = {0};
   int exist = stat(img_dir, &st);
   if (exist != 0) {
     fprintf(stderr, "Error invalid directory path %s: %s\n", img_dir, strerror(errno));
     return -1;
   }

   DIR *dir = opendir(img_dir);
   if (dir == NULL) {
     fprintf(stderr, "Error opening dir %s: %s\n",  img_dir, strerror(errno));
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
     fprintf(stderr, "Error scanning dir %s: %s\n",  img_dir, strerror(errno));

     free_namelist(namelist, sorted_entries);
     closedir(dir);
     return -1;
   }
   assert(sorted_entries == number_of_files);

   for (int i = 1; i < sorted_entries; i++) {
     char *first_entry = namelist[i-1]->d_name;
     char *img1_path = get_img_path(img_dir, first_entry);
     if (img1_path == NULL) {
       fprintf(stderr, "Error forming image path for entry name %s\n", first_entry);

       free_namelist(namelist, sorted_entries);
       closedir(dir);
       return -1;
     }

     char *second_entry = namelist[i]->d_name;
     char *img2_path = get_img_path(img_dir, second_entry);
     if (img2_path == NULL) {
       fprintf(stderr, "Error forming image path for entry name %s\n", second_entry);

       free_namelist(namelist, sorted_entries);
       closedir(dir);
       return -1;
     }

     unsigned char *img1 = load_png(img1_path);
     unsigned char *img2 = load_png(img2_path);

     if (img1 == NULL || img2 == NULL) {
       fprintf(stderr, "Error loading images.\n");

       free_namelist(namelist, sorted_entries);
       closedir(dir);
       free(img1_path);
       free(img2_path);
       return 1;
     }

     int diff = compare_images(img1, img2); 
     if (diff == 0) {
       // delete first path
       printf("removing file %s\n", img1_path);
       if ((remove(img1_path)) == -1) {
         fprintf(stderr, "Error occured removing %s: %s\n", img1_path, strerror(errno));
       }
     } else {
       printf("frames %s and %s are the this different = %d\n", img1_path, img2_path, diff);
     }

     free(img1_path);
     free(img1);
     free(img2_path);
     free(img2);
   }

   free_namelist(namelist, sorted_entries);
   closedir(dir);
   return 0;
}
