#include "frog.h"
#include "game.h"
#include "consumer.h"

void frog_process(int fd_write) {
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    Message msg;
    msg.type = MSG_FROG_UPDATE;
    while (1) {
        int ch = getch();
        msg.entity.dx = 0;
        msg.entity.dy = 0;

        switch (ch) {
            case KEY_UP:
                msg.entity.dy = -VERTICAL_JUMP;
                break;
            case KEY_DOWN:
                msg.entity.dy = +VERTICAL_JUMP;
                break;
            case KEY_LEFT:
                msg.entity.dx = -HORIZONTAL_JUMP;
                break;
            case KEY_RIGHT:
                msg.entity.dx = +HORIZONTAL_JUMP;
                break;
            case ' ':
                {
                    Message gmsg;
                    gmsg.type = MSG_GRENADE_SPAWN;
                    write(fd_write, &gmsg, sizeof(gmsg));
                }
                break;
            case 'q':
                exit(0);
            default:
                break;
        }
        //manda solo se ho mosso la rana
        if (msg.entity.dx != 0 || msg.entity.dy != 0) {
            write(fd_write, &msg, sizeof(msg));
        }
        usleep(50000);
    }
}

void frog_init(Entity *frog) {
    frog->x = (MAP_WIDTH - FROG_WIDTH) / 2;
    frog->y = MAP_HEIGHT - FROG_HEIGHT;
    frog->width = FROG_WIDTH;
    frog->height = FROG_HEIGHT;
    frog->type = ENTITY_FROG;
    strcpy(frog->sprite[0], "vOv");
    strcpy(frog->sprite[1], "wUw");
}
