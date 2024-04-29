#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define TARGET_FPS 60

static const int SQUARE_SIZE = 16;

typedef enum {
  DEAD = 0, ALIVE = 1,
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

typedef struct { int i; int j; } Dir;

static const Dir LEFT         = { -1,  0 };
static const Dir RIGHT        = {  1,  0 };
static const Dir DOWN         = {  0,  1 };
static const Dir UP           = {  0, -1 };
static const Dir TOP_LEFT     = { -1, -1 };
static const Dir TOP_RIGHT    = {  1, -1 };
static const Dir BOTTOM_LEFT  = { -1,  1 };
static const Dir BOTTOM_RIGHT = {  1,  1 };

cell_state_t check_neighbor_state(World *w, Cell *c, Dir d) {
  if ((c->j + d.j < 0) || (c->i + d.i < 0)) return DEAD;
  if ((c->j + d.j >= w->rows) || (c->i + d.i >= w->cols)) return DEAD;
  return w->cells[c->j + d.j][c->i + d.i].state;
}

uint32_t count_num_alive_neighbors(World *w, Cell *c) {
  uint32_t num_alive = 0;
  num_alive += check_neighbor_state(w, c, LEFT);
  num_alive += check_neighbor_state(w, c, RIGHT);
  num_alive += check_neighbor_state(w, c, DOWN);
  num_alive += check_neighbor_state(w, c, UP);
  num_alive += check_neighbor_state(w, c, TOP_LEFT);
  num_alive += check_neighbor_state(w, c, TOP_RIGHT);
  num_alive += check_neighbor_state(w, c, BOTTOM_LEFT);
  num_alive += check_neighbor_state(w, c, BOTTOM_RIGHT);
  return num_alive;
}

World world_init(int rows, int cols, int scale) {
  Cell **cs = (Cell **) malloc(sizeof(Cell *) *
                               (size_t)rows * (size_t)scale);
  for (int j = 0; j < scale * rows; j++) cs[j] = (Cell *) malloc(sizeof(Cell)  *
                                                                 (size_t)scale *
                                                                 (size_t)cols);
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

#define EFFECTIVE_NEIGHBORS(N, W)                                \
  do {                                                           \
    for (int j = 0; j < W->rows; j++) {                          \
      for (int i = 0; i < W->cols; i++) {                        \
        N[j][i] = count_num_alive_neighbors(W, &W->cells[j][i]); \
      }                                                          \
    }                                                            \
  } while(0)                                                     \

void world_next(World *w) {
  uint32_t alive_at[w->rows][w->cols];
  EFFECTIVE_NEIGHBORS(alive_at, w);
  for (int j = 0; j < w->rows; j++) {
    for (int i = 0; i < w->cols; i++) {
      Cell *c = &w->cells[j][i];
      uint32_t alive = alive_at[j][i];
      if (c->state == ALIVE && alive < 2 || alive > 3) c->state = DEAD;
      else if (alive == 3)                             c->state = ALIVE;
    }
  }
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

#define TO_WORLD(X,SZ) ((((float)X) / ((float)SZ)))
void mouse_handle_click(World *w, Camera2D *cam) {
  // TODO: prevent clicking outside world
  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    Vector2 m = GetScreenToWorld2D(GetMousePosition(), *cam);
    Cell *c =
      &w->cells[(size_t)TO_WORLD(m.y, w->sqsz)][(size_t)TO_WORLD(m.x, w->sqsz)];
    c->state = ALIVE;
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

void key_probe_int(bool *RUNNING) {
  int k = GetKeyPressed();
  switch (k) {
  case KEY_ESCAPE: exit(EXIT_SUCCESS);
  case KEY_SPACE: *RUNNING = !(*RUNNING); break;
  default: return;
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

#define SNAP_TO_CELL(X,SZ) ((int)(X) - (int)(X) % SZ)
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

  const int WORLD_SCALE   = 10;
  const int SCREEN_ROWS   = 42;
  const int SCREEN_COLS   = 68;
  const int SCREEN_WIDTH  = SQUARE_SIZE * SCREEN_COLS;
  const int SCREEN_HEIGHT = SQUARE_SIZE * SCREEN_ROWS;

  /* SetTraceLogLevel(LOG_NONE); // disable default logging if desired */
  SetConfigFlags(FLAG_VSYNC_HINT); // help with screen tearing
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Conway's Game of Life");
  SetExitKey(KEY_NULL);

  Camera2D camera = { 0 };
  camera.zoom = 1.0f;
  
  SetTargetFPS(TARGET_FPS);

  RenderTexture2D cb = load_board_texture(WORLD_SCALE * SCREEN_COLS,
                                          WORLD_SCALE * SCREEN_ROWS);

  World world = world_init(SCREEN_ROWS, SCREEN_COLS, WORLD_SCALE);

  hl.bg = &cb;
  hl.w  = &world;

  bool RUNNING = false;
  while (!WindowShouldClose()) {
    key_probe_int(&RUNNING);
    if (RUNNING) world_next(&world);
    key_probe_char(&world, &camera);
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
