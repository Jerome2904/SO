#include "frog.h"
#include "game.h"
#include "map.h"
#include "hole.h"
#include "timer.h"
#include "paramThreads.h"
#include "buffer.h"


pthread_mutex_t frog_mutex = PTHREAD_MUTEX_INITIALIZER;

// Inizializza la rana
void init_frog(Frog *frog) {
    frog->x = FROG_ROWS;
    frog->y = MAP_HEIGHT - FROG_COLS;

    // Forma della rana
    char sprite[FROG_ROWS][FROG_COLS] = {
        {'/', 'O', '\\'},
        {'Z', 'U', 'Z'},
    };

    for (int i = 0; i < FROG_ROWS; i++) {
        for (int j = 0; j < FROG_COLS; j++) {
            frog->sprite[i][j] = sprite[i][j];
        }
    }
}

// Disegna la rana sullo schermo
void draw_frog(Frog *frog,WINDOW* win) {
    for (int i = 0; i < FROG_ROWS; i++) {
        for (int j = 0; j < FROG_COLS; j++) {
            wattron(win,COLOR_PAIR(map[frog->y + i][frog->x + j]));
            mvwaddch(win,frog->y + i, frog->x + j, frog->sprite[i][j]);
            wattroff(win,COLOR_PAIR(map[frog->y + i][frog->x + j]));
        }
    }
    pthread_mutex_lock(&render_mutex);
    wrefresh(win);
    pthread_mutex_unlock(&render_mutex);
}

//aggiorna le posizioni della rana.
void update_frog(void* arg,int input){
    FrogThreadParams* prms = (FrogThreadParams*) arg;
    Frog* frog = prms->frog;
    WINDOW* win = prms->win;

    //in base all'input cambiamo la posizione
    switch (input) {
        case KEY_UP:
            //raggiunta una tana, la riempie e rinizia da capo
            if (frog->y == HOLE_Y+1){
                    fillHole(arg);
                    //rimetto la rana nella posizione iniziale
                    frog->x = FROG_ROWS;
                    frog->y = MAP_HEIGHT - FROG_COLS;
                        
                    resetTimer();
                    
                    if(checkHoles()){
                        setGameover();
                    }
                }
                
            //la rana puo andare SEMPRE verso su fin quando non arriva al bordo della tana
            if (frog->y > HOLE_Y+2){
                frog->y--;
            //appena sotto la tana controlliamo che la rana abbia la stessa x di una delle tane. se si la rana puo salire
            }else if (frog->x == HOLE_X1 ||frog->x == HOLE_X2 ||frog->x == HOLE_X3 ||frog->x == HOLE_X4 ||frog->x == HOLE_X5){
                frog->y--;
            }
            break;
        case KEY_DOWN:
            if (frog->y + FROG_ROWS < MAP_HEIGHT - 1) frog->y++;
            break;
        case KEY_LEFT:
            //quando arriva a meta' tana, la rana non puo muoversi di lato
            if (frog->x > 1 && frog->y!=HOLE_Y+1){
                frog->x--;
            }
            break;
        case KEY_RIGHT:
            //quando arriva a meta' tana, la rana non puo muoversi di lato
            if (frog->x + FROG_COLS < MAP_WIDTH - 1 && frog->y!=HOLE_Y+1){
                frog->x++;
            } 
            break;
    }
    
}

// Cancella la rana dalla posizione attuale
void clear_frog(Frog *frog,WINDOW* win) {
    for (int i = 0; i < FROG_ROWS; i++) {
        for (int j = 0; j < FROG_COLS; j++) {
            wattron(win,COLOR_PAIR(map[frog->y + i][frog->x + j]));
            mvwaddch(win,frog->y + i, frog->x + j, ' ');
            wattroff(win,COLOR_PAIR(map[frog->y + i][frog->x + j]));
        }
    }

    pthread_mutex_lock(&render_mutex);
    wrefresh(win);
    pthread_mutex_unlock(&render_mutex);
}

// Funzione del thread della rana
void *frog_thread(void* arg) {
    
    FrogThreadParams* prms = (FrogThreadParams*) arg;
    Frog* frog = prms->frog;
    WINDOW* win = prms->win;

    GameEvent event;
    event.x=frog->x;
    event.y=frog->y;


    while (true) {
        //chiude il thread rana quando termina il gioco
        pthread_mutex_lock(&gameover_mutex);
        if(gameover){
            pthread_mutex_unlock(&gameover_mutex);
            break; 
        }else{
            pthread_mutex_unlock(&gameover_mutex);
        }
        
        int ch = wgetch(win);
        switch (ch){
            case KEY_UP:
            case KEY_DOWN:
            case KEY_LEFT:
            case KEY_RIGHT:
            case 'q':
            case 'Q':
                event.type = ch;
                
                buffer_push(&buffer, event);
        
                break;
            default:
                break;
        }

    }
    pthread_exit(NULL);
}

