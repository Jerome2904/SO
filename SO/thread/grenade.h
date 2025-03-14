#ifndef GRENADE_H
#define GRENADE_H

#include "game.h"
#include "buffer.h"

typedef struct {
    CircularBuffer *buffer;
    int start_x;
    int start_y;
    int dx;
    int speed;
} GrenadeArgs;

void *grenade_thread(void* arg);
void draw_grenade(WINDOW *win, Entity *grenade);
void clear_grenade(WINDOW *win, Entity *grenade);

#endif
