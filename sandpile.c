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
int *next;
int *mask;


// you can also use this function to debug print your sandpile
static void render(char *filename) {
  int chan = 3;
  unsigned char *pixels = (unsigned char *)malloc(N * N * chan * sizeof(char));
  const int n = N;
  int stride = n * chan;
  if (!pixels)
    die("malloc failed for PNG pixel buffer");
  for (int i = 0; i < n; ++i) {
    int* row = grid + (size_t)(i + 1) * (n + 2);
    for (int j = 0; j < n; ++j) {
      unsigned char r = 0, g = 0, b = 0;
      switch (row[j + 1]) {
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

static int sweep(const int *restrict a, int *restrict b, const int *restrict m) {
  const int n = N;
  int changed = 0;
  for (int i = 0; i < n; ++i) {
    const int base = (i + 1) * (n + 2) + 1;
    for (int j = 0; j < n; ++j) {
      const int x = base + j;
      const int t = a[x] & ~3 & m[x];
      changed |= t;
      b[x] = a[x] - t
           + ((a[x - 1] >> 2) & m[x - 1]) + ((a[x + 1] >> 2) & m[x + 1])
           + ((a[x - (n + 2)] >> 2) & m[x - (n + 2)]) + ((a[x + (n + 2)] >> 2) & m[x + (n + 2)]);
    }
  }
  return changed;
}

static void stabilize() {
  int changed = 0;
  do {
    changed = sweep(grid, next, mask);
    int *tmp = grid;
    grid = next;
    next = tmp;
  } while (changed);
}

void find_identity() {
  struct timeval start, end;
  const int n = N;
  gettimeofday(&start, NULL);
  for (int i = 0; i < n; ++i) {
    int* row = grid + (size_t)(i + 1) * (n + 2);
    for (int j = 0; j < n; ++j) {
      row[j + 1] = 6;
    }
  }
  stabilize();
  for (int i = 0; i < n; ++i) {
    int* row = grid + (size_t)(i + 1) * (n + 2);
    for (int j = 0; j < n; ++j) {
      row[j + 1] = 6 - row[j + 1];
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

  size_t num_cells = (size_t)(N + 2) * (N + 2);
  grid = (int *)calloc(num_cells, sizeof(int));
  next = (int *)calloc(num_cells, sizeof(int));
  mask = (int *)calloc(num_cells, sizeof(int));
  int halfN = N / 2;
  for (int i = 0; i < N; ++i) {
    int *row = mask + (size_t)(i + 1) * (N + 2);
    for (int j = 0; j <= MAX(i, N - 1 - i); ++j) {
      if ((i - halfN) * (i - halfN) + (j - halfN) * (j - halfN) <= halfN * halfN) {
        row[j + 1] = -1;
      }
    }
  }

  find_identity();

  render("identity.png");

  free(grid);
  free(next);
  free(mask);
  return EXIT_SUCCESS;
}
