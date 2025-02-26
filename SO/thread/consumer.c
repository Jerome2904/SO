#include "consumer.h"
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
    Entity grenade_left, grenade_right, grenade_left_old, grenade_right_old;

    Message msg;
    
    int score = INITIAL_SCORE;
    int lives = NUM_LIVES;
    int time = ROUND_TIME;
    int hole_index = -1;
    int holes_reached = 0;

    while (lives > 0) {
        msg = buffer_pop(buffer);
        switch (msg.type) {
            case MSG_FROG_UPDATE:
                clear_frog(game_win,&frog);
                frog = msg.entity;
                clamp_entity(&frog);
                // Se la rana raggiunge la riga delle tane, controlla se è in una tana libera
                if (frog.y == HOLE_Y) {
                    hole_index = check_hole_reached(&frog);
                    if (hole_index != -1) {
                        hole_update(game_win,hole_index);
                        holes_reached++;
                        score += time * 100;
                        time = ROUND_TIME;
                        reset_round();
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
        }
        if (lives <= 0 || holes_reached == NUM_HOLES) {
            werase(game_win);
            box(game_win,0,0);
            if (holes_reached == NUM_HOLES)
                mvwprintw(game_win, MAP_HEIGHT / 2, (MAP_WIDTH - 22) / 2, "HAI VINTO! Score: %d", score);
            else
                mvwprintw(game_win, MAP_HEIGHT / 2, (MAP_WIDTH - 22) / 2, "HAI PERSO! Score: %d", score);
            wrefresh(game_win);
            sleep(2);
            break;
        }
        box(game_win,0,0);
        mvwprintw(info_win, 1, 1, "Lives: %-25d Score: %-25d Time:%4d", lives, score, time);
        wrefresh(info_win);
        usleep(1000);
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

