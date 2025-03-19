#include "consumer.h"
#include "crocodile.h"
#include "buffer.h"
#include "game.h"
#include "frog.h"
#include "map.h"
#include "grenade.h"

// Helper per forzare le entità all'interno della mappa
void clamp_entity(Entity *entity) {
    if (entity->x < 1) entity->x = 1;
    if (entity->y < 1) entity->y = 1;
    if (entity->x + entity->width > MAP_WIDTH - 1)
        entity->x = MAP_WIDTH - 1 - entity->width;
    if (entity->y + entity->height > MAP_HEIGHT - 1)
        entity->y = MAP_HEIGHT - 1 - entity->height;
}

void *consumer_thread(void *arg) {
    ConsumerArgs *args = (ConsumerArgs *)arg;
    CircularBuffer *buffer = args->buffer;
    WINDOW *game_win = args->game_win;
    WINDOW *info_win = args->info_win;

    Entity frog;
    frog_init(&frog);
    Entity grenade_left, grenade_right;

    RiverLane lanes[NUM_RIVER_LANES];
    init_lanes(lanes);
    
    //Ricicliamo i buffer circolari per gestire spawn e despawn dei singoli coccodrilli nelle corsie
    CircularBuffer *lane_queue[NUM_RIVER_LANES];
    for (int i = 0; i < NUM_RIVER_LANES; i++)
    {
        lane_queue[i]=malloc(sizeof(CircularBuffer));
        buffer_init(lane_queue[i]);
    }
    
    
    Message msg;
    
    int lives = NUM_LIVES;
    int time = ROUND_TIME;
    int hole_index = -1;
    int holes_reached = 0;

    int lane_idx;
    int croc_idx;

    while (lives > 0) {
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

        msg = buffer_pop(buffer);
        switch (msg.type) {
            case MSG_FROG_UPDATE:
                clear_frog(game_win,&frog);
                frog = msg.entity;
                // Se la rana raggiunge la riga delle tane, controlla se è in una tana libera
                if (frog.y == HOLE_Y) {
                    hole_index = check_hole_reached(&frog);
                    if (hole_index != -1) {
                        hole_update(game_win,hole_index);
                        holes_reached++;
                        update_score(time * 100);
                        time = ROUND_TIME;
                        reset_round();
                        frog.x = (MAP_WIDTH - FROG_WIDTH ) / 2 ;
                        frog.y = MAP_HEIGHT - FROG_HEIGHT - 1;
                    }
                }
                draw_frog(game_win, &frog);
                break;
            case MSG_TIMER_TICK:
                time--;
                if (time <= 0) {
                    lives--;
                    clear_frog(game_win,&frog);
                    time = ROUND_TIME;
                    reset_round();
                }
                break;
            case MSG_GRENADE_LEFT:
                clear_grenade(game_win,&grenade_left);
                grenade_left = msg.entity;
                draw_grenade(game_win,&grenade_left);
                break;
            case MSG_GRENADE_RIGHT:
                clear_grenade(game_win,&grenade_right);
                grenade_right = msg.entity;
                draw_grenade(game_win,&grenade_right);
                break;


            case MSG_CROC_UPDATE:
                // Find the crocodile in the lane queue (using a more reliable method)
                int found = 0;
                int buff_idx = lane_queue[msg.id]->out;
                for (int i = 0; i < lane_queue[msg.id]->count; i++) {
                    // Using a more forgiving position match
                    if (abs(msg.entity.x - lane_queue[msg.id]->buffer[buff_idx].entity.x) <= 1) {
                        clear_crocodile(game_win, &lane_queue[msg.id]->buffer[buff_idx].entity);
                        lane_queue[msg.id]->buffer[buff_idx] = msg;
                        draw_crocodile(game_win, &lane_queue[msg.id]->buffer[buff_idx].entity);
                        found = 1;
                        break;
                    }
                    buff_idx = (buff_idx + 1) % BUFFER_SIZE;
                }
                // If not found, it might be a new crocodile, so add it
                if (!found && lane_queue[msg.id]->count < BUFFER_SIZE) {
                    buffer_push(lane_queue[msg.id], msg);
                    draw_crocodile(game_win, &msg.entity);
                }
                break;
            case MSG_CROC_DESPAWN:
                // Remove the crocodile from the lane queue
                if (lane_queue[msg.id]->count > 0) {
                    Message removed_croc = buffer_pop(lane_queue[msg.id]);
                    clear_crocodile(game_win, &removed_croc.entity);
                }
                break;
                
        }
        if (lives <= 0 || holes_reached == NUM_HOLES) {
            if (holes_reached == NUM_HOLES){
                pthread_mutex_lock(&game_state_mutex);
                game_state = GAME_WIN;
                pthread_mutex_unlock(&game_state_mutex);
            }
            break;
        }
        pthread_mutex_lock(&render_mutex);
        mvwprintw(info_win, 1, 1, "Lives: %-25d Score: %-25d Time:%4d", lives, score, time);
        wrefresh(info_win);
        pthread_mutex_unlock(&render_mutex);
    }
    werase(info_win);
    wrefresh(info_win);
    werase(game_win);
    wrefresh(game_win);
    return NULL;
}

void reset_round() {
    pthread_mutex_lock(&reset_mutex);
    round_reset_flag = 1;
    pthread_mutex_unlock(&reset_mutex);
}

void update_score(int points) {
    pthread_mutex_lock(&render_mutex);
    score += points;
    pthread_mutex_unlock(&render_mutex);
}