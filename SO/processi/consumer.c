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

    //coordinate Y delle corsie
    int lane_y[NUM_RIVER_LANES];
    lane_y[0] = MAP_HEIGHT - (BOTTOM_SIDEWALK+FROG_HEIGHT);
    for (int l = 1; l < NUM_RIVER_LANES; l++) {
        lane_y[l] = lane_y[l-1] - FROG_HEIGHT;
    }

    // stato granate
    Entity grenades[MAX_GRENADES] = {0};
    Entity gren_prev[MAX_GRENADES] = {0};
    pid_t  gren_pid[MAX_GRENADES] = {0};
    bool   gren_active[MAX_GRENADES] = {false};
    int active_grenades = 0;

    // stato proiettili
    Entity  projectiles[MAX_PROJECTILES] = {0};
    Entity  proj_prev[MAX_PROJECTILES] = {0};
    pid_t   proj_pid[MAX_PROJECTILES] = {0};
    bool    proj_active[MAX_PROJECTILES] = {false};
    int     proj_count = 0;

    int frog_start_x = (MAP_WIDTH - FROG_WIDTH) / 2;
    int frog_start_y = MAP_HEIGHT - FROG_HEIGHT;

    frog_init(&frog);
    Entity frog_prev = frog;

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
                //sposto rana
                frog_move(&frog, &frog_prev,msg.entity.dx,msg.entity.dy);

                //controllo caduta in acqua
                frog_water_check(&frog, &frog_prev, lanes_state,lane_y,&lives,frog_start_x,frog_start_y);

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
                        //aggiorna coccodrillo
                        clear_entity(&lane_state->prev[i]);
                        lane_state->crocs[i] = msg.entity;
                        lane_state->prev[i] = lane_state->crocs[i];
                        draw_entity(&lane_state->crocs[i]);

                        //drift della rana se è sopra questo coccodrillo
                        frog_drift_if_on_croc(&frog,&frog_prev,&lane_state->crocs[i]);

                        //controllo caduta in acqua anche qui (nel caso in cui il coccodrillo abbia portato la rana fuori dallo schermo)
                        frog_water_check(&frog, &frog_prev,lanes_state,lane_y,&lives,frog_start_x,frog_start_y);
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
                //granata SINISTRA -> slot 0
                pid_t g0 = fork();
                if (g0 < 0) {
                    perror("fork grenade left");
                    exit(EXIT_FAILURE);
                }
                if (g0 == 0) {
                    close(fd_read);
                    grenade_process(fd_write, frog.x, frog.y, -1);
                } else {
                    gren_active[0] = true;
                    gren_pid[0] = g0;
                    gren_prev[0] = msg.entity;
                    grenades[0] = msg.entity;
                }

                //granata DESTRA -> slot 1
                pid_t g1 = fork();
                if (g1 < 0) {
                    perror("fork grenade right");
                    exit(EXIT_FAILURE);
                }
                if (g1 == 0) {
                    close(fd_read);
                    grenade_process(fd_write, frog.x, frog.y, +1);
                } else {
                    gren_active[1] = true;
                    gren_pid[1] = g1;
                    gren_prev[1] = msg.entity;
                    grenades[1] = msg.entity;
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
                dir = msg.entity.dx;

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

void frog_move(Entity *frog, Entity *frog_prev, int dx, int dy) {
    clear_entity(frog_prev);

    frog->x += dx;
    frog->y += dy;

    //limiti schermo
    if (frog->x < 0) 
        frog->x = 0;
    if (frog->x + frog->width > MAP_WIDTH)
        frog->x = MAP_WIDTH - frog->width;
    if (frog->y < 0) 
        frog->y = 0;
    if (frog->y + frog->height > MAP_HEIGHT)
        frog->y = MAP_HEIGHT - frog->height;

    *frog_prev = *frog;
}

void frog_water_check(Entity *frog, Entity *frog_prev, CrocLaneState lanes_state[], int lane_y[], int *lives, int frog_start_x, int frog_start_y) {
    bool in_river = false;
    bool on_croc  = false;
    int  which_lane = -1;

    //verifico se la rana si trova sulla Y di una corsia
    for (int l = 0; l < NUM_RIVER_LANES; l++) {
        if (frog->y == lane_y[l]) {
            in_river   = true;
            which_lane = l;
            break;
        }
    }
    if (!in_river) return;//se non è in nessuna corsia, nulla da fare

    //controllo se "on_croc" in quella corsia
    CrocLaneState *lane_state = &lanes_state[which_lane];
    for (int i = 0; i < MAX_CROCS_PER_LANE; i++) {
        if (!lane_state->active[i]) continue;
        Entity *croc = &lane_state->crocs[i];
        if (frog->x >= croc->x && frog->x < croc->x + croc->width) {
            on_croc = true;
            break;
        }
    }

    //se in_river e non on_croc allora cade in acqua
    if (in_river && !on_croc) {
        (*lives)--;
        clear_entity(frog_prev);

        frog->x = frog_start_x;
        frog->y = frog_start_y;
        *frog_prev = *frog;
    }
}

void frog_drift_if_on_croc(Entity *frog, Entity *frog_prev, Entity *croc) {
    if (frog->y == croc->y && frog->x >= croc->x && frog->x <  croc->x + croc->width) {
        clear_entity(frog_prev);
        frog->x += croc->dx;
        if (frog->x < 0) 
            frog->x = 0;
        if (frog->x + frog->width > MAP_WIDTH)
            frog->x = MAP_WIDTH - frog->width;
        *frog_prev = *frog;
    }
}