#ifndef _LIBZ_H 
#define _LIBZ_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <assert.h>
#include <unistd.h>
#include <png.h>

#define WIDTH 1920
#define HEIGHT 1080
#define CHANNELS 3  // RGB

unsigned char* load_png_v2(const char *filename);

// Function to load a PNG into memory
unsigned char* load_png(const char *filename);

#define TOO_DIFF 100

// Function to compare two images
int compare_images(unsigned char *img1, unsigned char *img2);

int filter_for_only_files(const struct dirent *entry);

char *get_img_path(char *img_dir, char *d_name);

void free_namelist(struct dirent **namelist, int length);

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

typedef const char *VideoFileFormatExtensions[];

extern VideoFileFormatExtensions video_file_extensions;

void display_allowed_file_ext(void);

#endif //_LIBZ_H 
