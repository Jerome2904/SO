
#include <unistd.h> // Per sleep()
#include "timer.h"
#include "game.h"
#include "frog.h"
#include "map.h"
#include "paramThreads.h"
#include "hole.h"
#include "buffer.h"


bool gameover=false;
pthread_mutex_t render_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t gameover_mutex = PTHREAD_MUTEX_INITIALIZER;
CircularBuffer buffer;

void start_game() {
    
    // Crea la finestra del timer (altezza 3, subito sopra la finestra di gioco)
    WINDOW *timer_win = newwin(4, MAP_WIDTH, (LINES - MAP_HEIGHT) / 2 - 4, (COLS - MAP_WIDTH) / 2);
    box(timer_win,0,0);
    pthread_mutex_lock(&render_mutex);
    wrefresh(timer_win);
    pthread_mutex_unlock(&render_mutex);
    // Crea la finestra di gioco
    WINDOW *game_win = newwin(MAP_HEIGHT, MAP_WIDTH, (LINES - MAP_HEIGHT) / 2, (COLS - MAP_WIDTH) / 2);
    box(game_win, 0, 0);
    keypad(game_win, TRUE);
    //inizializzo le tane
    init_holes();
    //inizializzo la matrice rappresentante la mappa
    init_bckmap();
    init_map_holes();
    //disegno la mappa su schermo
    draw_map(game_win);

    //inizializzo il buffer
    buffer_init(&buffer);

    //inizializzo la rana
    Frog frog;
    init_frog(&frog);
    FrogThreadParams frogprm = {&frog,game_win};
    // Crea il thread della rana
    pthread_t frog_tid;
    pthread_create(&frog_tid, NULL,frog_thread, &frogprm);
    // Inizializza il timer
    pthread_t timer_tid;
    pthread_create(&timer_tid, NULL, timer_thread, timer_win);     
    

    GameEvent event;
    
    while(true){
        pthread_mutex_lock(&gameover_mutex);
        if(gameover){
            //aggiorno la mappa
            pthread_mutex_lock(&render_mutex);
            werase(timer_win);
            wrefresh(timer_win);
            pthread_mutex_unlock(&render_mutex);

            pthread_mutex_unlock(&gameover_mutex);
            break;
        }
        pthread_mutex_unlock(&gameover_mutex);

        pthread_mutex_lock(&buffer.mutex);
        if (!buffer.is_empty){
            event= buffer_pop(&buffer);
            pthread_mutex_unlock(&buffer.mutex);
            switch (event.type){
                case KEY_UP:
                case KEY_LEFT:
                case KEY_RIGHT:
                case KEY_DOWN:
                    clear_frog(&frog,game_win);
                    update_frog(&frogprm,event.type);
                    draw_frog(&frog,game_win);
                    break;
                case 'q':
                case 'Q':
                    setGameover();
                    break;
                default:
                    break;
            }
        }else{
            pthread_mutex_unlock(&buffer.mutex);
            usleep(2000);
        }
    }

    // Aspetta che i thread terminino
    pthread_join(frog_tid, NULL);
    pthread_join(timer_tid, NULL);

    werase(timer_win);
    wrefresh(timer_win);
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

void setGameover(){
    pthread_mutex_lock(&gameover_mutex);
    gameover=true;
    pthread_mutex_unlock(&gameover_mutex);
}

