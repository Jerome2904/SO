#include "game.h"
#include "spawner.h"
#include "crocodile.h" 
#include "consumer.h"

void create_spawners(int fd_write, int fd_read, RiverLane lanes[], pid_t spawner_pids[], int n_lanes) {
    for (int i = 0; i < n_lanes; i++) {
        pid_t sp = fork();
        if (sp < 0) { perror("fork spawner"); exit(EXIT_FAILURE); }
        spawner_pids[i] = sp;
        if (sp == 0) {
            //diventa leader del suo process‐group
            setpgid(0, 0);
            close(fd_read);
            spawner_process(fd_write, fd_read, lanes[i]);
        }
    }
}

void spawner_process(int fd_write,int fd_read, RiverLane lane) {
    bool first = true;
    pid_t self = getpid();
    srand(time(NULL) ^ self);

    while (1) {
        if (!first) {
            //delay casuale tra 5 e 7 secondi
            int delay_s = 5 + rand() % 3;
            interruptible_sleep(delay_s * 1000000);
        }
        first = false;

        pid_t c = fork();
        if (c < 0) {
            perror("fork crocodile");
            exit(1);
        }
        if (c == 0) {
            close(fd_read);
            crocodile_process(fd_write, lane);
            exit(0);
        }
        clean();
    }
}

void interruptible_sleep(int total_microseconds) {
    int chunk_size = 50000; //50ms 
    int remaining = total_microseconds;
    
    while (remaining > 0) {
        int sleep_time;
        if (remaining > chunk_size) {
            sleep_time = chunk_size;
        } else {
            sleep_time = remaining;
        }
        usleep(sleep_time);
        remaining -= sleep_time;
    }
}