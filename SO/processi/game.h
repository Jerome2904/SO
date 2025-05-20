#ifndef GAME_H
#define GAME_H

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <stdbool.h>
#include <signal.h>


//Dimensione dell'area di gioco
#define MAP_HEIGHT 31
#define MAP_WIDTH 77
#define INFO_HEIGHT 3

#define NUM_OPTIONS 3

//Dimensioni e salti della rana
#define FROG_WIDTH 3
#define FROG_HEIGHT 2
#define HORIZONTAL_JUMP 1
#define VERTICAL_JUMP   FROG_HEIGHT

//Dimensioni Corsie
#define NUM_RIVER_LANES 10
#define CROCODILE_WIDTH 9
#define CROCODILE_HEIGHT 2

// Parametri di gioco
#define NUM_LIVES 3
#define INITIAL_SCORE 0
#define ROUND_TIME 50
#define NUM_HOLES 5

// Dimensioni massime della matrice sprite
#define SPRITE_ROWS 2
#define SPRITE_COLS 16

//Tipi di entità
typedef enum {
    ENTITY_FROG,
    ENTITY_GRENADE,
    ENTITY_CROCODILE,
    ENTITY_PROJECTILE,
} EntityType;

// Struttura per le entità
typedef struct {
    EntityType type;
    int x;
    int y;
    int width;
    int height;
    int dx;
    int dy;
    int speed;
    char sprite[SPRITE_ROWS][SPRITE_COLS];
} Entity;

typedef struct {
    int y;
    int direction;
    int speed;
    int index;
} RiverLane;

//Tipi di messaggi
typedef enum {
    MSG_TIMER_TICK,
    MSG_FROG_UPDATE,
    MSG_GRENADE_SPAWN, MSG_GRENADE_UPDATE, MSG_GRENADE_DESPAWN,
    MSG_CROC_SPAWN, MSG_CROC_UPDATE, MSG_CROC_DESPAWN,
    MSG_PROJECTILE_SPAWN, MSG_PROJECTILE_UPDATE, MSG_PROJECTILE_DESPAWN,
} MessageType;

//Struttura del messaggio inviato
typedef struct {
    MessageType type;
    int lane_id;
    int id;
    Entity entity;
} Message;

//Stato attuale del gioco
typedef enum {
    GAME_RUNNING,
    GAME_PAUSED,
    GAME_QUITTING,
    GAME_WIN
} GameState;


extern int game_state;  
extern int score;       
extern int round_reset_flag;  
extern pid_t spawner_pids[NUM_RIVER_LANES];

void show_instructions();
void exit_program();
void start_game();
void game_state_win();
void game_over();
void restart_game();
void draw_entity(Entity *entity);
void clear_entity(Entity *entity);

#endif