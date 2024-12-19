#ifndef THREADS_H
#define THREADS_H

#include <ncurses.h>
#include "frog.h"

// Parametri per il thread della rana
typedef struct {
    Frog *frog;
    WINDOW *win;
} FrogThreadParams;



#endif // THREADS_H
