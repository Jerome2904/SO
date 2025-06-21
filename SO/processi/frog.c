#include "frog.h"
#include "game.h"
#include "consumer.h"

void frog_process(int fd_write) {
    nodelay(stdscr, FALSE);
    keypad(stdscr, TRUE);
    Message msg;
    msg.type = MSG_FROG_UPDATE;
    while (1) {
        msg.entity.dx = 0;
        msg.entity.dy = 0;
        //leggo i tasti premuti dall'utente
        int ch = getch();

        switch (ch) {
            case 'w':
            case 'W':
            case KEY_UP:
                msg.entity.dy = -VERTICAL_JUMP; // salta in alto
                break;
            case 's':
            case 'S':
            case KEY_DOWN:
                msg.entity.dy = +VERTICAL_JUMP; // salta in basso
                break;
            case 'a':
            case 'A':
            case KEY_LEFT:
                msg.entity.dx = -HORIZONTAL_JUMP; // salta a sinistra
                break;
            case 'd':
            case 'D':
            case KEY_RIGHT:
                msg.entity.dx = +HORIZONTAL_JUMP; // salta a destra
                break;
            case ' ':
                { // lancia una granata
                    Message gmsg;
                    gmsg.type = MSG_GRENADE_SPAWN;
                    //invia il messaggio di spawn della granata
                    write(fd_write, &gmsg, sizeof(gmsg));
                }
                break;
            case 'p':
            case 'P':
                { // toggle pausa
                    Message pmsg;
                    pmsg.type = MSG_PAUSE;
                    //invia il messaggio di pausa
                    write(fd_write, &pmsg, sizeof(pmsg));
                }
                break;
            default:
                break;
        }
        //manda solo se ho mosso la rana
        if (msg.entity.dx != 0 || msg.entity.dy != 0) {
            write(fd_write, &msg, sizeof(msg));
        }
    }
}

void frog_init(Entity *frog) {
    // Inizializza le proprietà della rana
    frog->x = (MAP_WIDTH - FROG_WIDTH) / 2;
    frog->y = MAP_HEIGHT - FROG_HEIGHT;
    frog->width = FROG_WIDTH;
    frog->height = FROG_HEIGHT;
    frog->type = ENTITY_FROG;
    // Inizializza lo sprite della rana
    strcpy(frog->sprite[0], "vOv");
    strcpy(frog->sprite[1], "wUw");
}
