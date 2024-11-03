#include "libz.h"
#include <time.h>

#define TIMES 1000

int main(void) {
  double old_total = 0.0;
  double new_total = 0.0;

  time_t s_old, e_old, s_new, e_new;

  printf("Starting testing load_png vs load_png_v2\n");
  for (int i = 0; i < TIMES; i++) {
    time(&s_old);
    unsigned char *old_data = load_png("./output/000000001.png");
    time(&e_old);
    double old_diff = e_old - s_old;
    old_total += old_diff;

    time(&s_new);
    unsigned char *new_data = load_png_v2("./output/000000001.png");
    time(&e_new);
    double new_diff = e_new - s_new;
    new_total += new_diff;

    free(old_data);
    free(new_data);
  }
  printf("Completed testing load_png vs load_png_v2\n");

  printf("Results: load_png average = (%.8f) seconds,  load_png_v2 average = (%.8f) seconds\n", (old_total / TIMES), (new_total / TIMES));
}
