#include "game.h"
#include "frog.h"
#include "consumer.h"
#include "map.h"
#include "timer.h"
#include "crocodile.h"

int game_state = GAME_RUNNING;
int score = INITIAL_SCORE;

void start_game() {
    srand(time(NULL));
    // Creazione pipe
    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        endwin();
        exit(EXIT_FAILURE);
    }

    init_bckmap();
    init_holes_positions();
    init_map_holes();
    draw_map();

    WINDOW *info_win = newwin(INFO_HEIGHT, MAP_WIDTH, MAP_HEIGHT, 0);
    
    // Processo rana
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork rana");
        endwin();
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        close(fd[0]); // Chiudi il lato di lettura della pipe
        frog_process(fd[1]); 
        close(fd[1]); // Chiudi il lato di scrittura della pipe
        exit(0);
    }
    // Processo timer
    pid = fork();
    if (pid < 0) {
        perror("fork timer");
        endwin();
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        close(fd[0]);
        timer_process(fd[1]);
        exit(0);
    }

    // Inizializzo le corsie
    RiverLane lanes[NUM_RIVER_LANES];
    init_lanes(lanes);
    
    for (int i = 0; i < NUM_RIVER_LANES; i++) {
        pid_t spawner = fork();
        if (spawner < 0) {
            perror("fork crocodile spawner");
            endwin();
            exit(EXIT_FAILURE);
        }
        if (spawner == 0) {
            //PROCESSO SPAWNER per corsia i
            srand(time(NULL) ^ getpid());
            close(fd[0]);   // chiudo lettura
            RiverLane lane = lanes[i];
            bool first_spawn = true;
            while (1) {
                // primo spawn non ha ritardo
                if (first_spawn) {
                    first_spawn = false;
                } else {
                    //sleep casuale tra 3 e 6 secondi
                    int delay_ms = 3 + rand() % 3;
                    usleep(delay_ms * 1000000);
                }

                //fork dell'istanza coccodrillo
                pid_t cpid = fork();
                if (cpid < 0) {
                    perror("fork crocodile instance");
                    exit(1);
                }
                if (cpid == 0) {
                    crocodile_process(fd[1], lane); 
                    exit(0);
                }
                int status;
                while (waitpid(-1, &status, WNOHANG) > 0) {
                    ; // ripulisce tutti i figli già morti
                }
            }
        }
    }

    // Processo padre
    consumer(fd[0],fd[1],info_win); 
    close(fd[0]); // Chiudi il lato di lettura della pipe
    close(fd[1]); // Chiudi il lato di scrittura della pipe

    wait(NULL);
    wait(NULL);


    // Mostra messaggio finale
    if (game_state == GAME_WIN)
        game_state_win();
    else    
        game_over();
}

void game_state_win() {
    clear();
    mvprintw(LINES/2 - 1, (COLS - 11)/2, "HAI VINTO!");
    mvprintw(LINES/2, (COLS - 24)/2, "Punteggio Finale: %d", score);
    mvprintw(LINES/2 + 1, (COLS - 29)/2, "Premi 'r' per giocare ancora");
    mvprintw(LINES/2 + 2, (COLS - 21)/2, "Premi 'q' per uscire");
    refresh();

    nodelay(stdscr, FALSE);
    int ch;
    do { ch = getch(); } while (ch != 'q' && ch != 'r');

    if (ch == 'r') {
        restart_game();
    }
}

void game_over() {
    clear();
    mvprintw(LINES/2 - 1, (COLS - 11)/2, "HAI PERSO!");
    mvprintw(LINES/2, (COLS - 24)/2, "Punteggio Finale: %d", score);
    mvprintw(LINES/2 + 1, (COLS - 29)/2, "Premi 'r' per giocare ancora");
    mvprintw(LINES/2 + 2, (COLS - 21)/2, "Premi 'q' per uscire");
    refresh();

    nodelay(stdscr, FALSE);
    int ch;
    do { ch = getch(); } while (ch != 'q' && ch != 'r');

    if (ch == 'r') {
        restart_game();
    }
}

void restart_game() {
    game_state = GAME_RUNNING;
    score = INITIAL_SCORE;
    clear();
    refresh();
    start_game();
}
