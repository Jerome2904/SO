#include "timer.h"
#include "game.h"
#include "frog.h"
#include <unistd.h> // Per sleep()

// Variabili globali
int time_left;
bool timer_running;
pthread_mutex_t timer_mutex = PTHREAD_MUTEX_INITIALIZER;

void stop_timer() {
    pthread_mutex_lock(&timer_mutex);
    timer_running = false;
    pthread_mutex_unlock(&timer_mutex);
}

void resetTimer() {
    pthread_mutex_lock(&timer_mutex);
    time_left = TIMER_DURATION;
    pthread_mutex_unlock(&timer_mutex);
}

void *timer_thread(void* arg) {
    WINDOW* win = (WINDOW*) arg;

    pthread_mutex_lock(&timer_mutex);
    time_left = TIMER_DURATION;
    timer_running = true;
    pthread_mutex_unlock(&timer_mutex);

    while (true) {
        pthread_mutex_lock(&gameover_mutex);
        if (gameover) {
            pthread_mutex_unlock(&gameover_mutex);
            break; // Esci dal ciclo se il gioco è terminato
        }
        pthread_mutex_unlock(&gameover_mutex);

        pthread_mutex_lock(&timer_mutex);
        if (!timer_running || time_left <= 0) {
            // Cancella il contenuto della finestra del timer
            pthread_mutex_lock(&render_mutex);
            werase(win);
            wrefresh(win);
            pthread_mutex_unlock(&render_mutex);
            pthread_mutex_unlock(&timer_mutex);

            setGameover();
            
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
}
