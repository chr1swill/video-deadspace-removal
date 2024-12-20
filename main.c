#include "libz.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
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
  for (int i = 0; ((strcmp(video_file_extensions[i], DELIMITER)) != 0) ; i++) {
    if ((strcmp(video_file_extensions[i], file_ext) == 0)) {
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

  char *output_dir = "./output";
  // then fork that off into ffmpeg
  char *digits_in_img_paths = "%09d";
  char *command = "rm -rf output && mkdir -p output && ffmpeg -i %s %s/%s.png";
  int command_buff_len = ((strlen(command) + 1 + strlen(input_file_path) + 1 + 
                          strlen(output_dir) + 1 + strlen(digits_in_img_paths) + 1) * sizeof(char));

  char *command_buff = (char *)malloc(command_buff_len);
  if (command_buff == NULL) {
    fprintf(stderr, "Error allocating buffer for ffmpeg command\n");
    free(input_file_path);
    return -1;
  }

  int ok = snprintf(command_buff, command_buff_len, command,
                    input_file_path, output_dir, digits_in_img_paths);
  if (ok < 0) {
    fprintf(stderr, "Error creating command for ffmpeg\n"); 
    free(input_file_path);
    free(command_buff);
    return -1;
  }

  int command_result = system(command_buff);
  if (command_result != 0) {
    fprintf(stderr, "Error occured running command %s: %s\n", command_buff, strerror(errno));
    free(input_file_path);
    free(command_buff);
    return -1;
  }

  // if it is successfuly you can run the app but you choose the dir that it output too instead of the user choosing
  free(input_file_path);
  free(command_buff);
  
  // we make the images into the dir we created

   char *img_dir = output_dir;

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

     unsigned char *img1 = load_png_v2(img1_path);
     unsigned char *img2 = load_png_v2(img2_path);

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

   // TODO: fix odd looking imag paths
   const int path_buffer_size = 24;
   char old_name_buffer[path_buffer_size];
   char new_name_buffer[path_buffer_size];
   int img_count = 1;
   for (int i = 0; i < sorted_entries; i++) {
     memset(old_name_buffer, path_buffer_size, sizeof(char));
     memset(new_name_buffer, path_buffer_size, sizeof(char));

     int ret = -1;
     ret = snprintf(old_name_buffer, path_buffer_size, "%s/%s", img_dir, namelist[i]->d_name);
     if (ret < 0) {
       fprintf(stderr, "Error creating buffer for old path: %s/%s\n", img_dir, namelist[i]->d_name);
       free_namelist(namelist, sorted_entries);
       closedir(dir);
       return -1;
     }

     if ((stat(old_name_buffer, &st)) != 0) {
       continue;
     }

     ret = snprintf(new_name_buffer, path_buffer_size, "%s/%09d.png", img_dir, img_count);
     if (ret < 0) {
       fprintf(stderr, "Error creating buffer for new path: %s/%09d.png\n", img_dir, img_count);
       free_namelist(namelist, sorted_entries);
       closedir(dir);
       return -1;
     }

     if ((rename(old_name_buffer, new_name_buffer)) == -1) {
       fprintf(stderr, "Error renaming %s to %s: %s\n", old_name_buffer, new_name_buffer, strerror(errno));
       free_namelist(namelist, sorted_entries);
       closedir(dir);
       return -1;
     }

     printf("renamed: old_name = (%s), new_name = (%s)\n", old_name_buffer, new_name_buffer);
     img_count++;
   }

   free_namelist(namelist, sorted_entries);
   closedir(dir);

   // TODO reorder files with correct numbers
   // run ffmpeg to reassemble everything

   char *final_command = "rm -rf ./final && mkdir -p ./final && "
     "ffmpeg -framerate 60 -start_number 1 -i ./output/%s.png -c:v libx264 -pix_fmt yuv420p ./final/output.mkv";

   int final_command_buff_len = (256 * sizeof(char));

   char *final_command_buff = (char *)malloc(final_command_buff_len);
   if (final_command_buff == NULL) {
     fprintf(stderr, "Error allocating buffer for ffmpeg final command\n");
     return -1;
   }

   int okay = snprintf(final_command_buff, final_command_buff_len,
                      final_command, digits_in_img_paths);
   if (okay < 0) {
     fprintf(stderr, "Error creating final command for ffmpeg\n"); 
     free(final_command_buff);
     return -1;
   }

   int final_command_result = system(final_command_buff);
   if (final_command_result != 0) {
     fprintf(stderr, "Error occured running final command %s: %s\n", final_command_buff, strerror(errno));
     free(final_command_buff);
     return -1;
   }

   free(final_command_buff);
   return 0;
}
