#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

static void die(const char *s) {
  fprintf(stderr, "%s\n", s);
  exit(1);
}

int N = 801;
int *grid;

#define IDX(i, j) ((i) * n + (j))


// you can also use this function to debug print your sandpile
static void render(char *filename) {
  int chan = 3;
  unsigned char *pixels = (unsigned char *)malloc(N * N * chan * sizeof(char));
  const int n = N;
  int stride = n * chan;
  if (!pixels)
    die("malloc failed for PNG pixel buffer");
  for (int i = 0; i < n; ++i) {
    int* row = grid + (size_t)i * n;
    for (int j = 0; j < n; ++j) {
      unsigned char r = 0, g = 0, b = 0;
      switch (row[j]) {
      case 0: // obsidian
        r = 15;
        g = 15;
        b = 25;
        break;
      case 1: // electric cyan
        r = 0;
        g = 210;
        b = 255;
        break;
      case 2: // vibrant magenta
        r = 230;
        g = 30;
        b = 130;
        break;
      case 3: // bright gold
        r = 255;
        g = 200;
        b = 0;
        break;
      default: // white
        r = 0;
        g = 0;
        b = 0;
      }

      int pid = (i * N + j) * chan;
      pixels[pid] = r;
      pixels[pid + 1] = g;
      pixels[pid + 2] = b;
    }
  }
  if (stbi_write_png(filename, N, N, chan, pixels, stride)) {
    printf("Successfully saved sandpile identity to: %s\n", filename);
  } else {
    die("Error: stb_image_write failed to save png file\n");
  }
  free(pixels);
}

static void stabilize() {
  int changed = 0;
  const int n = N;
  int halfN = n / 2;
  int halfNsq = halfN * halfN;
  do {
    changed = 0;
    for (int i = 0; i < n; ++i) {
      int i_temp = i - halfN;
      int* row = grid + (size_t)i * n;
      for (int j = 0; j <= MAX(i, n - 1 - i); ++j) {
        int j_temp = j - halfN;
        if (i_temp * i_temp + j_temp * j_temp <=
            halfNsq) {
          if (row[j] < 4)
            continue;
          changed = 1; // topple it
          row[j] -= 4;
          if (i > 0)
            row[j - n]++;
          if (j > 0)
            row[j - 1]++;
          if (i < n - 1)
            row[j + n]++;
          if (j < n - 1)
            row[j + 1]++;
        }
      }
    }
  } while (changed);
}

void find_identity() {
  struct timeval start, end;
  const int n = N;
  gettimeofday(&start, NULL);
  for (int i = 0; i < n; ++i) {
    int* row = grid + (size_t)i * n;
    for (int j = 0; j < n; ++j) {
      row[j] = 6;
    }
  }
  stabilize();
  for (int i = 0; i < n; ++i) {
    int* row = grid + (size_t)i * n;
    for (int j = 0; j < n; ++j) {
      row[j] = 6 - row[j];
    }
  }
  stabilize();
  gettimeofday(&end, NULL);
  long seconds = end.tv_sec - start.tv_sec;
  long microseconds = end.tv_usec - start.tv_usec;

  double elapsed_sec = seconds + microseconds * 1e-6;
  double elapsed_ms = (seconds * 1000.0) + (microseconds / 1000.0);

  printf("Elapsed time: %.6f seconds (%.3f ms)\n", elapsed_sec, elapsed_ms);
}

int main(int argc, char **argv) {
  if (argc < 2 || argc > 2) {
    fprintf(stderr, "Usage: %s N\n", argv[0]);
    return EXIT_FAILURE;
  }
  char *end;
  uintmax_t parsed = strtoumax(argv[1], &end, 10);
  if (errno || argv[1][0] < '0' || argv[1][0] > '9' || *end || !parsed ||
      parsed > INT32_MAX)
    die("grid size must be a positive int");
  N = (int)parsed;

  grid = (int *)malloc(N * N * sizeof(int));

  find_identity();

  render("identity.png");

  free(grid);
  return EXIT_SUCCESS;
}
