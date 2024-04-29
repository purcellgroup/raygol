#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>
#include "world.h"

#define TARGET_FPS 60

#define SQUARE_SIZE 16
#define WORLD_SCALE 10
#define SCREEN_ROWS 42
#define SCREEN_COLS 68

#define SCREEN_WIDTH  SQUARE_SIZE * SCREEN_COLS
#define SCREEN_HEIGHT SQUARE_SIZE * SCREEN_ROWS

static atomic_bool RUN = false;

#define TO_WORLD(X,SZ)     (((float)X) / ((float)SZ))
#define SNAP_TO_CELL(X,SZ) ((int)(X) - (int)(X) % SZ)

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

void mouse_handle_click(World *w, Camera2D *cam) {
  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    Vector2 m = GetScreenToWorld2D(GetMousePosition(), *cam);
    int row = (int)TO_WORLD(m.y, w->sqsz);
    int col = (int)TO_WORLD(m.x, w->sqsz);

    // Check if the click is within the world bounds
    if (row >= 0 && row < w->rows && col >= 0 && col < w->cols) {
      Cell *c = &w->cells[row][col];
      c->state = ALIVE;
    }
  }
  if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
    Vector2 m_delta = GetMouseDelta();
    cam->offset = Vector2Add(cam->offset, m_delta);
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

void camera_handle_zoom(Camera2D *cam) {
  static const float zoom_increment = 0.2f;
  float wheel = GetMouseWheelMove();
  if (wheel != 0) {
    Vector2 mouse_pos = GetScreenToWorld2D(GetMousePosition(), *cam);
    cam->offset       = GetMousePosition();
    cam->target       = mouse_pos;
    cam->zoom += (wheel * zoom_increment);
    if (cam->zoom < zoom_increment) cam->zoom = zoom_increment;
  }
}

void camera_reset(Camera2D *cam) {
  cam->zoom   = 1.0f;
  cam->target = (Vector2) {0.0f, 0.0f};
  cam->offset = (Vector2) {0.0f, 0.0f};
}

void key_probe_int() {
  int k = GetKeyPressed();
  switch (k) {
  case KEY_ESCAPE: exit(EXIT_SUCCESS);
  case KEY_SPACE: RUN = !RUN; return;
  default:                    return;
  }
}

void key_probe_char(World *w, Camera2D *cam) {
  int c = GetCharPressed();
  switch (c) {
  case 'n': world_next(w); return; // should disable when RUNNING
  case 'r': camera_reset(cam); return;
  case 'q': exit(EXIT_SUCCESS);
  default: return;
  }
}

void mouse_handle_highlight(Camera2D *cam) {
  Vector2 m = GetScreenToWorld2D(GetMousePosition(), *cam);
  DrawRectangleLinesEx((Rectangle) { (float)SNAP_TO_CELL(m.x, SQUARE_SIZE),
                                     (float)SNAP_TO_CELL(m.y, SQUARE_SIZE),
                                     (float)SQUARE_SIZE,
                                     (float)SQUARE_SIZE, }, 2.5f, GREEN);
}

struct hitlist { RenderTexture2D *bg; World *w; };

void handle_exit(int code, void *args) {
  printf("Exit code: %d\n", code);
  struct hitlist *hl = (struct hitlist *) args;
  world_destroy(hl->w);
  UnloadRenderTexture(*(hl->bg));
  CloseWindow();
}

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
        mouse_handle_highlight(&camera);
        mouse_handle_click(&world, &camera);
        world_render(&world);
      EndMode2D();
    EndDrawing();
  }

  return 0;
}
