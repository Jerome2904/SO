#include "grenade.h"
#include "map.h"
#include <stdlib.h>
#include <unistd.h>

void *grenade_thread(void* arg){
    GrenadeArgs *args = (GrenadeArgs*) arg;
    CircularBuffer *buffer = args->buffer;
    Message msg;
    msg.id=0;


    msg.entity.width = 1;
    msg.entity.height = 1;
    msg.entity.type = ENTITY_GRENADE;
    msg.entity.dx = args->dx;
    msg.entity.speed=args->speed;
    msg.entity.impacted=false;
    
    msg.entity.cooldown=-1;
    msg.entity.is_badcroc=false;
    msg.entity.sprite[0][0]='*';

    if(msg.entity.dx==-1){
        msg.type = MSG_GRENADE_LEFT;
    }else{
        msg.type = MSG_GRENADE_RIGHT;
    }
    // Inizializza la posizione con quella passata
    msg.entity.x = args->start_x;
    msg.entity.y = args->start_y;


    // Aggiorna la posizione finché non raggiunge il bordo sinistro
    while (msg.entity.x > 0 && msg.entity.x < MAP_WIDTH) {
        pthread_mutex_lock(&game_state_mutex);
        if (game_state == GAME_PAUSED){
            pthread_mutex_unlock(&game_state_mutex); //sblocca subito per evitare che rimanga bloccato
            usleep(100000); // aspetta un po e ricontrolla se è ancora in pausa
            continue;
        }
        pthread_mutex_unlock(&game_state_mutex);
        
        pthread_mutex_lock(&game_state_mutex);
        if (game_state == GAME_QUITTING || game_state == GAME_WIN){
            pthread_mutex_unlock(&game_state_mutex);
            pthread_exit(NULL);
        }
        pthread_mutex_unlock(&game_state_mutex);

        usleep(args->speed);
        msg.entity.x += args->dx;
        buffer_push(buffer, msg);
    }

    msg.id=-1;
    buffer_push(buffer, msg);

    
    pthread_exit(NULL);
}

// Disegna la granata sullo schermo
void draw_grenade(WINDOW *win, Entity *grenade) {
    if(grenade->x>0 && grenade->x<MAP_WIDTH-1){
        wattron(win,COLOR_PAIR(map[grenade->y][grenade->x]));
        mvwaddch(win,grenade->y, grenade->x, '*');
        wattroff(win,COLOR_PAIR(map[grenade->y][grenade->x]));
        wrefresh(win);
    }
    
}
void clear_grenade(WINDOW *win, Entity *grenade) {
    if(grenade->x>0 && grenade->x<MAP_WIDTH-1){
        wattron(win,COLOR_PAIR(map[grenade->y][grenade->x]));
        mvwaddch(win,grenade->y, grenade->x, ' ');
        wattroff(win,COLOR_PAIR(map[grenade->y][grenade->x]));
        wrefresh(win);
    }
}