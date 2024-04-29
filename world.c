#include "world.h"
#include <stdint.h>
#include <stdlib.h>

static const Dir LEFT         = { -1,  0 };
static const Dir RIGHT        = {  1,  0 };
static const Dir DOWN         = {  0,  1 };
static const Dir UP           = {  0, -1 };
static const Dir TOP_LEFT     = { -1, -1 };
static const Dir TOP_RIGHT    = {  1, -1 };
static const Dir BOTTOM_LEFT  = { -1,  1 };
static const Dir BOTTOM_RIGHT = {  1,  1 };

static cell_state_t check_neighbor_state(World *w, Cell *c, Dir d) {
  if ((c->j + d.j < 0) || (c->i + d.i < 0)) return DEAD;
  if ((c->j + d.j >= w->rows) || (c->i + d.i >= w->cols)) return DEAD;
  return w->cells[c->j + d.j][c->i + d.i].state;
}

static uint32_t count_num_alive_neighbors(World *w, Cell *c) {
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

World world_init(int rows, int cols, int scale, int sqsz) {
  Cell **cs = (Cell **) malloc(sizeof(Cell *) * (size_t)rows * (size_t)scale);

  for (int j = 0; j < scale * rows; j++)
    cs[j] = (Cell *) malloc(sizeof(Cell) * (size_t)scale * (size_t)cols);

  for (int j = 0; j < scale * rows; j++) {
    for (int i = 0; i < scale * cols; i++) {
      cs[j][i] = (Cell) { .i = i, .j = j,
                          .x = sqsz * i, .y = sqsz * j,
                          .state = DEAD };
    }
  }
  return (World) { .cells = cs, .rows = scale * rows,
                   .cols = scale * cols, .sqsz = sqsz };
}

void world_destroy(World *w) {
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
      if ((c->state == ALIVE) && (alive < 2 || alive > 3)) c->state = DEAD;
      else if (alive == 3)                                 c->state = ALIVE;
    }
  }
}

#undef EFFECTIVE_NEIGHBORS
