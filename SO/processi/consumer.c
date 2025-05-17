#include "game.h"
#include "consumer.h"
#include "frog.h"
#include "map.h"
#include "timer.h"
#include "crocodile.h"
#include "grenade.h"





void consumer(int fd_read,int fd_write, WINDOW *info_win) {
    Message msg;
    Entity frog;

    int time = ROUND_TIME;
    int lives = NUM_LIVES;
    int score = INITIAL_SCORE;
    int hole_index = -1;
    int holes_reached = 0;
    
    // stato corsie (coccodrilli)
    CrocLaneState lanes_state[NUM_RIVER_LANES] = {0};

    // stato granate
    Entity grenades[MAX_GRENADES]   = {0};
    Entity gren_prev[MAX_GRENADES]  = {0};
    pid_t  gren_pid[MAX_GRENADES]   = {0};
    bool   gren_active[MAX_GRENADES] = {false};
    int active_grenades = 0;

    // stato proiettili
    Entity  projectiles[MAX_PROJECTILES] = {0};
    Entity  proj_prev[MAX_PROJECTILES]   = {0};
    pid_t   proj_pid[MAX_PROJECTILES]    = {0};
    bool    proj_active[MAX_PROJECTILES] = {false};
    int     proj_count = 0;

    frog_init(&frog);

    
    int lane =-1;
    pid_t id =-1;
    //tiene lo stato della corsia 
    CrocLaneState *lane_state = NULL;
    //direzione della granata
    int dir = 0;

    while (1) {
        if (read(fd_read, &msg, sizeof(msg)) <= 0) break;
        
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
                lane = msg.lane_id;
                id = msg.id;
                lane_state = &lanes_state[lane];

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
                lane = msg.lane_id;
                id = msg.id;
                lane_state = &lanes_state[lane];

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
                lane = msg.lane_id;
                id = msg.id;
                lane_state = &lanes_state[lane];

                for (int i = 0; i < MAX_CROCS_PER_LANE; i++) {
                    if (lane_state->active[i] && lane_state->pid[i] == id) {
                        clear_entity(&lane_state->prev[i]);
                        lane_state->active[i] = false;
                        break;
                    }
                }
                break;

            case MSG_GRENADE_SPAWN: {
                // se ci sono già 2 granate attive, non spawnarne altre
                if (gren_active[0] || gren_active[1]) {
                    break;
                }
                //posizione della rana
                int fx = msg.entity.x, fy = msg.entity.y;
                // Granata SINISTRA -> slot 0
                pid_t g0 = fork();
                if (g0 < 0) {
                    perror("fork grenade left");
                    exit(EXIT_FAILURE);
                }
                if (g0 == 0) {
                    close(fd_read);
                    grenade_process(fd_write, fx, fy, -1);
                } else {
                    gren_active[0]  = true;
                    gren_pid[0]     = g0;
                    gren_prev[0]    = msg.entity;
                    grenades[0]     = msg.entity;
                }

                // Granata DESTRA -> slot 1
                pid_t g1 = fork();
                if (g1 < 0) {
                    perror("fork grenade right");
                    exit(EXIT_FAILURE);
                }
                if (g1 == 0) {
                    close(fd_read);
                    grenade_process(fd_write, fx, fy, +1);
                } else {
                    gren_active[1]  = true;
                    gren_pid[1]     = g1;
                    gren_prev[1]    = msg.entity;
                    grenades[1]     = msg.entity;
                }
                active_grenades = MAX_GRENADES;
                break;
            }
            case MSG_GRENADE_UPDATE:
                id = msg.id;
                for (int i = 0; i < MAX_GRENADES; i++) {
                    if (gren_active[i] && gren_pid[i] == id) {
                        clear_grenade(&gren_prev[i]);
                        grenades[i] = msg.entity;
                        gren_prev[i] = grenades[i];
                        draw_grenade(&grenades[i]);
                        break;
                    }
                }
                break;

            case MSG_GRENADE_DESPAWN:
                id = msg.id;
                for (int i = 0; i < MAX_GRENADES; i++) {
                    if (gren_active[i] && gren_pid[i] == id) {
                        clear_grenade(&gren_prev[i]);
                        gren_active[i] = false;
                        break;
                    }
                }
                break;

            case MSG_PROJECTILE_SPAWN: {
                //se siamo già al massimo non sparare
                if (proj_count >= MAX_PROJECTILES) break;
                //posizione del coccodrillo
                int cx = msg.entity.x;
                int cy = msg.entity.y;
                int dir = msg.entity.dx;

                //fork del processo proiettile
                pid_t p = fork();
                if (p < 0) { perror("fork projectile"); exit(EXIT_FAILURE); }
                if (p == 0) {
                    close(fd_read);
                    projectile_process(fd_write, cx, cy, dir);
                }

                //padre registra il nuovo proiettile nel primo slot libero
                for (int i = 0; i < MAX_PROJECTILES; i++) {
                    if (!proj_active[i]) {
                        proj_active[i]   = true;
                        proj_pid[i] = p;
                        proj_prev[i] = msg.entity;
                        projectiles[i] = msg.entity;
                        proj_count++;
                        break;
                    }
                }
                break;
            }
            case MSG_PROJECTILE_UPDATE: {
                id = msg.id;
                for (int i = 0; i < MAX_PROJECTILES; i++) {
                    if (proj_active[i] && proj_pid[i] == id) {
                        clear_projectile(&proj_prev[i]);
                        projectiles[i] = msg.entity;
                        draw_projectile(&projectiles[i]);
                        proj_prev[i] = projectiles[i];
                        break;
                    }
                }
                break;
            }

            case MSG_PROJECTILE_DESPAWN: {
                id = msg.id;
                for (int i = 0; i < MAX_PROJECTILES; i++) {
                    if (proj_active[i] && proj_pid[i] == id) {
                        clear_projectile(&proj_prev[i]);
                        proj_active[i] = false;
                        proj_count--;
                        break;
                    }
                }
                break;
            }

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