#include "game.h"
#include "crocodile.h"
#include "map.h"

// Inizializza le corsie
void init_lanes(RiverLane lanes[]) {
    lanes[0].direction = (rand() % 2 == 0 ? 1 : -1);
    lanes[0].speed = 200000;
    lanes[0].y = MAP_HEIGHT - (BOTTOM_SIDEWALK+FROG_HEIGHT);
    lanes[0].index = 0;

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
    bool prefire_warning = false;
    int prefire_timer = 0; //microsecondi da aspettare per sparare
    int step = 50000; //50 ms per ogni ciclo
 
    msg.type = MSG_CROC_UPDATE;

    while ((croc.dx > 0 && croc.x < MAP_WIDTH) || (croc.dx < 0 && croc.x + croc.width > 0)) {
        //se non ha ancora sparato,valutiamo se far partire il warning
        if (!has_shot && !prefire_warning && (rand() % 100) < 1) {
            prefire_warning = true;
            prefire_timer = 400000;
            //cambia sprite per il warning
            if (croc.dx > 0) {
                strcpy(croc.sprite[0], "<BBBBBB//");
                strcpy(croc.sprite[1], "<BBBBBB\\\\");
            } else{
                strcpy(croc.sprite[0], "\\\\BBBBBB>");
                strcpy(croc.sprite[1], "//BBBBBB>");
            }
        }

        //se siamo in fase di pre-fire
        if (prefire_warning && prefire_timer > 0) {
            prefire_timer -= step;
        }

        //quando il timer è scaduto, spara
        if (prefire_warning && prefire_timer <= 0) {
            Message pmsg;
            pmsg.type    = MSG_PROJECTILE_SPAWN;
            pmsg.lane_id = lane.index;
            pmsg.id      = my_pid;

            if (croc.dx > 0)
                pmsg.entity.x = croc.x + croc.width;
            else
                pmsg.entity.x = croc.x - 1;

            pmsg.entity.y = croc.y;
            pmsg.entity.dx = croc.dx;
            write(fd_write, &pmsg, sizeof(pmsg));

            has_shot = true;
            prefire_warning = false;

            //ripristina sprite normale
            strcpy(croc.sprite[0], "<BBBBBBB>");
            strcpy(croc.sprite[1], "<BBBBBBB>");
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