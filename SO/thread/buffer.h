#ifndef BUFFER_H
#define BUFFER_H

#include <pthread.h>
#include <semaphore.h>
#include "game.h"

void buffer_init(CircularBuffer *cb);
void buffer_destroy(CircularBuffer *cb);
void buffer_push(CircularBuffer *cb, Message msg);
Message buffer_pop(CircularBuffer *cb);

#endif