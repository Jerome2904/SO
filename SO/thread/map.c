#include "map.h"
#include "game.h"

int map[MAP_HEIGHT][MAP_WIDTH];

void init_bckmap(){
    init_pair(2,COLOR_BLACK,COLOR_GREEN);//PRATO
    init_pair(3,COLOR_BLACK,COLOR_BLUE);//ACQUA
    init_pair(4,COLOR_BLACK,COLOR_YELLOW);//TANA LIBERA
    init_pair(5,COLOR_BLACK,COLOR_RED);//TANA OCCUPATA

    //associo alla matrice map il valore del color pair apposito
    for(int y = 0; y < MAP_HEIGHT;y++){
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            switch (y<10||y>MAP_HEIGHT-6)
            {
            case true:
                    map[y][x]=2;
                break;
            case false:
                    map[y][x]=3;
                break;
            }
        }
        
    }
}

void draw_map(WINDOW* win){
    for (int y = 1; y < MAP_HEIGHT-1; y++)
    {
        for (int x = 1; x < MAP_WIDTH-1; x++)
        {
            wattron(win,COLOR_PAIR(map[y][x]));
            mvwaddch(win,y,x,' ');
            wattroff(win,COLOR_PAIR(map[y][x]));
        }
        
    }
    pthread_mutex_lock(&render_mutex);
    wrefresh(win);
    pthread_mutex_unlock(&render_mutex);
}