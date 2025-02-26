#ifndef FROG_H
#define FROG_H

#include "game.h"

typedef struct {
    CircularBuffer *buffer;
    WINDOW *win;
} FrogArgs;

void frog_init(Entity *frog);
void draw_frog(WINDOW *game_win, Entity *frog);
void clear_frog(WINDOW *game_win, Entity *frog);
void *frog_thread(void *arg);

#endif