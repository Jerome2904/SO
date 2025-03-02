#include "buffer.h"

// Inizializza il buffer circolare
void buffer_init(CircularBuffer *cb) {
    cb->in = 0;
    cb->out = 0;
    cb->count = 0;
    pthread_mutex_init(&cb->mutex, NULL);
    pthread_cond_init(&cb->empty,NULL);
    pthread_cond_init(&cb->full,NULL);
}

void buffer_destroy(CircularBuffer *cb) {
    pthread_mutex_destroy(&cb->mutex);
    pthread_cond_destroy(&cb->empty);
    pthread_cond_destroy(&cb->full);
}
// Inserisce un valore nel buffer 
void buffer_push(CircularBuffer *cb, Message msg) {
    pthread_mutex_lock(&cb->mutex); //blocca l'accesso al buffer

    while(cb->count == BUFFER_SIZE){
        pthread_cond_wait(&cb->empty,&cb->mutex);
    }
    // Produce un messaggio
    cb->buffer[cb->in] = msg;
    cb->in = (cb->in + 1) % BUFFER_SIZE; 
    cb->count++;

    pthread_cond_signal(&cb->full);
    pthread_mutex_unlock(&cb->mutex); //sblocca l'accesso al buffer
}
// Estare un valore dal buffer
Message buffer_pop(CircularBuffer *cb) {
    Message msg;
    pthread_mutex_lock(&cb->mutex); //blocca l'accesso al buffer
    while(cb->count==0){
        pthread_cond_wait(&cb->full,&cb->mutex);
    }
    // Consuma un messaggio
    msg = cb->buffer[cb->out];
    cb->out = (cb->out + 1) % BUFFER_SIZE;
    cb->count--;
    pthread_cond_signal(&cb->empty);
    pthread_mutex_unlock(&cb->mutex); //sblocca l'accesso al buffer
    return msg;
}
