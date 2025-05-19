#include "game.h"

#define NUM_OPTIONS 3
const char *menu_options[NUM_OPTIONS] = {"GIOCA","ISTRUZIONI", "ESCI"};

void show_instructions();
void exit_program();

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
        init_pair(1, COLOR_RED, COLOR_BLACK);
    }

    // Controllo dimensione minima del terminale
    int min_rows = MAP_HEIGHT + INFO_HEIGHT;
    int min_cols = MAP_WIDTH;
    bool size_confirmed = false;
    while (!size_confirmed) {
        clear();
        mvprintw(LINES/2, (COLS - 35)/2,"Current size: %d rows x %d columns", LINES, COLS);

        if (LINES == min_rows && COLS == min_cols) {
            attron(A_BOLD | COLOR_PAIR(1));
            mvprintw(LINES/2 - 2, (COLS - 26) / 2,"Terminal size is correct!");
            attroff(A_BOLD | COLOR_PAIR(1));
            mvprintw(LINES/2 + 2, (COLS - 44) / 2,"Press ENTER to continue or resize to adjust");
            
            refresh();
            int ch = getch();
            if (ch == '\n') {
                size_confirmed = true;
            }
        } else {
            mvprintw(LINES/2 - 2, (COLS - 28)/2, "Resize terminal before play");
            mvprintw(LINES/2 + 2, (COLS - 43)/2,"Please set at least %d rows and %d columns", min_rows, min_cols);
        }
        refresh();
        usleep(50000);
    }
    clear();

    int selected = 0;
    while (1) {
        // Disegna il menu
        clear();
        mvprintw(10, (COLS - strlen("FROGGER RESURRECTION")) / 2,"FROGGER RESURRECTION");
        
        for (int i = 0; i < NUM_OPTIONS; i++) {
            if (i == selected) {
                attron(A_REVERSE | A_BOLD | COLOR_PAIR(1));
                mvprintw(15 + i*2, (COLS - strlen(menu_options[i])) / 2,"%s", menu_options[i]);
                attroff(A_REVERSE | A_BOLD | COLOR_PAIR(1));
            } else {
                mvprintw(15 + i*2, (COLS - strlen(menu_options[i])) / 2,"%s", menu_options[i]);
            }
        }
        refresh();

        int ch = getch();
        switch (ch) {
            case KEY_UP:
                selected = (selected - 1 + NUM_OPTIONS) % NUM_OPTIONS;
                break;
            case KEY_DOWN:
                selected = (selected + 1) % NUM_OPTIONS;
                break;
            case '\n':
                if (selected == 0) {
                    clear();
                    refresh();
                    start_game();
                }
                else if (selected == 1) {
                    show_instructions();
                }
                else if (selected == 2) {
                    exit_program();
                }
                break;
        }
    }

    endwin();
    return 0;
}

void show_instructions() {
    clear();
    mvprintw(3, (COLS - 11)/2, "ISTRUZIONI");
    mvprintw(6, (COLS - 32)/2, "1. Muovi la rana con le frecce.");
    mvprintw(9, (COLS - 43)/2, "2. Evita gli ostacoli e raggiungi la tana.");
    mvprintw(12, (COLS - 38)/2, "3. Premi SPAZIO per lanciare granate.");
    mvprintw(15, (COLS - 38)/2, "Premi un tasto per tornare al menu...");
    refresh();
    getch();
    clear();
    refresh();
}

void exit_program() {
    clear();
    refresh();
    endwin();
    exit(0);
}