#include "game.h"

#define NUM_OPTIONS 3
const char *menu_options[NUM_OPTIONS] = {"GIOCA","ISTRUZIONI", "ESCI"};
const char *diff_options[NUM_OPTIONS] = {"FACILE","NORMALE","DIFFICILE"};

void show_instructions();
void exit_program();
Difficulty show_difficulty_menu(WINDOW* win);

Difficulty difficulty = NORMAL;

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

    //Inizializzo la finestra iniziale
    int menu_starty = (LINES - MAP_HEIGHT) / 2;
    int menu_startx = (COLS - MAP_WIDTH) / 2;

    WINDOW *menu_win=newwin(MAP_HEIGHT, MAP_WIDTH, menu_starty, menu_startx);
    keypad(menu_win, TRUE);
    box(menu_win, 0, 0);
    refresh();


    int selected = 0;
    while (1) {
        // Disegna il menu
        werase(menu_win);
        box(menu_win,0,0);
        mvwprintw(menu_win,5, (MAP_WIDTH - strlen("FROGGER RESURRECTION")) / 2,"FROGGER RESURRECTION");
        
        for (int i = 0; i < NUM_OPTIONS; i++) {
            if (i == selected) {
                wattron(menu_win,A_REVERSE | A_BOLD | COLOR_PAIR(1));
                mvwprintw(menu_win,10 + i*2, (MAP_WIDTH - strlen(menu_options[i])) / 2,"%s", menu_options[i]);
                wattroff(menu_win,A_REVERSE | A_BOLD | COLOR_PAIR(1));
            } else {
                mvwprintw(menu_win,10 + i*2, (MAP_WIDTH - strlen(menu_options[i])) / 2,"%s", menu_options[i]);
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
            case '\n':
                if (selected == 0) {
                    werase(menu_win);
                    wrefresh(menu_win);
                    difficulty = show_difficulty_menu(menu_win);
                    werase(menu_win);
                    start_game();
                }
                else if (selected == 1) {
                    show_instructions(menu_win);
                }
                else if (selected == 2) {
                    exit_program();
                }
                break;
        }
    }
    delwin(menu_win);
    endwin();
    return 0;
}

void show_instructions(WINDOW* win) {
    werase(win);
    box(win,0,0);
    mvwprintw(win,5,(MAP_WIDTH-strlen("ISTRUZIONI"))/2, "ISTRUZIONI");
    mvwprintw(win,10,(MAP_WIDTH-strlen("1. Muovi la rana con le frecce."))/2, "1. Muovi la rana con le frecce.");
    mvwprintw(win,12,(MAP_WIDTH-strlen("2. Evita gli ostacoli e raggiungi la tana."))/2, "2. Evita gli ostacoli e raggiungi la tana.");
    mvwprintw(win,14,(MAP_WIDTH-strlen("3. Premi SPAZIO per lanciare granate."))/2, "3. Premi SPAZIO per lanciare granate.");
    mvwprintw(win,16,(MAP_WIDTH-strlen("Premi un tasto per tornare al menu..."))/2, "Premi un tasto per tornare al menu...");
    wrefresh(win);
    getch();
    werase(win);
    wrefresh(win);
}

void exit_program() {
    clear();
    refresh();
    endwin();
    exit(0);
}

Difficulty show_difficulty_menu(WINDOW* win) {
    int sel = 1; //difficoltà predefinita NORMAL
    keypad(stdscr, TRUE);
    curs_set(0);

    while (1) {
        werase(win);
        box(win,0,0);
        mvwprintw(win,5, (MAP_WIDTH - strlen("Seleziona difficolta'"))/2, "Seleziona difficolta'");
        for (int i = 0; i < NUM_OPTIONS; i++) {
            int y = 10 + i*2;
            int x = (MAP_WIDTH - strlen(diff_options[i]))/2;
            if (i == sel) {
                wattron(win,A_REVERSE | A_BOLD);
                mvwprintw(win,y, x, "%s", diff_options[i]);
                wattroff(win,A_REVERSE | A_BOLD);
            } else {
                mvwprintw(win, y, x, "%s", diff_options[i]);
            }
        }
        wrefresh(win);
        int ch = getch();
        switch (ch) {
            case KEY_UP: sel = (sel + NUM_OPTIONS - 1) % NUM_OPTIONS; break;
            case KEY_DOWN: sel = (sel + 1) % NUM_OPTIONS; break;
            case '\n': return sel;
        }
    }
}
