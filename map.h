#ifndef MAP_H
#define MAP_H

#include <ncurses.h>
#include "game.h"


// Matrice della mappa
extern int map[MAP_HEIGHT][MAP_WIDTH];

// Funzioni per la gestione della mappa
void init_bckmap();
void draw_map(WINDOW *win);

#endif // MAP_H
