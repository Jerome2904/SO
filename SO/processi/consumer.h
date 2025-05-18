#ifndef CONSUMER_H
#define CONSUMER_H

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

void consumer(int fd_read,int fd_write, WINDOW *info_win);
void frog_move(Entity *frog, Entity *frog_prev, int dx, int dy);
void frog_water_check(Entity *frog, Entity *frog_prev, CrocLaneState lanes_state[], int lane_y[], int *lives, int frog_start_x, int frog_start_y);
void frog_drift_on_croc(Entity *frog, Entity *frog_prev, Entity *croc);
void check_grenade_projectile_collisions(Entity grenades[], Entity gren_prev[], bool gren_active[], pid_t gren_pid[],Entity projectiles[], Entity proj_prev[], bool proj_active[], pid_t proj_pid[]);
void clean();
#endif