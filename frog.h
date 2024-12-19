#ifndef FROG_H
#define FROG_H

#include <ncurses.h>
#include <pthread.h>
#include <stdbool.h>

// Dimensioni della rana
#define FROG_ROWS 2
#define FROG_COLS 3

// Struttura per rappresentare la rana
typedef struct {
    int x, y;                       // Posizione della rana
    char shape[FROG_ROWS][FROG_COLS]; // Forma della rana
} Frog;

// Variabili globali
extern bool frog_running;           // Stato del thread della rana
extern pthread_mutex_t frog_mutex; //mutex per lo spostamento rana

// Funzioni
void init_frog(Frog *frog);
void draw_frog(const Frog *frog,WINDOW* win);
void *frog_thread(void* arg);       // Funzione del thread della rana

#endif // FROG_H
