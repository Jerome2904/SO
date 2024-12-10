#include "timer.h"
#include <ncurses.h>
#include <unistd.h> // Per sleep()

// Variabili globali
int time_left;
bool timer_running;
pthread_mutex_t timer_mutex = PTHREAD_MUTEX_INITIALIZER;

void start_timer(int duration) {
    pthread_t tid;

    // Inizializza le variabili globali
    pthread_mutex_lock(&timer_mutex);
    time_left = duration;
    timer_running = true;
    pthread_mutex_unlock(&timer_mutex);

    // Crea il thread del timer
    pthread_create(&tid, NULL, timer_thread, NULL);
    pthread_detach(tid); // Rendi il thread "detached" per evitarne la join esplicita
}

void stop_timer() {
    pthread_mutex_lock(&timer_mutex);
    clrtoeol();
    timer_running = false;
    pthread_mutex_unlock(&timer_mutex);
}

void *timer_thread() {
    while (1) {
        pthread_mutex_lock(&timer_mutex);
        if (!timer_running || time_left <= 0) {
            pthread_mutex_unlock(&timer_mutex);
            break;
        }

        clrtoeol();
        mvprintw(1,(COLS-20)/2,"Tempo rimanente: %d",time_left);
        refresh();

        time_left--;
        pthread_mutex_unlock(&timer_mutex);

        sleep(1); // Aspetta un secondo
    }

    pthread_mutex_lock(&timer_mutex);
    timer_running = false;
    pthread_mutex_unlock(&timer_mutex);
}
