#ifndef GAME_H
#define GAME_H

#include <ncurses.h>
#include <pthread.h>
#include <stdbool.h>

#define MAP_HEIGHT 40
#define MAP_WIDTH 50


// Funzioni
extern bool gameover;
extern pthread_mutex_t gameover_mutex; //mutex per chiudere il gioco
extern pthread_mutex_t render_mutex; //mutex per il render delle finestre

void start_game();             // Inizializza il gioco
void setGameover();

#endif // GAME_H
