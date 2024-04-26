#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include "color.h"

#define TARGET_FPS 30

static int SQUARE_SIZE = 20;
static int NUM_ROWS = 36;
static int NUM_COLS = 54;

void draw_board(void) {
  for (int y = 0; y < NUM_ROWS; y++) {
    for (int x = 0; x < NUM_COLS; x++) {
      DrawRectangle(x * SQUARE_SIZE, y * SQUARE_SIZE,
                    SQUARE_SIZE, SQUARE_SIZE,
                    (x + y) % 2 ? DARKGRAY : LIGHTGRAY);
    }
  }
}

int main(void) {
  InitWindow(SQUARE_SIZE * NUM_COLS,
             SQUARE_SIZE * NUM_ROWS,
             "Conway's Game of Life");

  SetTargetFPS(TARGET_FPS);

  while (!WindowShouldClose()) {
    BeginDrawing();
      ClearBackground(DARKGRAY);
      draw_board();
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
