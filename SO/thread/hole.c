// hole.c
#include "hole.h"
#include "paramThreads.h"
#include "map.h"

Hole tane[NUM_HOLES];

//Viene riempito l'array di tane
void init_holes() {
    int hole_positions[NUM_HOLES] = {HOLE_X1, HOLE_X2, HOLE_X3, HOLE_X4, HOLE_X5};
    for (int i = 0; i < NUM_HOLES; i++) {
        tane[i].x = hole_positions[i];
        tane[i].y = HOLE_Y;
        tane[i].occupied = false;
    }
}

// Inserisce le tane nella matrice di interi rappresentante la mappa
void init_map_holes() {
    for (int i = 0; i < NUM_HOLES; i++) {
        for (int dy = 0; dy < FROG_ROWS; dy++) {
            for (int dx = 0; dx < FROG_COLS; dx++) {
                map[tane[i].y + dy][tane[i].x + dx] = 4;
            }
        }
    }
}
void fillHole(void* arg){
    FrogThreadParams* prms = (FrogThreadParams*) arg;
    Frog* frog = prms->frog;
    WINDOW* win = prms->win;

    //trovo la tana e la segno occupata
    int i;
    switch (frog->x){
        case HOLE_X1:
                i=0;
            break;
        case HOLE_X2:
                i=1;
            break;
        case HOLE_X3:
                i=2;
            break;
        case HOLE_X4:
                i=3;
            break;
        case HOLE_X5:
                i=4;
            break;
    }
    tane[i].occupied=true;

    //cambio valore della tana nella mappa
    for (int dy = 0; dy < FROG_ROWS; dy++) {
        for (int dx = 0; dx < FROG_COLS; dx++) {
            map[tane[i].y + dy][tane[i].x + dx] = 5;
        }
    }
    draw_map(win);
}

bool checkHoles(){
    for (int i = 0; i < NUM_HOLES; i++)
    {
        if (!tane[i].occupied) return false;
    }
    return true;
}
