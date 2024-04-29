#ifndef IO_H_
#define IO_H_
#include <raylib.h>
#include <raymath.h>
#include <stdatomic.h>
#include "world.h"

#define TO_WORLD(X,SZ)     (((float)X) / ((float)SZ))
#define SNAP_TO_CELL(X,SZ) ((int)(X) - (int)(X) % SZ)

extern atomic_bool RUN;

void camera_handle_zoom(Camera2D *);
void mouse_handle_highlight(World *, Camera2D *);
void mouse_handle_click(World *, Camera2D *);
void key_probe_int(void);
void key_probe_char(World *, Camera2D *);

#endif // IO_H_
