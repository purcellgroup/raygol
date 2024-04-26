#include <raylib.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "color.h"

static const char *WINDOW_TITLE = "Conway's Game of Life";
static int SCREEN_HEIGHT = 0;
static int SCREEN_WIDTH = 0;
static int SQUARE_SIZE = 20;

void read_args(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "Must provide screen rows and columns. args: %d\n", argc);
    exit(EXIT_FAILURE);
  }

  SCREEN_HEIGHT = atoi(argv[1]);
  SCREEN_WIDTH = atoi(argv[2]);
  printf("ROWS: %d, COLS: %d\n", SCREEN_HEIGHT, SCREEN_WIDTH);
}

void draw_grid(int rows, int cols) {
  int x;
  int y;

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      x = j + SQUARE_SIZE;
      y = i + SQUARE_SIZE;
      DrawRectangle(x, y, SQUARE_SIZE, SQUARE_SIZE, i % 2 == 0 ? WHITE : BLACK);
    }
  }
}

int main(int argc, char **argv) {
  read_args(argc, argv);
  InitWindow(SCREEN_WIDTH, SCREEN_WIDTH, WINDOW_TITLE);
  SetTargetFPS(60);
  draw_grid(SCREEN_HEIGHT, SCREEN_WIDTH);

  while(!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
