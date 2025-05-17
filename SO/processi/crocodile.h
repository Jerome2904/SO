#ifndef CROCODILE_H
#define CROCODILE_H

#include "game.h"

#define MAX_CROCS_PER_LANE  3

#define MAX_PROJECTILES 10

//struttura per memorizzare lo stato di un coccodrillo in una corsia del fiume
typedef struct {
    Entity crocs[MAX_CROCS_PER_LANE];
    Entity prev[MAX_CROCS_PER_LANE];
    pid_t pid[MAX_CROCS_PER_LANE];   
    bool active[MAX_CROCS_PER_LANE];
} CrocLaneState;

void init_lanes(RiverLane lanes[]);
void crocodile_process(int fd_write, RiverLane lane);
void crocodile_init(Entity *crocodile, RiverLane *lane);

void projectile_process(int fd_write,int start_x, int start_y, int dx);
void draw_projectile(Entity *projectile);
void clear_projectile(Entity *projectile);


#endif
