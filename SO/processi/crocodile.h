#ifndef CROCODILE_H
#define CROCODILE_H

#include "game.h"


void init_lanes(RiverLane lanes[]);
void crocodile_process(int fd_write, RiverLane lane);
void crocodile_init(Entity *crocodile, RiverLane *lane);

void projectile_process(int fd_write,int start_x, int start_y, int dx);
void draw_projectile(Entity *projectile,WINDOW* game_win);
void clear_projectile(Entity *projectile,WINDOW* game_win);


#endif
