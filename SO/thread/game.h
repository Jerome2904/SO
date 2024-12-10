#ifndef GAME_H
#define GAME_H

#include <ncurses.h>
#include <pthread.h>
#include <stdbool.h>


// Funzioni
void start_game(int height,int width,int starty,int startx);             // Inizializza il gioco
void *timer_thread(); // Funzione del thread per il timer

#endif // GAME_H
