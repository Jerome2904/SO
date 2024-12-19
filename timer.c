#include "timer.h"
#include "game.h"
#include "frog.h"
#include <unistd.h> // Per sleep()

// Variabili globali
int time_left;
bool timer_running;
pthread_mutex_t timer_mutex = PTHREAD_MUTEX_INITIALIZER;

void start_timer(WINDOW* win) {
    pthread_t tid;

    // Inizializza le variabili globali
    pthread_mutex_lock(&timer_mutex);
    time_left = TIMER_DURATION;
    timer_running = true;
    pthread_mutex_unlock(&timer_mutex);

    // Crea il thread del timer
    pthread_create(&tid, NULL, timer_thread, win);
    pthread_detach(tid); // Rendi il thread "detached" per evitarne la join esplicita
}

void stop_timer() {
    pthread_mutex_lock(&timer_mutex);
    timer_running = false;
    frog_running = false;
    pthread_mutex_unlock(&timer_mutex);
}

void *timer_thread(void* arg) {
    WINDOW* win = (WINDOW*) arg;
    while (1) {
        pthread_mutex_lock(&timer_mutex);
        if (!timer_running || time_left <= 0) {
            // Cancella il contenuto della finestra del timer
            werase(win);
            pthread_mutex_lock(&render_mutex);
            wrefresh(win);
            pthread_mutex_unlock(&render_mutex);

            timer_running = false; // Interrompe il timer
            pthread_mutex_unlock(&timer_mutex);
            break;
        }

        // Aggiorna il timer nella finestra
        werase(win);
        box(win,0,0);
        mvwprintw(win, 1, 1, "Tempo rimanente: %d", time_left);
        
        pthread_mutex_lock(&render_mutex);
        wrefresh(win);
        pthread_mutex_unlock(&render_mutex);

        time_left--;
        pthread_mutex_unlock(&timer_mutex);

        sleep(1); // Aspetta un secondo
    }
    pthread_exit(NULL);
    return NULL;
}
