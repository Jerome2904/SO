
#include <unistd.h> // Per sleep()
#include "timer.h"
#include "game.h"
#include "frog.h"
#include "map.h"
#include "paramThreads.h"


// Mutex per sincronizzare le chiamate a wrefresh
pthread_mutex_t render_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_map(WINDOW* win);

void start_game() {
    
    // Crea la finestra del timer (altezza 3, subito sopra la finestra di gioco)
    WINDOW *timer_win = newwin(4, MAP_WIDTH, (LINES - MAP_HEIGHT) / 2 - 4, (COLS - MAP_WIDTH) / 2);
    box(timer_win,0,0);
    pthread_mutex_lock(&render_mutex);
    wrefresh(timer_win);
    pthread_mutex_unlock(&render_mutex);
    // Crea la finestra di gioco e di sfondo
    WINDOW *game_win = newwin(MAP_HEIGHT, MAP_WIDTH, (LINES - MAP_HEIGHT) / 2, (COLS - MAP_WIDTH) / 2);
    box(game_win, 0, 0);
    keypad(game_win, TRUE);

    init_bckmap();
    draw_map(game_win);
    
    Frog frog;
    init_frog(&frog);
    FrogThreadParams frogprm = {&frog,game_win};

    // Crea il thread della rana
    pthread_t frog_tid;
    pthread_create(&frog_tid, NULL,frog_thread, &frogprm);
    pthread_detach(frog_tid);

    // Inizializza il timer
    start_timer(timer_win);
    // Mostra la finestra e gestisci il gioco
    while (timer_running && frog_running) {
    }

    pthread_join(frog_tid,NULL);
    werase(timer_win);
    delwin(timer_win);
    // Fine del gioco
    werase(game_win);
    box(game_win,0,0);
    mvwprintw(game_win, 2, (MAP_WIDTH - 12) / 2, "Fine partita!");
    mvwprintw(game_win, 4, (MAP_WIDTH - 28) / 2, "Premi un tasto per continuare...");
    wrefresh(game_win);
    wgetch(game_win);

    
    delwin(game_win);
}

