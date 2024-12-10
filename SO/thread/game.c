
#include <unistd.h> // Per sleep()
#include "timer.h"
#include "game.h"


void start_game(int height,int width,int starty,int startx) {

    // Crea la finestra di gioco
    WINDOW *game_win = newwin(height, width, starty, startx);
    box(game_win, 0, 0);
    keypad(game_win, TRUE);

    // Inizializza il timer
    start_timer(TIMER_DURATION);
    // Mostra la finestra e gestisci il gioco
    while (timer_running) {

        // Aspetta un input non bloccante
        int ch = wgetch(game_win);
        if (ch == 'q' || ch == 'Q') {
            stop_timer();
            break;
        }

    }

    // Fine del gioco
    werase(game_win);
    box(game_win,0,0);
    mvwprintw(game_win, 2, (width - 12) / 2, "Fine partita!");
    mvwprintw(game_win, 4, (width - 28) / 2, "Premi un tasto per continuare...");
    wrefresh(game_win);
    wgetch(game_win);

    delwin(game_win);
}


