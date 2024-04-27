#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <stdio.h>

#define TARGET_FPS 60

static const int SQUARE_SIZE = 16;

typedef enum {
  DEAD, ALIVE,
} cell_state_t ;

typedef struct {
  int i; int j; // world space
  int x; int y; // screen space
  cell_state_t state;
} Cell;

typedef struct {
  Cell **cells;
  int rows;
  int cols;
  int sqsz;
} World;

World world_init(int rows, int cols, int scale) {
  Cell **cs = (Cell **) malloc(sizeof(Cell *) * rows * scale);
  for (int j = 0; j < scale * rows; j++) {
    cs[j] = (Cell *) malloc(sizeof(Cell) * scale * cols);
  }
  for (int j = 0; j < scale * rows; j++) {
    for (int i = 0; i < scale * cols; i++) {
      cs[j][i] = (Cell) {
        .i = i, .j = j,
        .x = SQUARE_SIZE * i,
        .y = SQUARE_SIZE * j,
        .state = DEAD,
      };
    }
  }
  return (World) {
    .cells = cs,
    .rows = scale * rows,
    .cols = scale * cols,
    .sqsz = SQUARE_SIZE,
  };
}

void world_destroy(World *w) {
  printf("DESTROYING ZE WORLD\n");
  for (int j = 0; j < w->rows; j++) free(w->cells[j]);
  free(w->cells);
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

#define TO_WORLD(X,SZ) ((int)((X) / (SZ)))
void mouse_handle_click(World *w, Camera2D *cam) {
  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    Vector2 m = GetScreenToWorld2D(GetMousePosition(), *cam);
    w->cells[TO_WORLD(m.y, w->sqsz)][TO_WORLD(m.x, w->sqsz)].state = ALIVE;
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

void key_probe_int(void) {
  int k = GetKeyPressed();
  switch (k) {
  case KEY_ESCAPE: exit(EXIT_SUCCESS);
  default: return;
  }
}

void key_probe_char(Camera2D *cam) {
  char c = GetCharPressed();
  switch (c) {
  case 'r': camera_reset(cam); return;
  case 'q': exit(EXIT_SUCCESS);
  default: return;
  }
}

/* Lack of floating point arithmetic means some weird gap appears in hl. */
#define SNAP_TO_CELL(X,SZ) ((int)(X) - (int)(X) % SZ)
void mouse_handle_highlight(Camera2D *cam) {
  Vector2 m = GetScreenToWorld2D(GetMousePosition(), *cam);
  DrawRectangleLinesEx((Rectangle) { SNAP_TO_CELL(m.x, SQUARE_SIZE),
                                     SNAP_TO_CELL(m.y, SQUARE_SIZE),
                                     SQUARE_SIZE,
                                     SQUARE_SIZE, }, 2.5f, GREEN);
}

struct hitlist { RenderTexture2D *bg; World *w; };

void handle_exit(int code, void *args) {
  printf("Exiting and shizz.\n");
  struct hitlist *hl = (struct hitlist *) args;
  world_destroy(hl->w);
  UnloadRenderTexture(*(hl->bg));
  CloseWindow();
}

int main(void) {
  struct hitlist hl = {0};
  on_exit(handle_exit, &hl);

  const int WORLD_SCALE      = 10;
  const int SCREEN_ROWS      = 36;
  const int SCREEN_COLS      = 54;
  const int SCREEN_WIDTH     = SQUARE_SIZE * SCREEN_COLS;
  const int SCREEN_HEIGHT    = SQUARE_SIZE * SCREEN_ROWS;
  const int SCREEN_BG_OFFSET = (-1) * WORLD_SCALE * SQUARE_SIZE / 2;

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Conway's Game of Life");
  SetExitKey(KEY_NULL);

  Camera2D camera = { 0 };
  camera.zoom = 1.0f;
  
  SetTargetFPS(TARGET_FPS);

  RenderTexture2D cb = load_board_texture(WORLD_SCALE * SCREEN_COLS,
                                          WORLD_SCALE * SCREEN_ROWS);
  World world = world_init(SCREEN_ROWS, SCREEN_COLS, WORLD_SCALE);

  hl.bg = &cb;
  hl.w = &world;

  while (!WindowShouldClose()) {
    key_probe_int();
    key_probe_char(&camera);
    BeginDrawing();
      ClearBackground(BLACK);
      BeginMode2D(camera);
        camera_handle_zoom(&camera);
        DrawTexture(cb.texture,
                    0, 0,
                    WHITE);
        mouse_handle_highlight(&camera);
        mouse_handle_click(&world, &camera);
        world_render(&world);
      EndMode2D();
    EndDrawing();
  }

  return 0;
}
