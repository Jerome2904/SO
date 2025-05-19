#include "game.h"
#include "frog.h"
#include "consumer.h"
#include "map.h"
#include "timer.h"
#include "crocodile.h"

int game_state = GAME_RUNNING;
int score = INITIAL_SCORE;
pid_t spawner_pids[NUM_RIVER_LANES];

void start_game() {
    srand(time(NULL));
    // Creazione pipe
    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        endwin();
        exit(EXIT_FAILURE);
    }

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

    
    create_spawners(fd[1],fd[0], lanes, spawner_pids, NUM_RIVER_LANES);

    // Processo padre
    consumer(fd[0], fd[1], info_win,spawner_pids, NUM_RIVER_LANES,lanes);
    close(fd[0]); // Chiudi il lato di lettura della pipe
    close(fd[1]); // Chiudi il lato di scrittura della pipe


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

void create_spawners(int fd_write,int fd_read, RiverLane lanes[],pid_t spawner_pids[], int n_lanes) {
    for (int i = 0; i < n_lanes; i++) {
        pid_t sp = fork();
        if (sp < 0) { perror("fork spawner"); exit(1); }
        spawner_pids[i] = sp;
        if (sp == 0) {
            // PROCESSO SPAWNER per corsia i
            setpgid(0, 0); //fa sì che ogni spawner diventi capo di un suo process group
            close(fd_read);
            //faccio lo XOR con il pid per avere un seme diverso per ogni spawner
            srand(time(NULL) ^ getpid()); 
            RiverLane lane = lanes[i];
            bool first_spawns = true;
            while (1) {
                //primo spawn non ha ritardo
                if (!first_spawns) {
                    //sleep casuale tra 3 e 6 secondi
                    int delay_ms = 3 + rand() % 3;
                    usleep(delay_ms * 1000000);
                }
                first_spawns = false;
                pid_t c = fork();
                if (c == 0) {
                    close(fd_read);
                    crocodile_process(fd_write, lane);
                    exit(0);
                }
                clean();
            }
        }
        
    }
}

void kill_all_spawners(pid_t spawner_pids[], int n) {
    for (int i = 0; i < n; i++) {
        pid_t pgid = spawner_pids[i];
        if (pgid > 0) {
            //uccide tutto il process‑group (spawner + coccodrilli figli)
            kill(-pgid, SIGKILL);       
            //ripulisce lo spawner
            waitpid(pgid, NULL, 0);     
            spawner_pids[i] = 0;
        }
    }
}