#include "buffer.h"
#include "game.h"
#include <stdio.h>
#include <stdlib.h>

// Inizializza il buffer circolare
void buffer_init(CircularBuffer *buffer) {
    buffer->head = 0;
    buffer->tail = 0;
    sem_init(&buffer->empty, 0, BUFFER_SIZE); //BUFFER_SIZE spazi inizialmente liberi
    sem_init(&buffer->full, 0, 0); //0 spazi inizialmente occupati
    pthread_mutex_init(&buffer->mutex, NULL);
}

void buffer_destroy(CircularBuffer *buffer) {
    sem_destroy(&buffer->empty);
    sem_destroy(&buffer->full);
    pthread_mutex_destroy(&buffer->mutex);
} 
// Inserisce un valore nel buffer (produttore)
void buffer_push(CircularBuffer *buffer, GameEvent event) {
    sem_wait(&buffer->empty); //aspetta che ci sia spazio libero
    pthread_mutex_lock(&buffer->mutex); //blocca l'accesso al buffer
    //Produce un evento
    buffer->events[buffer->head] = event;
    buffer->head = (buffer->head + 1) % BUFFER_SIZE;
    pthread_mutex_unlock(&buffer->mutex); //sblocca l'accesso al buffer
    sem_post(&buffer->full);//segnala che c'e un elemento in più
}

// Estare un valore dal buffer (consumatore)
GameEvent buffer_pop(CircularBuffer *buffer) {
    GameEvent event = {0, 0, 0}; // Evento di default
    sem_wait(&buffer->full); //aspetta che ci sia almeno un elemento
    pthread_mutex_lock(&buffer->mutex); //blocca l'accesso al buffer
    //Consuma un evento
    event = buffer->events[buffer->tail];
    buffer->tail = (buffer->tail + 1) % BUFFER_SIZE;
    pthread_mutex_unlock(&buffer->mutex); //sblocca l'accesso al buffer
    sem_post(&buffer->empty); //segnala che c'e un elemento in meno
    return event;
}
