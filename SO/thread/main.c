#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"

#define NUM_OPTIONS 3

void show_instructions(WINDOW *menu_win);
void exit_program(WINDOW *menu_win);

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
        init_pair(2, COLOR_WHITE, COLOR_BLUE); // Colori per sfondo istruzioni
    }

    // Dimensioni e posizione della finestra del menu
    int menu_height = 10, menu_width = 40;
    int menu_starty = (LINES - menu_height) / 2;
    int menu_startx = (COLS - menu_width) / 2;

    WINDOW *menu_win=newwin(menu_height, menu_width, menu_starty, menu_startx);
    keypad(menu_win, TRUE);
    box(menu_win, 0, 0);

    const char *menu_options[NUM_OPTIONS] = {"Gioca", "Istruzioni", "Esci"};
    int selected = 0;

    while (1) {
        // Disegna il menu
        werase(menu_win);
        box(menu_win, 0, 0);
        mvwprintw(menu_win, 1, (menu_width - 22) / 2, "Frogger Resurrection");
        for (int i = 0; i < NUM_OPTIONS; i++) {
            if (i == selected) {
                wattron(menu_win, COLOR_PAIR(1));
                mvwprintw(menu_win, 3 + i, (menu_width - strlen(menu_options[i])) / 2, "%s", menu_options[i]);
                wattroff(menu_win, COLOR_PAIR(1));
            } else {
                mvwprintw(menu_win, 3 + i, (menu_width - strlen(menu_options[i])) / 2, "%s", menu_options[i]);
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
                    start_game(menu_height,menu_width,menu_starty,menu_startx);
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
    int inst_height = 15, inst_width = 50;
    int inst_starty = (LINES - inst_height) / 2;
    int inst_startx = (COLS - inst_width) / 2;

    WINDOW *inst_win=newwin(inst_height, inst_width, inst_starty, inst_startx);
    keypad(inst_win,TRUE);
    box(inst_win,0,0);

    // Disegna le istruzioni
    werase(inst_win);
    box(inst_win, 0, 0);
    wattron(inst_win, COLOR_PAIR(2));
    mvwprintw(inst_win, 1, (inst_width - 11) / 2, "Istruzioni");
    wattroff(inst_win, COLOR_PAIR(2));

    mvwprintw(inst_win, 3, 2, "1. Muovi la rana con le frecce.");
    mvwprintw(inst_win, 4, 2, "2. Evita gli ostacoli e raggiungi la tana.");
    mvwprintw(inst_win, 5, 2, "3. Premi SPAZIO per lanciare granate.");
    mvwprintw(inst_win, 7, 2, "Premi un tasto per tornare al menu...");
    wrefresh(inst_win);

    getch(); // Aspetta un input
    werase(inst_win);
    wrefresh(inst_win);
    delwin(inst_win);

    // Ritorna al menu
    wrefresh(menu_win);
}

void exit_program(WINDOW *menu_win) {
    werase(menu_win);
    wrefresh(menu_win);
    delwin(menu_win);
    endwin();
    printf("Grazie per aver giocato. Arrivederci!\n");
    exit(0);
}

