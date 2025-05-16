#include "frog.h"
#include "game.h"
#include "consumer.h"

void frog_process(int fd_write) {
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    Message msg;
    // inizializza posizione
    msg.entity.x = (MAP_WIDTH - FROG_WIDTH) / 2;
    msg.entity.y = MAP_HEIGHT - FROG_HEIGHT;
    msg.type = MSG_FROG_UPDATE;
    
    while (1) {
        int ch = getch();
        switch (ch) {
          case KEY_UP:    if (msg.entity.y >= VERTICAL_JUMP) msg.entity.y -= VERTICAL_JUMP; break;
          case KEY_DOWN:  if (msg.entity.y + VERTICAL_JUMP + FROG_HEIGHT <= MAP_HEIGHT) msg.entity.y += VERTICAL_JUMP; break;
          case KEY_LEFT:  if (msg.entity.x >= HORIZONTAL_JUMP) msg.entity.x -= HORIZONTAL_JUMP; break;
          case KEY_RIGHT: if (msg.entity.x + HORIZONTAL_JUMP + FROG_WIDTH <= MAP_WIDTH) msg.entity.x += HORIZONTAL_JUMP; break;
          case 'q': exit(0);
          default:   break;
        }
        write(fd_write, &msg, sizeof(msg));
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
