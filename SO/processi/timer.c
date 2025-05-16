#include "timer.h"

void timer_process(int fd_write) {
    Message msg;
    while (1){
        msg.type = MSG_TIMER_TICK;
        write(fd_write, &msg, sizeof(msg));
        sleep(1);
    }
    exit(0);
}