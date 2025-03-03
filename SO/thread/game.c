#include "game.h"
#include "frog.h"
#include "consumer.h"
#include "timer.h"
#include "buffer.h"
#include "map.h"


int round_reset_flag = 0;
pthread_mutex_t reset_mutex = PTHREAD_MUTEX_INITIALIZER;

int active_grenades = 0;
pthread_mutex_t grenade_mutex = PTHREAD_MUTEX_INITIALIZER;


void start_game() {
    // Inizializzo la finestra di gioco
    int game_starty = (LINES - MAP_HEIGHT) / 2;
    int game_startx = (COLS - MAP_WIDTH) / 2;

    WINDOW *game_win = newwin(MAP_HEIGHT, MAP_WIDTH, game_starty, game_startx);
    keypad(game_win, TRUE);
    box(game_win, 0, 0);
    init_bckmap();
    init_holes_positions();
    init_map_holes();
    draw_map(game_win);

    WINDOW *info_win = newwin(INFO_HEIGHT, MAP_WIDTH, game_starty + MAP_HEIGHT, game_startx);
    box(info_win, 0, 0);
    wrefresh(info_win);

    // Inizializzo il buffer circolare
    CircularBuffer buffer;
    buffer_init(&buffer);

    // Argomenti per i thread
    ConsumerArgs consumer_args = {&buffer, game_win, info_win};
    FrogArgs frog_args = {&buffer, game_win};
    TimerArgs timer_args = {&buffer};

    pthread_t frog_tid, consumer_tid,timer_tid;

    // Creazione dei thread
    pthread_create(&frog_tid, NULL, frog_thread, &frog_args);
    pthread_create(&timer_tid, NULL, timer_thread, &timer_args);
    pthread_create(&consumer_tid, NULL, consumer_thread, &consumer_args);

    
    pthread_join(consumer_tid, NULL);
    pthread_cancel(frog_tid);
    pthread_cancel(timer_tid);

    buffer_destroy(&buffer);    

    // Pulizia della finestra di gioco
    werase(info_win);
    wrefresh(info_win);
    delwin(info_win);

    werase(game_win);
    wrefresh(game_win);
    delwin(game_win);
}