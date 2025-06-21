#include "game.h"
#include "crocodile.h"
#include "map.h"

// Inizializza le corsie
void init_lanes(RiverLane lanes[]) {
    //Inizializza la prima corsia
    lanes[0].direction = (rand() % 2 == 0 ? 1 : -1);
    lanes[0].speed = 200000;
    lanes[0].y = MAP_HEIGHT - (BOTTOM_SIDEWALK+FROG_HEIGHT);
    lanes[0].index = 0;

    //Inizializza le altre corsie
    for (int i = 1; i < NUM_RIVER_LANES; i++) {
        //le corsie sono alternate, quindi la direzione e la velocità sono invertite rispetto alla corsia precedente
        lanes[i].direction = -lanes[i-1].direction; 
        lanes[i].speed = lanes[i-1].speed - 10000; //la velocità diminuisce di 10000 per ogni corsia
        lanes[i].y = lanes[i-1].y - FROG_HEIGHT; //la y diminuisce di FROG_HEIGHT per ogni corsia
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
    // se la difficoltà è facile, i crocs si aggiornano più lentamente (50ms in più)
    if (difficulty == EASY) crocodile->speed += 50000;
    // se la difficoltà è difficile, i crocs si aggiornano più velocemente
    else if (difficulty == HARD) crocodile->speed -= 50000;

    // inizializza lo sprite del coccodrillo
    strcpy(crocodile->sprite[0], "<BBBBBBB>");
    strcpy(crocodile->sprite[1], "<BBBBBBB>");
}

void crocodile_process(int fd_write, RiverLane lane) {
    pid_t my_pid = getpid(); // ottengo il pid del processo corrente

    

    //inizializzo un nuovo coccodrillo
    Entity croc;
    crocodile_init(&croc, &lane);
    
    // regola la probabilità di warning/sparo
    int shoot_chance;
    if (difficulty == EASY) shoot_chance = 1; //0.1%
    else if (difficulty == HARD) shoot_chance = 50; //5%
    else shoot_chance = 30; //3%

    //spawn iniziale
    if (lane.direction > 0)
        croc.x = 0; //spawn a sinistra
    else
        croc.x = MAP_WIDTH -croc.width; //spawn a destra

    Message msg;

    //messaggio di spawn
    msg.type = MSG_CROC_SPAWN;
    msg.lane_id = lane.index;
    msg.id = my_pid;
    msg.entity = croc;
    //scrivo il messaggio di spawn nella pipe
    write(fd_write, &msg, sizeof(msg));

    bool has_shot = false; //indica se il coccodrillo ha già sparato
    bool prefire_warning = false; //indica se il coccodrillo è in fase di pre-fire
    int prefire_timer = 0; //microsecondi da aspettare per sparare
    int step = 50000; //50 ms per ogni ciclo
    
    msg.type = MSG_CROC_UPDATE;
    //finché il coccodrillo è nello schermo
    while ((croc.dx > 0 && croc.x < MAP_WIDTH) || (croc.dx < 0 && croc.x + croc.width > 0)) {
        //se non ha ancora sparato,valutiamo se far partire il warning
        if (!has_shot && !prefire_warning && (rand() % 1000) < shoot_chance) {
            prefire_warning = true; //attiva il pre-fire warning
            prefire_timer = 400000; //400 ms prima di sparare
            //cambia sprite per il warning (in base alla direzione)
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
            prefire_timer -= step; //decrementa il timer di pre-fire
        }

        //quando il timer è scaduto, spara
        if (prefire_warning && prefire_timer <= 0) {
            //crea un nuovo messaggio per il proiettile
            Message pmsg;
            pmsg.type = MSG_PROJECTILE_SPAWN;
            pmsg.lane_id = lane.index;
            pmsg.id = my_pid;
            // se il coccodrillo sta andando a destra, il proiettile parte a destra del coccodrillo
            if (croc.dx > 0) {
                pmsg.entity.x = croc.x + croc.width + 1;
                // evita di sparare fuori dallo schermo
                if (pmsg.entity.x >= MAP_WIDTH) 
                    pmsg.entity.x = MAP_WIDTH - 1;
            }   
            // se il coccodrillo sta andando a sinistra, il proiettile parte a sinistra del coccodrillo
            else {
                pmsg.entity.x = croc.x - 2;
                // evita di sparare fuori dallo schermo
                if (pmsg.entity.x < 0) 
                    pmsg.entity.x = 0;
            }
                
            //posizione y del proiettile è la stessa del coccodrillo
            pmsg.entity.y = croc.y;
            pmsg.entity.dx = croc.dx;
            // scrivo il messaggio di spawn del proiettile nella pipe
            write(fd_write, &pmsg, sizeof(pmsg));

            has_shot = true; //indica che il coccodrillo ha già sparato
            prefire_warning = false; //disattiva il pre-fire warning

            //ripristina sprite normale
            strcpy(croc.sprite[0], "<BBBBBBB>");
            strcpy(croc.sprite[1], "<BBBBBBB>");
        }
        //scrivo il messaggio di aggiornamento del coccodrillo nella pipe
        msg.entity = croc;
        write(fd_write, &msg, sizeof(msg));
        //sposto il coccodrillo in base alla sua direzione
        croc.x += croc.dx;
        usleep(croc.speed);
    }

    //messaggio di despawn
    msg.type = MSG_CROC_DESPAWN;
    msg.entity = croc;
    write(fd_write, &msg, sizeof(msg));


    exit(0);
}

void projectile_process(int fd_write,int start_x, int start_y, int dx) {
    //inizializzo un nuovo proiettile
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
    pid_t my_pid = getpid(); // ottengo il pid del processo corrente

    //manda posizione della granata finchè non esce dallo schermo
    msg.type = MSG_PROJECTILE_UPDATE;
    while ((projectile.dx > 0 && projectile.x < MAP_WIDTH) ||(projectile.dx < 0 && projectile.x + projectile.width > 0)) {
        msg.type = MSG_PROJECTILE_UPDATE;
        msg.entity = projectile;
        msg.id = my_pid;
        write(fd_write, &msg, sizeof(msg));
        //sposto il proiettile in base alla sua direzione
        projectile.x += projectile.dx;
        usleep(projectile.speed);
    }

    //messaggio di despawn
    msg.type = MSG_PROJECTILE_DESPAWN;
    msg.entity = projectile;
    msg.id = my_pid;
    write(fd_write, &msg, sizeof(msg));


    exit(0);
}
//disegna un coccodrillo sullo schermo
void draw_crocodile(Entity *crocodile) {
    for (int i = 0; i < crocodile->height; i++) {
        for (int j = 0; j < crocodile->width; j++) {
            attron(COLOR_PAIR(6));// attiva il colore del coccodrillo
            // disegna il carattere nella posizione del coccodrillo
            mvaddch(crocodile->y + i, crocodile->x + j, crocodile->sprite[i][j]);
            attroff(COLOR_PAIR(6));// disattiva il colore del coccodrillo
        }
    }
}
//disegna un proiettile sullo schermo
void draw_projectile(Entity *projectile) {
    //controlla se le coordinate del proiettile sono valide
    if (projectile->x >= 0 && projectile->x < MAP_WIDTH && projectile->y >= 0 && projectile->y < MAP_HEIGHT) {
        attron(COLOR_PAIR(8)); // attiva il colore del proiettile
        // disegna il carattere del proiettile nella sua posizione
        mvaddch(projectile->y, projectile->x, projectile->sprite[0][0]);
        attroff(COLOR_PAIR(8)); // disattiva il colore del proiettile
    }
}
//cancella un proiettile dallo schermo
void clear_projectile(Entity *projectile) {
    for (int i = 0; i < projectile->height; i++) {
        for (int j = 0; j < projectile->width; j++) {
            attron(COLOR_PAIR(map[projectile->y + i][projectile->x + j])); // attiva il colore della cella in cui si trova il proiettile
            mvaddch(projectile->y + i, projectile->x + j, ' '); // cancella il carattere nella posizione del proiettile
            attroff(COLOR_PAIR(map[projectile->y + i][projectile->x + j])); 
        }
    }
}