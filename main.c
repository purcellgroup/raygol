#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <stdio.h>

#include "world.h"
#include "io.h"

#define TARGET_FPS 60

#define SQUARE_SIZE 16
#define WORLD_SCALE 10
#define SCREEN_ROWS 42
#define SCREEN_COLS 68

#define SCREEN_WIDTH  SQUARE_SIZE * SCREEN_COLS
#define SCREEN_HEIGHT SQUARE_SIZE * SCREEN_ROWS

struct hitlist { RenderTexture2D *bg; World *w; };
void handle_exit(int code, void *args);
void world_render(World *);
RenderTexture2D load_board_texture(int, int);

int main(void) {
  struct hitlist hl = {0};
  on_exit(handle_exit, &hl);

  SetConfigFlags(FLAG_VSYNC_HINT); // help with screen tearing
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Conway's Game of Life");
  SetExitKey(KEY_NULL); // we handle escape with on_exit

  Camera2D camera = { 0 };
  camera.zoom = 1.0f;
  
  SetTargetFPS(TARGET_FPS);

  // WORLD_SCALE dictates size of World OUTSIDE of screen space.
  RenderTexture2D cb = load_board_texture(WORLD_SCALE * SCREEN_COLS,
                                          WORLD_SCALE * SCREEN_ROWS);

  World world = world_init(SCREEN_ROWS, SCREEN_COLS,
                           WORLD_SCALE, SQUARE_SIZE);

  hl.bg = &cb;
  hl.w  = &world;

  while (!WindowShouldClose()) {
    key_probe_int();
    key_probe_char(&world, &camera);
    if (RUN) world_next(&world);
    BeginDrawing();
      ClearBackground(BLACK);
      BeginMode2D(camera);
        camera_handle_zoom(&camera);
        DrawTexture(cb.texture, 0, 0, WHITE);
        mouse_handle_highlight(&world, &camera);
        mouse_handle_click(&world, &camera);
        world_render(&world);
      EndMode2D();
    EndDrawing();
  }

  return 0;
}

void handle_exit(int code, void *args) {
  printf("Exit code: %d\n", code);
  struct hitlist *hl = (struct hitlist *) args;
  world_destroy(hl->w);
  UnloadRenderTexture(*(hl->bg));
  CloseWindow();
}

void world_render(World *w) {
  for (int row = 0; row < w->rows; row++) {
    for (int col = 0; col < w->cols; col++) {
      if (w->cells[row][col].state == ALIVE) {
        Cell c = w->cells[row][col];
        DrawRectangle(c.x, c.y, w->sqsz, w->sqsz, GREEN);
      }
    }
  }
}

RenderTexture2D load_board_texture(int rows, int cols) {
  RenderTexture2D cb = LoadRenderTexture(rows * SQUARE_SIZE,
                                         cols * SQUARE_SIZE);
  BeginTextureMode(cb);

  for (int y = 0; y < cols; y++) {
    for (int x = 0; x < rows; x++) {
      DrawRectangle(x * SQUARE_SIZE, y * SQUARE_SIZE,
                    SQUARE_SIZE, SQUARE_SIZE,
                    (x + y) % 2 ? DARKGRAY : BLACK);
    }
  }

  EndTextureMode();
  return cb;
}
