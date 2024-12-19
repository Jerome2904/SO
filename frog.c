#include "frog.h"
#include "game.h"
#include "map.h"
#include "paramThreads.h"

// Variabile globale per controllare il thread della rana
bool frog_running = true;

pthread_mutex_t frog_mutex = PTHREAD_MUTEX_INITIALIZER;

// Inizializza la rana
void init_frog(Frog *frog) {
    frog->x = FROG_ROWS;
    frog->y = MAP_HEIGHT - FROG_COLS;

    // Forma della rana
    char default_shape[FROG_ROWS][FROG_COLS] = {
        {'/', 'O', '\\'},
        {'Z', 'U', 'Z'},
    };

    for (int i = 0; i < FROG_ROWS; i++) {
        for (int j = 0; j < FROG_COLS; j++) {
            frog->shape[i][j] = default_shape[i][j];
        }
    }
}

// Disegna la rana sullo schermo
void draw_frog(const Frog *frog,WINDOW* win) {
    for (int i = 0; i < FROG_ROWS; i++) {
        for (int j = 0; j < FROG_COLS; j++) {
            wattron(win,COLOR_PAIR(map[frog->y + i][frog->x + j]));
            mvwaddch(win,frog->y + i, frog->x + j, frog->shape[i][j]);
            wattroff(win,COLOR_PAIR(map[frog->y + i][frog->x + j]));
        }
    }
    pthread_mutex_lock(&render_mutex);
    wrefresh(win);
    pthread_mutex_unlock(&render_mutex);
}

// Cancella la rana dalla posizione attuale
void clear_frog(const Frog *frog,WINDOW* win) {
    for (int i = 0; i < FROG_ROWS; i++) {
        for (int j = 0; j < FROG_COLS; j++) {
            wattron(win,COLOR_PAIR(map[frog->y + i][frog->x + j]));
            mvwaddch(win,frog->y + i, frog->x + j, ' ');
            wattroff(win,COLOR_PAIR(map[frog->y + i][frog->x + j]));
        }
    }

    pthread_mutex_lock(&render_mutex);
    wrefresh(win);
    pthread_mutex_unlock(&render_mutex);
}

// Funzione del thread della rana
void *frog_thread(void* arg) {
    FrogThreadParams* prms = (FrogThreadParams*) arg;
    Frog *frog = prms->frog;
    WINDOW* win = prms->win;

    while (frog_running) {
        // Aspetta l'input dell'utente
        int ch = wgetch(win);

        pthread_mutex_lock(&frog_mutex);

        // Cancella la rana dalla posizione attuale
        clear_frog(frog, win);

        // Aggiorna la posizione in base all'input
        switch (ch) {
            case KEY_UP:
                if (frog->y > 1) frog->y--;
                break;
            case KEY_DOWN:
                if (frog->y + FROG_ROWS < MAP_HEIGHT - 1) frog->y++;
                break;
            case KEY_LEFT:
                if (frog->x > 1) frog->x--;
                break;
            case KEY_RIGHT:
                if (frog->x + FROG_COLS < MAP_WIDTH - 1) frog->x++;
                break;
            case 'q':
            case 'Q':
                frog_running = false;
                break;
        }

        // Disegna la rana nella nuova posizione
        draw_frog(frog, win);

        pthread_mutex_unlock(&frog_mutex);
    }
    pthread_exit(NULL);
}
