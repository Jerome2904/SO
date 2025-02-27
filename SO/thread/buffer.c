#include "buffer.h"

// Inizializza il buffer circolare
void buffer_init(CircularBuffer *cb) {
    cb->in = 0;
    cb->out = 0;
    pthread_mutex_init(&cb->mutex, NULL);
    sem_init(&cb->empty, 0, BUFFER_SIZE); //BUFFER_SIZE spazi inizialmente liberi
    sem_init(&cb->full, 0, 0); //0 spazi inizialmente occupati
}

void buffer_destroy(CircularBuffer *cb) {
    pthread_mutex_destroy(&cb->mutex);
    sem_destroy(&cb->empty);
    sem_destroy(&cb->full);
}
// Inserisce un valore nel buffer 
void buffer_push(CircularBuffer *cb, Message msg) {
    sem_wait(&cb->empty); //aspetta che ci sia spazio libero
    pthread_mutex_lock(&cb->mutex); //blocca l'accesso al buffer
    // Produce un messaggio
    cb->buffer[cb->in] = msg;
    cb->in = (cb->in + 1) % BUFFER_SIZE; 
    pthread_mutex_unlock(&cb->mutex); //sblocca l'accesso al buffer
    sem_post(&cb->full); //segnala che c'e un elemento in più
}
// Estare un valore dal buffer
Message buffer_pop(CircularBuffer *cb) {
    Message msg;
    sem_wait(&cb->full); //aspetta che ci sia almeno un elemento
    pthread_mutex_lock(&cb->mutex); //blocca l'accesso al buffer
    // Consuma un messaggio
    msg = cb->buffer[cb->out];
    cb->out = (cb->out + 1) % BUFFER_SIZE;
    pthread_mutex_unlock(&cb->mutex); //blocca l'accesso al buffer
    sem_post(&cb->empty); //segnala che c'e un elemento in meno
    return msg;
}
