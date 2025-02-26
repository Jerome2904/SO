#include "timer.h"
#include "buffer.h"


void *timer_thread(void *arg) {
    TimerArgs *args = (TimerArgs *)arg;
    CircularBuffer *buffer = args->buffer;
    Message msg;

    while (1) {
        sleep(1);
        msg.type = MSG_TIMER_TICK;
        buffer_push(buffer, msg);
    }
    return NULL;
}