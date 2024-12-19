#ifndef TIMER_H
#define TIMER_H

#include <ncurses.h>
#include <pthread.h>
#include <stdbool.h>

//costanti
#define TIMER_DURATION 10 // Durata del timer in secondi

// Variabili globali
extern int time_left;          // Tempo rimanente per il timer
extern bool timer_running;     // Stato del timer
extern pthread_mutex_t timer_mutex; // Mutex per sincronizzazione

// Funzioni
void start_timer(WINDOW* win);       // Avvia il timer con una durata specificata
void stop_timer();                    // Ferma il timer
void *timer_thread(void* arg);        // Funzione del thread del timer

#endif // TIMER_H
