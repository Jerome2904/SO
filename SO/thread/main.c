
#include "game.h"

int main() {
    // Inizializzazione di ncurses
    initscr();
    clear();
    noecho();
    curs_set(0);
    cbreak();
    keypad(stdscr, TRUE);

    // Abilita i colori se supportati
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK); // Colori per opzione selezionata
    }

    //Inizializzo la finestra iniziale
    int menu_starty = (LINES - MAP_HEIGHT) / 2;
    int menu_startx = (COLS - MAP_WIDTH) / 2;

    WINDOW *menu_win=newwin(MAP_HEIGHT, MAP_WIDTH, menu_starty, menu_startx);
    keypad(menu_win, TRUE);
    box(menu_win, 0, 0);
    refresh();

    const char *menu_options[NUM_OPTIONS] = {"GIOCA","ISTRUZIONI", "ESCI"};
    int selected = 0;

    while (1) {
        // Disegna il menu
        werase(menu_win);
        box(menu_win, 0, 0);
        mvwprintw(menu_win, 10, (MAP_WIDTH - strlen("FROGGER RESURRECTION")) / 2, "FROGGER RESURRECTION");
        for (int i = 0; i < NUM_OPTIONS; i++) {
            if (i == selected) {
                wattron(menu_win, A_REVERSE | A_BOLD | COLOR_PAIR(1));
                mvwprintw(menu_win, 15 + i*2, (MAP_WIDTH - strlen(menu_options[i])) / 2, "%s", menu_options[i]);
                wattroff(menu_win, A_REVERSE | A_BOLD | COLOR_PAIR(1));
            } else {
                mvwprintw(menu_win, 15 + i*2, (MAP_WIDTH - strlen(menu_options[i])) / 2, "%s", menu_options[i]);
            }
        }
        wrefresh(menu_win);

        int ch = getch();
        switch (ch) {
            case KEY_UP:
                selected = (selected - 1 + NUM_OPTIONS) % NUM_OPTIONS;
                break;
            case KEY_DOWN:
                selected = (selected + 1) % NUM_OPTIONS;
                break;
            case '\n': // Invio
                if (selected == 0) {
                    werase(menu_win);
                    wrefresh(menu_win);
                    start_game();
                    exit_program(menu_win);
                } else if (selected == 1) {
                    show_instructions(menu_win);
                } else if (selected == 2) {
                    exit_program(menu_win);
                }
                break;
            
        }
    }

    delwin(menu_win);
    endwin();
    return 0;
}


void show_instructions(WINDOW *menu_win) {

    // Disegna le istruzioni
    werase(menu_win);
    box(menu_win, 0, 0);
    mvwprintw(menu_win, 3, (MAP_WIDTH - 11)/2, "ISTRUZIONI");

    mvwprintw(menu_win, 6, (MAP_WIDTH - 32)/2, "1. Muovi la rana con le frecce.");
    mvwprintw(menu_win, 9, (MAP_WIDTH - 43)/2, "2. Evita gli ostacoli e raggiungi la tana.");
    mvwprintw(menu_win, 12, (MAP_WIDTH - 38)/2, "3. Premi SPAZIO per lanciare granate.");
    mvwprintw(menu_win, 15, (MAP_WIDTH - 38)/2, "Premi un tasto per tornare al menu...");
    wrefresh(menu_win);

    getch(); // Aspetta un input
    werase(menu_win);
    wrefresh(menu_win);
}

void exit_program(WINDOW *menu_win) {
    werase(menu_win);
    wrefresh(menu_win);
    delwin(menu_win);
    endwin();
    exit(0);
}
