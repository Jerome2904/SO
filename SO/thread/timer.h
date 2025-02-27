#ifndef TIMER_H
#define TIMER_H

#include "game.h"

typedef struct {
    CircularBuffer *buffer;
} TimerArgs;

void *timer_thread(void *arg);


#endif