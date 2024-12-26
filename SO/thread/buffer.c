#include "buffer.h"
#include "game.h"
#include <stdio.h>
#include <stdlib.h>

// Inizializza il buffer circolare
void buffer_init(CircularBuffer *buffer) {
    buffer->head = 0;
    buffer->tail = 0;
    buffer->is_full = false;
    buffer->is_empty = true;
    pthread_mutex_init(&buffer->mutex, NULL);
}

// Inserisce un valore nel buffer (produttore)
void buffer_push(CircularBuffer *buffer, GameEvent event) {
    if (!buffer->is_full) { // Inserisci solo se il buffer non è pieno
        buffer->events[buffer->head] = event;
        buffer->head = (buffer->head + 1) % BUFFER_SIZE;
        buffer->is_empty = false;
        if (buffer->head == buffer->tail) {
            buffer->is_full = true;
        }
    }
}

// Estrae un valore dal buffer (consumatore)
GameEvent buffer_pop(CircularBuffer *buffer) {

    GameEvent event = {0, 0, 0}; // Evento di default
    if (!buffer->is_empty) {
        event = buffer->events[buffer->tail];
        buffer->tail = (buffer->tail + 1) % BUFFER_SIZE;

        buffer->is_full = false;
        if (buffer->head == buffer->tail) {
            buffer->is_empty = true;
        }
    }
    return event;
}
