#include "grenade.h"
#include "map.h"
#include <stdlib.h>
#include <unistd.h>

void *grenade_left_thread(void *arg) {
    GrenadeArgs *args = (GrenadeArgs*) arg;
    CircularBuffer *buffer = args->buffer;
    Message msg;
    msg.type = MSG_GRENADE_LEFT;
    msg.id = -1;
    msg.entity.width = 1;
    msg.entity.height = 1;
    msg.entity.type = ENTITY_GRENADE;
    msg.entity.dx = args->dx;
    // Inizializza la posizione con quella passata
    msg.entity.x = args->start_x;
    msg.entity.y = args->start_y;
    // Aggiorna la posizione finché non raggiunge il bordo sinistro
    while (msg.entity.x > 0) {
        usleep(args->speed);
        msg.entity.x += args->dx;
        buffer_push(buffer, msg);
    }
    return NULL;
}

void *grenade_right_thread(void *arg) {
    GrenadeArgs *args = (GrenadeArgs*) arg;
    CircularBuffer *buffer = args->buffer;
    Message msg;
    msg.type = MSG_GRENADE_RIGHT;
    msg.id = 1;
    msg.entity.width = 1;
    msg.entity.height = 1;
    msg.entity.type = ENTITY_GRENADE;
    msg.entity.dx = args->dx;
    // Inizializza la posizione con quella passata
    msg.entity.x = args->start_x;
    msg.entity.y = args->start_y;
    
    // Aggiorna la posizione finché non raggiunge il bordo destro
    while (msg.entity.x < MAP_WIDTH) {
        usleep(args->speed);
        msg.entity.x += args->dx; 
        buffer_push(buffer, msg);
    }
    return NULL;
}

// Disegna la granata sullo schermo
void draw_grenade(WINDOW *win, Entity *grenade) {
    if(grenade->x>0 && grenade->x<MAP_WIDTH-1){
        wattron(win,COLOR_PAIR(map[grenade->y][grenade->x]));
        mvwaddch(win,grenade->y, grenade->x, '*');
        wattroff(win,COLOR_PAIR(map[grenade->y][grenade->x]));
        pthread_mutex_lock(&render_mutex);
        wrefresh(win);
        pthread_mutex_unlock(&render_mutex);
    }
    
}
void clear_grenade(WINDOW *win, Entity *grenade) {
    if(grenade->x>0 && grenade->x<MAP_WIDTH-1){
        wattron(win,COLOR_PAIR(map[grenade->y][grenade->x]));
        mvwaddch(win,grenade->y, grenade->x, ' ');
        wattroff(win,COLOR_PAIR(map[grenade->y][grenade->x]));
        pthread_mutex_lock(&render_mutex);
        wrefresh(win);
        pthread_mutex_unlock(&render_mutex);
    }
}