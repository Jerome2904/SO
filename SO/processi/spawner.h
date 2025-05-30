#ifndef SPAWNER_H
#define SPAWNER_H


#include "game.h"
#include "crocodile.h"


void create_spawners(int fd_write, int fd_read, RiverLane lanes[], pid_t spawner_pids[], int n_lanes);
void spawner_process(int fd_write, int fd_read, RiverLane lane);
void interruptible_sleep(int total_microseconds);


#endif