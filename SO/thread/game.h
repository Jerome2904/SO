#ifndef GAME_H
#define GAME_H

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>


//Dimensione dell'area di gioco
#define MAP_HEIGHT 40
#define MAP_WIDTH 77
#define INFO_HEIGHT 3

#define NUM_OPTIONS 3

//Dimensioni del buffer condiviso
#define BUFFER_SIZE 64

//Dimensioni e salti della rana
#define FROG_WIDTH 3
#define FROG_HEIGHT 2
#define HORIZONTAL_JUMP FROG_WIDTH
#define VERTICAL_JUMP   FROG_HEIGHT

//Dimensioni Corsie e Coccodrilli
#define NUM_RIVER_LANES 13
#define NUM_CROC 6
#define CROCODILE_WIDTH 10
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
    ENTITY_CROCODILEPROJ,
    // Altre entità possono essere aggiunte in futuro
} EntityType;

// Struttura per le entità
typedef struct {
    EntityType type;
    int x;
    int y;
    int width;
    int height;
    int dx; // direzione
    int speed;
    char sprite[SPRITE_ROWS][SPRITE_COLS];
    bool is_badcroc;
    bool impacted;//serve per quando i proiettili rana o coccodrilli fanno una collisione.
    int cooldown;
} Entity;

typedef struct{
    int y;
    int direction;
    int speed;
    int index;
    int movements;
    int movements_to_spawn;
}RiverLane;

//Tipi di messaggi
typedef enum {
    MSG_FROG_UPDATE,
    MSG_TIMER_TICK,
    MSG_GRENADE_LEFT,
    MSG_GRENADE_RIGHT,
    MSG_CROC_UPDATE,
    MSG_CROC_PROJECTILE,
    MSG_CROC_SPAWN,
    MSG_CROC_DESPAWN,
    MSG_GRENADE_SPAWN,
} MessageType;


//Struttura del messaggio inviato
typedef struct {
    MessageType type;
    int id; // 0 per la rana
    Entity entity;
} Message;



typedef struct {
    Message buffer[BUFFER_SIZE];
    int in, out,count; // in/out index e contatore
    sem_t empty;// quante celle vuote
    sem_t full; // quante celle piene
    pthread_mutex_t mutex;// per proteggere in/out
} CircularBuffer;


//Stato attuale del gioco
typedef enum {
    GAME_RUNNING,
    GAME_PAUSED,
    GAME_QUITTING,
    GAME_WIN
} GameState;

extern int round_reset_flag;
extern pthread_mutex_t reset_mutex;

extern int game_state;
extern pthread_mutex_t game_state_mutex;

extern int score;

void show_instructions(WINDOW *menu_win);
void exit_program(WINDOW *menu_win);
void start_game();
void game_state_win();
void game_over();
void restart_game();

#endif
