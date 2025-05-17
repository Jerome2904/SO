#include "game.h"
#include "crocodile.h"
#include "map.h"

// Inizializza le corsie
void init_lanes(RiverLane lanes[]) {
    lanes[0].direction = (rand() % 2 == 0 ? 1 : -1);
    lanes[0].speed = 200000;
    lanes[0].y = MAP_HEIGHT - (BOTTOM_SIDEWALK+FROG_HEIGHT);
    lanes[0].index = 0;
    lanes[0].movements_to_spawn = 0;

    for (int i = 1; i < NUM_RIVER_LANES; i++) {
        lanes[i].direction = -lanes[i-1].direction;
        lanes[i].speed = lanes[i-1].speed - 10000;
        lanes[i].y = lanes[i-1].y - FROG_HEIGHT;
        lanes[i].index = i;
    }
}

// Inizializza tutte le proprietà di un singolo coccodrillo
void crocodile_init(Entity *crocodile, RiverLane *lane) {
    crocodile->type = ENTITY_CROCODILE;
    crocodile->width = CROCODILE_WIDTH;
    crocodile->height = FROG_HEIGHT;
    crocodile->y = lane->y;
    crocodile->dx = lane->direction;
    crocodile->speed = lane->speed;
    crocodile->impacted = false;
    crocodile->is_badcroc = (rand() % 100) < 15;
    crocodile->cooldown = crocodile->is_badcroc ? 20 + (rand() % 20) : -1;

    strcpy(crocodile->sprite[0], "<BBBBBBB>");
    strcpy(crocodile->sprite[1], "<BBBBBBB>");
}

void crocodile_process(int fd_write, RiverLane lane) {
    pid_t my_pid = getpid();

    //sleep casuale tra 1 e 3 secondi
    int delay_ms = 1 + rand() % 3;
    usleep(delay_ms * 1000000);

    //inizializzo un nuovo coccodrillo
    Entity croc;
    crocodile_init(&croc, &lane);

    // spawn iniziale fuori schermo
    if (lane.direction > 0)
        croc.x = -croc.width;
    else
        croc.x = MAP_WIDTH;

    Message msg;

    //messaggio di spawn
    msg.type   = MSG_CROC_SPAWN;
    msg.lane_id = lane.index;
    msg.id = my_pid;
    msg.entity = croc;
    write(fd_write, &msg, sizeof(msg));

    bool has_shot = false;
    //manda posizione del coccodrillo finchè non esce dallo schermo
    msg.type = MSG_CROC_UPDATE;
    while ((croc.dx > 0 && croc.x < MAP_WIDTH) ||(croc.dx < 0 && croc.x + croc.width > 0)) {
        //1% di probabilità di sparare un proiettile
        if (!has_shot && (rand() % 100) < 1) {
            Message pmsg;
            pmsg.type    = MSG_PROJECTILE_SPAWN;
            pmsg.lane_id = lane.index;   
            pmsg.id = my_pid;
            //posizione di partenza del proiettile
            if (croc.dx > 0) {
                pmsg.entity.x = croc.x + croc.width;
            } else {
                pmsg.entity.x = croc.x - 1;
            }
            pmsg.entity.y = croc.y;
            pmsg.entity.dx = croc.dx;
            write(fd_write, &pmsg, sizeof(pmsg));
            //impedisco di sparare più volte
            has_shot = true;
        }
        msg.entity = croc;
        write(fd_write, &msg, sizeof(msg));

        croc.x += croc.dx;
        usleep(croc.speed);
    }

    //messaggio di despawn
    msg.type   = MSG_CROC_DESPAWN;
    msg.entity = croc;
    write(fd_write, &msg, sizeof(msg));


    exit(0);
}

void projectile_process(int fd_write,int start_x, int start_y, int dx) {
    Message msg;
    Entity projectile;
    projectile.type = ENTITY_PROJECTILE;
    projectile.width = 1;
    projectile.height = 1;
    projectile.x = start_x;
    projectile.y = start_y;
    projectile.dx = dx;
    projectile.speed = 40000;
    projectile.sprite[0][0] = '=';
    pid_t my_pid = getpid();

    //manda posizione della granata finchè non esce dallo schermo
    msg.type = MSG_PROJECTILE_UPDATE;
    while ((projectile.dx > 0 && projectile.x < MAP_WIDTH) ||(projectile.dx < 0 && projectile.x + projectile.width > 0)) {
        msg.type   = MSG_PROJECTILE_UPDATE;
        msg.entity = projectile;
        msg.id = my_pid;
        write(fd_write, &msg, sizeof(msg));

        projectile.x += projectile.dx;
        usleep(projectile.speed);
    }

    //messaggio di despawn
    msg.type   = MSG_PROJECTILE_DESPAWN;
    msg.entity = projectile;
    msg.id = my_pid;
    write(fd_write, &msg, sizeof(msg));


    exit(0);
}

void draw_projectile(Entity *projectile) {
    if (projectile->x >= 0 && projectile->x < MAP_WIDTH && projectile->y >= 0 && projectile->y < MAP_HEIGHT) {
        attron(COLOR_PAIR(map[projectile->y][projectile->x]));
        mvaddch(projectile->y, projectile->x, projectile->sprite[0][0]);
        attroff(COLOR_PAIR(map[projectile->y][projectile->x]));
    }
}

void clear_projectile(Entity *projectile) {
    if (projectile->x >= 0 && projectile->x < MAP_WIDTH && projectile->y >= 0 && projectile->y < MAP_HEIGHT) {
        attron(COLOR_PAIR(map[projectile->y][projectile->x]));
        mvaddch(projectile->y, projectile->x,' ');
        attroff(COLOR_PAIR(map[projectile->y][projectile->x]));
    }
}