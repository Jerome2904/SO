#ifndef MAP_H
#define MAP_H

#include <ncurses.h>
#include "game.h"

#define NUM_HOLES 5

// Costanti per le posizioni delle tane
#define HOLE_Y 3
#define HOLE_X1 7  
#define HOLE_X2 22 
#define HOLE_X3 37
#define HOLE_X4 52 
#define HOLE_X5 67


// Struttura per rappresentare una tana
typedef struct {
    int x;
    int y;
    bool occupied;
} Hole;

extern pthread_mutex_t render_mutex; //mutex per il render delle finestre

// Matrice della mappa
extern int map[MAP_HEIGHT][MAP_WIDTH];

extern Hole tane[NUM_HOLES];

// Funzioni per la gestione della mappa
void init_bckmap();
void draw_map(WINDOW *win);
void init_holes_positions();
void init_map_holes();
int check_hole_reached(Entity *frog);
void hole_update(WINDOW *win,int hole_index);
bool checkHoles();


#endif 
