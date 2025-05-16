#include "game.h"
#include "consumer.h"
#include "frog.h"
#include "map.h"
#include "timer.h"
#include "crocodile.h"


CrocLaneState lanes_state[NUM_RIVER_LANES];

void consumer(int fd_read, WINDOW *info_win) {
    Message msg;
    Entity frog;

    int time = ROUND_TIME;
    int lives = NUM_LIVES;
    int score = INITIAL_SCORE;

    frog_init(&frog);

    while (1) {
        if (read(fd_read, &msg, sizeof(msg)) <= 0) break;
        // Array per coccodrilli
        int lane = msg.lane_id;
        pid_t id = msg.id;
        CrocLaneState *lane_state = &lanes_state[lane];

        switch (msg.type) {
            case MSG_TIMER_TICK:
                time--;
                break;

            case MSG_FROG_UPDATE:
                clear_entity(&frog);
                frog.x = msg.entity.x;
                frog.y = msg.entity.y;
                break;

            case MSG_CROC_SPAWN:
                for (int i = 0; i < MAX_CROCS_PER_LANE; i++) {
                    if (!lane_state->active[i]) {
                        lane_state->crocs[i] = msg.entity;
                        lane_state->prev[i] = msg.entity;
                        lane_state->pid[i] = id;
                        lane_state->active[i] = true;
                        draw_entity(&lane_state->crocs[i]);
                        break;
                    }
                }
                break;

            case MSG_CROC_UPDATE:
                // cerca lo slot giusto
                for (int i = 0; i < MAX_CROCS_PER_LANE; i++) {
                    if (lane_state->active[i] && lane_state->pid[i] == id) {
                        clear_entity(&lane_state->prev[i]);
                        lane_state->crocs[i] = msg.entity;
                        lane_state->prev[i] = lane_state->crocs[i];
                        draw_entity(&lane_state->crocs[i]);
                        break;
                    }
                }
                break;

            case MSG_CROC_DESPAWN:
                for (int i = 0; i < MAX_CROCS_PER_LANE; i++) {
                    if (lane_state->active[i] && lane_state->pid[i] == id) {
                        clear_entity(&lane_state->prev[i]);
                        lane_state->active[i] = false;
                        break;
                    }
                }
                break;

            
            default:
                break;
        }
        //la rana sarà sempre visibile
        draw_entity(&frog);
    
        // Aggiorna info_win
        werase(info_win);
        mvwprintw(info_win, 1, 2,"Lives: %-25dScore: %-25dTime: %-3d",lives, score, time);
        wrefresh(info_win);

        refresh();
    }
}


void draw_entity(Entity *entity) {
    for (int i = 0; i < entity->height; i++) {
        for (int j = 0; j < entity->width; j++) {
            attron(COLOR_PAIR(map[entity->y + i][entity->x + j]));
            mvaddch(entity->y + i, entity->x + j, entity->sprite[i][j]);
            attroff(COLOR_PAIR(map[entity->y + i][entity->x + j]));
        }
    }
}

// Cancella una qualsiasi entità dallo schermo
void clear_entity(Entity *entity) {
    for (int i = 0; i < entity->height; i++) {
        for (int j = 0; j < entity->width; j++) {
            attron(COLOR_PAIR(map[entity->y + i][entity->x + j]));
            mvaddch(entity->y + i, entity->x + j, ' ');
            attroff(COLOR_PAIR(map[entity->y + i][entity->x + j]));
        }
    }
}