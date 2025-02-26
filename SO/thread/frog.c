#include "frog.h"
#include "game.h"
#include "buffer.h"
#include "map.h"
#include "consumer.h"
#include "grenade.h"


void *frog_thread(void *arg) {
    FrogArgs *args = (FrogArgs *)arg;
    CircularBuffer *buffer = args->buffer;
    WINDOW *game_win = args->win;
    Entity frog;
    frog_init(&frog);
    int ch;
    Message msg;
    pthread_t grenade_left_tid, grenade_right_tid;

    while (1) {
        // Controllo eventuale reset
        pthread_mutex_lock(&reset_mutex);
        if (round_reset_flag) {
            frog.x = (MAP_WIDTH - frog.width) / 2;
            frog.y = MAP_HEIGHT - frog.height - 1;
            round_reset_flag = 0;
        }
        pthread_mutex_unlock(&reset_mutex);
        
        ch = wgetch(game_win);
        switch (ch) {
            case KEY_UP:
                frog.y -= VERTICAL_JUMP;
                //la rana puo andare sempre verso su fin quando non arriva al bordo della tana
                if (frog.y == HOLE_Y) {
                    // se la rana si trova nella stessa colonna di una tana può entrare
                    if (frog.x == HOLE_X1 || frog.x == HOLE_X2 || frog.x == HOLE_X3 || frog.x == HOLE_X4 || frog.x == HOLE_X5) {
                        frog.y == HOLE_Y; // Enta nella tana 

                        int hole_index = check_hole_reached(&frog);
                        if (tane[hole_index].occupied) frog.y += VERTICAL_JUMP; // Se la tana è già stata raggiunta, non è più possibile entrare
                    } 
                    else 
                        frog.y += VERTICAL_JUMP; 
                }
                break;
            case KEY_DOWN:
                frog.y += VERTICAL_JUMP;
                break;
            case KEY_LEFT:
                frog.x -= HORIZONTAL_JUMP;
                break;
            case KEY_RIGHT:
                frog.x += HORIZONTAL_JUMP;
                break;
            case ' ':
            {
                // Allocazione dinamica dei parametri
                GrenadeArgs *grenade_args_left = malloc(sizeof(GrenadeArgs));
                grenade_args_left->buffer = buffer;
                grenade_args_left->start_x = frog.x-1;
                grenade_args_left->start_y = frog.y;
                grenade_args_left->dx = -1;
                grenade_args_left->speed = 40000;
                
                GrenadeArgs *grenade_args_right = malloc(sizeof(GrenadeArgs));
                grenade_args_right->buffer = buffer;
                grenade_args_right->start_x = frog.x + frog.width;
                grenade_args_right->start_y = frog.y;
                grenade_args_right->dx = 1;
                grenade_args_right->speed = 40000;

                pthread_create(&grenade_left_tid, NULL, grenade_left_thread, grenade_args_left);
                pthread_create(&grenade_right_tid, NULL, grenade_right_thread, grenade_args_right);
                pthread_detach(grenade_left_tid);
                pthread_detach(grenade_right_tid);
                break;
            }
            case 'q':
            case 'Q':
                
                
                break;
            default:
                break;
        }
        
        msg.type = MSG_FROG_UPDATE;
        msg.entity = frog;
        buffer_push(buffer, msg);
        usleep(50000);
    }
    return NULL;
}

void frog_init(Entity *frog) {
    frog->x = (MAP_WIDTH - FROG_WIDTH ) / 2 ;
    frog->y = MAP_HEIGHT - FROG_HEIGHT - 1;
    frog->width = FROG_WIDTH;
    frog->height = FROG_HEIGHT;
    frog->type = ENTITY_FROG;
    // Forma della rana
    char sprite[FROG_HEIGHT][FROG_WIDTH] = {
        {'v', 'O', 'v'},
        {'w', 'U', 'w'},
    };

    for (int i = 0; i < FROG_HEIGHT; i++) {
        for (int j = 0; j < FROG_WIDTH; j++) {
            frog->sprite[i][j] = sprite[i][j];
        }
    }
}



// Disegna la rana sullo schermo
void draw_frog(WINDOW *win, Entity *frog) {
    for (int i = 0; i < frog->height; i++) {
        for (int j = 0; j < frog->width; j++) {
            wattron(win,COLOR_PAIR(map[frog->y + i][frog->x + j]));
            mvwaddch(win,frog->y + i, frog->x + j, frog->sprite[i][j]);
            wattroff(win,COLOR_PAIR(map[frog->y + i][frog->x + j]));
        }
    }
    pthread_mutex_lock(&render_mutex);
    wrefresh(win);
    pthread_mutex_unlock(&render_mutex);
}
void clear_frog(WINDOW *win, Entity *frog) {
    for (int i = 0; i < frog->height; i++) {
        for (int j = 0; j < frog->width; j++) {
            wattron(win,COLOR_PAIR(map[frog->y + i][frog->x + j]));
            mvwaddch(win,frog->y + i, frog->x + j,' ');
            wattroff(win,COLOR_PAIR(map[frog->y + i][frog->x + j]));
        }
    }
    pthread_mutex_lock(&render_mutex);
    wrefresh(win);
    pthread_mutex_unlock(&render_mutex);
}