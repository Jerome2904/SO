#ifndef BUFFER_H
#define BUFFER_H

#include <pthread.h>
#include <stdbool.h>

#define BUFFER_SIZE 16 // Dimensione del buffer circolare

typedef struct{
    int type;   // Tipo di evento (es. movimento rana, proiettile, ecc.)
    int x;      // Coordinata x
    int y;      // Coordinata y
}
GameEvent;

typedef struct {
    GameEvent events[BUFFER_SIZE]; // Array di eventi
    int head;                  // Indice di testa
    int tail;                  // Indice di coda
    bool is_empty;             // Booleano buffer vuoto
    bool is_full;              // Booleano buffer pieno
    pthread_mutex_t mutex;     // Mutex per sincronizzare l'accesso
} CircularBuffer;


extern CircularBuffer buffer; 
// Funzioni per gestire il buffer
void buffer_init(CircularBuffer *buffer);
void buffer_push(CircularBuffer *buffer, GameEvent event);
GameEvent buffer_pop(CircularBuffer *buffer);

#endif
