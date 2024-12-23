// hole.h
#ifndef HOLE_H
#define HOLE_H

#include <stdbool.h>
#include "game.h"
#include "frog.h"

#define NUM_HOLES 5

// Costanti per le posizioni delle tane
#define HOLE_Y 4
#define HOLE_X1 6
#define HOLE_X2 15
#define HOLE_X3 24
#define HOLE_X4 33
#define HOLE_X5 42


// Struttura per rappresentare una tana
typedef struct {
    int x;
    int y;
    bool occupied;
} Hole;

extern Hole tane[NUM_HOLES];

// Funzioni
void init_holes();
void init_map_holes();
void fillHole(void* arg);
bool checkHoles();

#endif // HOLE_H
