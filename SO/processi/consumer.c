#include "game.h"
#include "consumer.h"
#include "frog.h"
#include "map.h"
#include "timer.h"
#include "crocodile.h"
#include "grenade.h"
#include "spawner.h"


void consumer(int fd_read,int fd_write,WINDOW *info_win,pid_t spawner_pids[],int n_spawners,RiverLane lanes[],pid_t frog_pid,pid_t timer_pid){
    init_bckmap();
    init_holes_positions();
    init_map_holes();
    draw_map();

    Message msg;
    Entity frog;

    int time = ROUND_TIME;
    int lives = NUM_LIVES;
    int hole_index = -1;
    int holes_reached = 0;
    
    // stato corsie (coccodrilli)
    CrocLaneState lanes_state[NUM_RIVER_LANES] = {0};
    CrocLaneState *lane_state = NULL; //serve solo a rendere più leggibile e compatto il codice

    //coordinate Y delle corsie
    int lane_y[NUM_RIVER_LANES];
    lane_y[0] = MAP_HEIGHT - (BOTTOM_SIDEWALK+FROG_HEIGHT);
    for (int l = 1; l < NUM_RIVER_LANES; l++) {
        lane_y[l] = lane_y[l-1] - FROG_HEIGHT;
    }

    // stato granate
    Entity grenades[MAX_GRENADES] = {0};
    Entity gren_prev[MAX_GRENADES] = {0};
    pid_t gren_pid[MAX_GRENADES] = {0};
    bool gren_active[MAX_GRENADES] = {false};
    int active_grenades = 0;

    // stato proiettili
    Entity projectiles[MAX_PROJECTILES] = {0};
    Entity proj_prev[MAX_PROJECTILES] = {0};
    pid_t proj_pid[MAX_PROJECTILES] = {0};
    bool proj_active[MAX_PROJECTILES] = {false};
    int proj_count = 0;
    
    int frog_start_x = (MAP_WIDTH - FROG_WIDTH) / 2;
    int frog_start_y = MAP_HEIGHT - FROG_HEIGHT;

    frog_init(&frog);
    Entity frog_prev = frog;

    //variabili per i msg dei croc
    int lane =-1;
    pid_t id =-1; //usato anche per granate e proiettili
    
    //direzione della granata/proiettile
    int dir = 0;

    bool paused = false;

    while (lives > 0 && game_state == GAME_RUNNING) {
        if (read(fd_read, &msg, sizeof(msg)) <= 0) break;
        //toggle pausa
        if (msg.type == MSG_PAUSE) {
            paused = !paused;
            if (paused) {
                //metto in pausa tutti i produttori tranne la rana
                pause_producers(timer_pid, spawner_pids, n_spawners,gren_pid, gren_active,proj_pid, proj_active);
                //mostro menu pausa
                mvprintw(LINES/2-1,(COLS-6)/2,"PAUSA");
                mvprintw(LINES/2,  (COLS-23)/2,"Premi P per riprendere");
                refresh();
            } else {                         
                //riprende tutti i produttori
                resume_producers(timer_pid, spawner_pids, n_spawners,gren_pid, gren_active,proj_pid, proj_active);
                //ridisegno
                clear();
                draw_map();
                werase(info_win);
                wrefresh(info_win);
            }
        }
        if (paused) {
            //scarto tutti i messaggi mentre sono in pausa
            continue;
        }

        switch (msg.type) {
            case MSG_TIMER_TICK:
                time--;
                if (time <= 0) {
                    lives--;
                    time = ROUND_TIME;
                    restart_round(&frog,&frog_prev,frog_start_x,frog_start_y,lanes_state,lanes,gren_active,proj_active,gren_prev,proj_prev);
                }
                break;

            case MSG_FROG_UPDATE:
                //sposto rana
                frog_move(&frog, &frog_prev,msg.entity.dx,msg.entity.dy);
                
                //controllo caduta in acqua
                bool fell_in_water = frog_water_check(&frog, &frog_prev, lanes_state,lane_y,&lives,frog_start_x,frog_start_y);
                if (fell_in_water) {
                    lives--;
                    //killo tutte le entità
                    kill_all_entities(spawner_pids, NUM_RIVER_LANES,lanes_state,gren_pid, gren_active,proj_pid, proj_active);
                    //resetto gli stati
                    restart_round(&frog, &frog_prev,frog_start_x, frog_start_y,lanes_state, lanes,gren_active, proj_active,grenades, projectiles);
                    time = ROUND_TIME;
                    //ricreo i nuovi spawner
                    create_spawners(fd_write, fd_read, lanes, spawner_pids, NUM_RIVER_LANES);
                    continue;
                }
                hole_index = check_hole_reached(&frog);
                if (hole_index >= 0) {
                    hole_update(hole_index);
                    holes_reached++;

                    if (checkHoles()) {
                        game_state = GAME_WIN;
                        return;
                    }
                    score += time * 100;
                    //killo tutte le entità
                    kill_all_entities(spawner_pids, NUM_RIVER_LANES,lanes_state,gren_pid, gren_active,proj_pid, proj_active);
                    //resetto gli stati
                    restart_round(&frog, &frog_prev,frog_start_x, frog_start_y,lanes_state, lanes,gren_active, proj_active,grenades, projectiles);
                    time = ROUND_TIME;
                    //ricreo i nuovi spawner
                    create_spawners(fd_write, fd_read, lanes, spawner_pids, NUM_RIVER_LANES);
                    continue;
                }
                else if (frog.y == HOLE_Y && hole_index == -1) {
                    lives--;
                    //killo tutte le entità
                    kill_all_entities(spawner_pids, NUM_RIVER_LANES,lanes_state,gren_pid, gren_active,proj_pid, proj_active);
                    //resetto gli stati
                    restart_round(&frog, &frog_prev,frog_start_x, frog_start_y,lanes_state, lanes,gren_active, proj_active,grenades, projectiles);
                    time = ROUND_TIME;
                    //ricreo i nuovi spawner
                    create_spawners(fd_write, fd_read, lanes, spawner_pids, NUM_RIVER_LANES);
                }
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
                        frog_drift_on_croc(&frog,&frog_prev,&lane_state->crocs[i]);

                        //controllo caduta in acqua anche qui (nel caso in cui il coccodrillo abbia portato la rana fuori dallo schermo)
                        bool fell_in_water = frog_water_check(&frog, &frog_prev, lanes_state,lane_y,&lives,frog_start_x,frog_start_y);
                        if (fell_in_water) {
                            lives--;
                            //killo tutte le entità
                            kill_all_entities(spawner_pids, NUM_RIVER_LANES,lanes_state,gren_pid, gren_active,proj_pid, proj_active);
                            //resetto gli stati
                            restart_round(&frog, &frog_prev,frog_start_x, frog_start_y,lanes_state, lanes,gren_active, proj_active,grenades, projectiles);
                            time = ROUND_TIME;
                            //ricreo i nuovi spawner
                            create_spawners(fd_write, fd_read, lanes, spawner_pids, NUM_RIVER_LANES);
                            continue;
                        }
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
                        //controllo se la rana viene colpita
                        if (check_projectile_hits_frog(&projectiles[i], &frog)) {
                            lives--;
                            //killo tutte le entità
                            kill_all_entities(spawner_pids, NUM_RIVER_LANES,lanes_state,gren_pid, gren_active,proj_pid, proj_active);
                            //resetto gli stati
                            restart_round(&frog, &frog_prev,frog_start_x, frog_start_y,lanes_state, lanes,gren_active, proj_active,grenades, projectiles);
                            time = ROUND_TIME;
                            //ricreo i nuovi spawner
                            create_spawners(fd_write, fd_read, lanes, spawner_pids, NUM_RIVER_LANES);
                            continue;
                        }
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
        
        check_grenade_projectile_collisions(grenades, gren_prev, gren_active, gren_pid,projectiles, proj_prev, proj_active, proj_pid);
        clean();
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

bool frog_water_check(Entity *frog, Entity *frog_prev, CrocLaneState lanes_state[], int lane_y[], int *lives, int frog_start_x, int frog_start_y) {
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
    if (!in_river) return false;//se non è in nessuna corsia, nulla da fare

    //controllo se "on_croc" in quella corsia
    CrocLaneState *lane_state = &lanes_state[which_lane];
    for (int i = 0; i < MAX_CROCS_PER_LANE; i++) {
        if (!lane_state->active[i]) continue;
        Entity *croc = &lane_state->crocs[i];
        // coordinate orizzontali di frog e croc
        int frog_left  = frog->x;
        int frog_right = frog->x + frog->width;
        int croc_left  = croc->x;
        int croc_right = croc->x + croc->width;

        //controllo se la rana è sopra il coccodrillo
        if (frog_right > croc_left && frog_left < croc_right) {
            on_croc = true;
            break;
        }
    }

    //se in_river e NON on_croc allora cade in acqua
    if (in_river && !on_croc) return true;

    return false;
}

void frog_drift_on_croc(Entity *frog, Entity *frog_prev, Entity *croc) {
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

void check_grenade_projectile_collisions(Entity grenades[], Entity gren_prev[], bool gren_active[], pid_t gren_pid[],Entity projectiles[], Entity proj_prev[], bool proj_active[], pid_t proj_pid[]){
    for (int i = 0; i < MAX_GRENADES; i++) {
        if (!gren_active[i]) continue;
        for (int j = 0; j < MAX_PROJECTILES; j++) {
            if (!proj_active[j]) continue;

            if (grenades[i].x == projectiles[j].x && grenades[i].y == projectiles[j].y){
                //cancello da schermo
                clear_grenade(&gren_prev[i]);
                clear_projectile(&proj_prev[j]);
                //disattivo
                gren_active[i] = false;
                proj_active[j] = false;
            }
        }
    }
}

bool check_projectile_hits_frog(Entity *p, Entity *f) {
    return (p->x >= f->x && p->x < f->x + f->width &&
            p->y >= f->y && p->y < f->y + f->height);
}

void clean() {
    pid_t w;
    //ripulisci qualsiasi figlio terminato/zombie
    while ((w = waitpid(-1, NULL, WNOHANG)) > 0) {
        // ho ripulito un figlio terminato/zombie 
    }
}

void reset_crocs_state(CrocLaneState lanes_state[]) {
    for (int l = 0; l < NUM_RIVER_LANES; l++) {
        for (int i = 0; i < MAX_CROCS_PER_LANE; i++) {
            clear_entity(&lanes_state[l].prev[i]);
            lanes_state[l].active[i] = false;
            lanes_state[l].pid[i] = 0;
        }
            
    }
}

void reset_grenades_state(bool gren_active[],Entity gren_prev[],int max_gren) {
    for (int i = 0; i < max_gren; i++) {
        clear_grenade(&gren_prev[i]);
        gren_active[i] = false;
    } 
}

void reset_projectiles_state(bool proj_active[],Entity proj_prev[],int max_proj) {
    for (int i = 0; i < max_proj; i++) {
        clear_projectile(&proj_prev[i]);
        proj_active[i] = false;
    }
}

void reset_frog_position(Entity *frog,Entity *frog_prev,int frog_start_x,int frog_start_y) {
    clear_entity(frog_prev);
    frog->x = frog_start_x;
    frog->y = frog_start_y;
    *frog_prev = *frog;
}

void restart_round(Entity *frog,Entity *frog_prev,int frog_start_x,int frog_start_y,CrocLaneState lanes_state[],RiverLane lanes[], bool gren_active[], bool proj_active[],Entity grenades[],Entity projectiles[]) {
    reset_frog_position(frog,frog_prev,frog_start_x,frog_start_y);
    reset_crocs_state(lanes_state);
    reset_grenades_state(gren_active,grenades,MAX_GRENADES);
    reset_projectiles_state(proj_active,projectiles,MAX_PROJECTILES);
    init_lanes(lanes);
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

void kill_all_crocs(CrocLaneState lanes_state[]) {
    for (int l = 0; l < NUM_RIVER_LANES; l++) {
        for (int i = 0; i < MAX_CROCS_PER_LANE; i++) {
            pid_t pid = lanes_state[l].pid[i];
            if (lanes_state[l].active[i] && pid > 0) {
                kill(pid, SIGKILL);
                waitpid(pid, NULL, 0);
            }
        }
    }
}

void kill_all_grenades(pid_t gren_pid[], bool gren_active[]) {
    for (int i = 0; i < MAX_GRENADES; i++) {
        if (gren_active[i] && gren_pid[i] > 0) {
            kill(gren_pid[i], SIGKILL);
            waitpid(gren_pid[i], NULL, 0);
        }
    }
}

void kill_all_projectiles(pid_t proj_pid[], bool proj_active[]) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (proj_active[i] && proj_pid[i] > 0) {
            kill(proj_pid[i], SIGKILL);
            waitpid(proj_pid[i], NULL, 0);
        }
    }
}

void kill_all_entities(pid_t spawner_pids[],int n_spawners,CrocLaneState lanes_state[],pid_t gren_pid[], bool gren_active[],pid_t proj_pid[], bool proj_active[]){
    kill_all_spawners(spawner_pids, n_spawners);
    kill_all_crocs(lanes_state);
    kill_all_grenades(gren_pid, gren_active);
    kill_all_projectiles(proj_pid, proj_active);
}

void pause_producers(pid_t timer_pid,pid_t spawner_pids[], int n_spawners,pid_t gren_pid[], bool gren_active[],pid_t proj_pid[], bool proj_active[]) {
    //metto in pausa timer
    kill(timer_pid, SIGSTOP);
    //metto in pausa ogni spawner (e il suo process-group di crocs)
    stop_all_spawners(spawner_pids, n_spawners);
    //metto in pausa granate
    stop_all_grenades(gren_pid, gren_active);
    //metto in pausa proiettili
    stop_all_projectiles(proj_pid, proj_active);
}

void resume_producers(pid_t timer_pid,pid_t spawner_pids[], int n_spawners,pid_t gren_pid[], bool gren_active[],pid_t proj_pid[], bool proj_active[]) {
    //riprendo timer
    kill(timer_pid, SIGCONT);
    //riprendo spawner (+crocs)
    resume_all_spawners(spawner_pids, n_spawners);
    //riprendo granate
    resume_all_grenades(gren_pid, gren_active);
    //riprendo proiettili
    resume_all_projectiles(proj_pid, proj_active);
}

void stop_all_spawners(pid_t spawner_pids[], int n) {
    for (int i = 0; i < n; i++) {
        if (spawner_pids[i] > 0)
            kill(-spawner_pids[i], SIGSTOP);
    }
}

void stop_all_grenades(pid_t gren_pid[], bool gren_active[]) {
    for (int i = 0; i < MAX_GRENADES; i++) {
        if (gren_active[i] && gren_pid[i] > 0) {
            kill(gren_pid[i], SIGSTOP);
        }
    }
}

void stop_all_projectiles(pid_t proj_pid[], bool proj_active[]) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (proj_active[i] && proj_pid[i] > 0) {
            kill(proj_pid[i], SIGSTOP);
        }
    }
}

void resume_all_spawners(pid_t spawner_pids[], int n) {
    for (int i = 0; i < n; i++) {
        if (spawner_pids[i] > 0)
            kill(-spawner_pids[i], SIGCONT);
    }
}
void resume_all_grenades(pid_t gren_pid[], bool gren_active[]) {
    for (int i = 0; i < MAX_GRENADES; i++) {
        if (gren_active[i] && gren_pid[i] > 0) {
            kill(gren_pid[i], SIGCONT);
        }
    }
}
void resume_all_projectiles(pid_t proj_pid[], bool proj_active[]) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (proj_active[i] && proj_pid[i] > 0) {
            kill(proj_pid[i], SIGCONT);
        }
    }
}