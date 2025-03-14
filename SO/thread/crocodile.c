#include "crocodile.h"
#include "map.h"
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

void init_lanes(RiverLane lanes[]) {
    
    // First lane direction is random
    lanes[0].direction = (rand() % 2 == 0) ? 1 : -1;
    lanes[0].speed = 120000;
    lanes[0].y = MAP_HEIGHT - 7;
    lanes[0].can_spawn = true;
    
    // Alternate directions for subsequent lanes
    for (int i = 1; i < NUM_RIVER_LANES; i++) {
        lanes[i].direction = -lanes[i-1].direction;
        lanes[i].speed = lanes[i-1].speed - 5000;
        lanes[i].y = lanes[i-1].y - FROG_HEIGHT;
        lanes[i].can_spawn = true;
    }
}

void crocodile_init(Entity *crocodile, RiverLane *lane,int offset) {
    
    crocodile->type = ENTITY_CROCODILE;
    crocodile->width = CROCODILE_WIDTH;
    crocodile->height = CROCODILE_HEIGHT;
    crocodile->y = lane->y;
    crocodile->dx = lane->direction;
    crocodile->speed = lane->speed;
    
    // Initial x position based on direction
    if (lane->direction > 0) {
        crocodile->x = 1 + (12 * offset);
    } else {
        crocodile->x = MAP_WIDTH - (17 * offset);
    }

    // Crocodile sprite
    char sprite[CROCODILE_HEIGHT][CROCODILE_WIDTH] = {
        {'<', 'R', 'R', 'R', 'R', '>'},
        {'<', 'R', 'R', 'R', 'R', '>'}
    };

    for (int i = 0; i < CROCODILE_HEIGHT; i++) {
        for (int j = 0; j < CROCODILE_WIDTH; j++) {
            crocodile->sprite[i][j] = sprite[i][j];
        }
    }
}

void *crocodile_thread(void *arg) {
    CrocodileArgs *args = (CrocodileArgs*) arg;
    CircularBuffer *buffer = args->buffer;
    RiverLane *lane = args->lane;
    int index = args->index;
    int offset = args->offset;

    Message msg;
    
    
    Entity crocodile;
    
    // Initialize crocodile
    crocodile_init(&crocodile,lane,offset);
    
    // Prepare message
    msg.type = MSG_CROC_UPDATE;
    msg.id.lane = index;
    msg.id.croc_index = offset;

    while (1) {
        // Check if game is paused
        pthread_mutex_lock(&game_state_mutex);
        if (game_state == GAME_PAUSED) {
            pthread_mutex_unlock(&game_state_mutex);
            usleep(100000);
            continue;
        }
        pthread_mutex_unlock(&game_state_mutex);
        
        // Check if game is quitting
        pthread_mutex_lock(&game_state_mutex);
        if (game_state == GAME_QUITTING || game_state == GAME_WIN) {
            pthread_mutex_unlock(&game_state_mutex);
            pthread_exit(NULL);
        }
        pthread_mutex_unlock(&game_state_mutex);
        
        // Sleep based on speed
        usleep(crocodile.speed);
        
        // Update x position based on lane direction
        crocodile.x += crocodile.dx;
        
        // Check for boundaries and wrap around
        if (crocodile.dx > 0 && crocodile.x > MAP_WIDTH) {
            crocodile.x = 1 - crocodile.width;
        } else if (crocodile.dx < 0 && crocodile.x < 1 - crocodile.width) {
            crocodile.x = MAP_WIDTH;
        }
        
        // Prepare and send message
        msg.entity = crocodile;
        buffer_push(buffer, msg);
    }
    
    free(args);
    return NULL;
}

void *crocodile_projectile_thread(void *arg) {
    // Implementation for projectile thread
    // Similar to grenade threads, but with specific river lane characteristics
    return NULL;
}

void draw_crocodile(WINDOW *win, Entity *crocodile) {
    for (int i = 0; i < CROCODILE_HEIGHT; i++) {
        for (int j = 0; j < CROCODILE_WIDTH; j++) {
            if(crocodile->y + i < MAP_HEIGHT && crocodile->y + i > 0 &&
                crocodile->x + j < MAP_WIDTH - 1 && crocodile->x + j > 0){
                    wattron(win, COLOR_PAIR(map[crocodile->y + i][crocodile->x + j]));
                    mvwaddch(win, crocodile->y + i, crocodile->x + j, crocodile->sprite[i][j]);
                    wattroff(win, COLOR_PAIR(map[crocodile->y + i][crocodile->x + j]));
                }
        }
    }
    
    pthread_mutex_lock(&render_mutex);
    wrefresh(win);
    pthread_mutex_unlock(&render_mutex);
}

void clear_crocodile(WINDOW *win, Entity *crocodile) {
    for (int i = 0; i < CROCODILE_HEIGHT; i++) {
        for (int j = 0; j < CROCODILE_WIDTH; j++) {
            if(crocodile->y + i < MAP_HEIGHT && crocodile->y + i > 0 &&
                crocodile->x + j < MAP_WIDTH - 1&& crocodile->x + j > 0){
                    wattron(win, COLOR_PAIR(map[crocodile->y + i][crocodile->x + j]));
                    mvwaddch(win, crocodile->y + i, crocodile->x + j, ' ');
                    wattroff(win, COLOR_PAIR(map[crocodile->y + i][crocodile->x + j]));
                }
        }
    }
    pthread_mutex_lock(&render_mutex);
    wrefresh(win);
    pthread_mutex_unlock(&render_mutex);
}