# Nome dell'eseguibile
TARGET = frogger

# File sorgente
SRC = main.c game.c timer.c frog.c map.c

# File oggetto (generati dai sorgenti)
OBJ = $(SRC:.c=.o)

# Header file
HEADERS = game.h timer.h frog.h map.h paramThreads.h

# Flags del compilatore
CC = gcc
FLAGS = -lncurses -lpthread -Wall -Wextra

# Regola principale
all: $(TARGET)

# Regola per creare l'eseguibile
$(TARGET): $(OBJ)
	gcc -o $@ $^ $(FLAGS)

# Regola per creare i file oggetto
%.o: %.c $(HEADERS)
	$(CC) -c $< -o $@

# Pulizia dei file generati
clean:
	rm -f $(OBJ) $(TARGET)

# Pulizia e ricompilazione completa
rebuild: clean all

