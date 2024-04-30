#include "io.h"
#include <stdlib.h>

atomic_bool RUN = false;

static void camera_reset(Camera2D *cam) {
  cam->zoom   = 1.0f;
  cam->target = (Vector2) {0.0f, 0.0f};
  cam->offset = (Vector2) {0.0f, 0.0f};
}

void camera_handle_zoom(Camera2D *cam) {
  static const float zoom_increment = 0.05f;
  float wheel = GetMouseWheelMove();
  if (wheel != 0) {
    Vector2 mouse_pos = GetScreenToWorld2D(GetMousePosition(), *cam);
    cam->offset       = GetMousePosition();
    cam->target       = mouse_pos;
    cam->zoom += (wheel * zoom_increment);
    if (cam->zoom < zoom_increment) cam->zoom = zoom_increment;
  }
}

void mouse_handle_highlight(World *w, Camera2D *cam) {
  Vector2 m = GetScreenToWorld2D(GetMousePosition(), *cam);
  DrawRectangleLinesEx((Rectangle) { (float)SNAP_TO_CELL(m.x, w->sqsz),
                                     (float)SNAP_TO_CELL(m.y, w->sqsz),
                                     (float)w->sqsz,
                                     (float)w->sqsz, }, 2.5f, GREEN);
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
