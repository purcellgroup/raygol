#ifndef WORLD_H_
#define WORLD_H_

typedef enum { DEAD, ALIVE } cell_state_t ;

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

World world_init(int, int, int, int);
void world_destroy(World *);
void world_next(World *);

#endif // WORLD_H_
