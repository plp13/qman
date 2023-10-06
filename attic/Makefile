# Configurable variables
CC=gcc

COMPILER_CMD=$(CC) `pkg-config --cflags ncursesw` -O0 -g
LINKER_CMD=$(CC) `pkg-config --libs ncursesw` -O0 -g
ONESTEP_CMD=$(CC) `pkg-config --cflags --libs ncursesw` -O0 -g

all: qman

qman: util.o program.o qman.o
	$(LINKER_CMD) -o qman program.o util.o qman.o

util.o: util.c
	$(COMPILER_CMD) -c util.c

program.o: program.c
	$(COMPILER_CMD) -c program.c

qman.o: qman.c
	$(COMPILER_CMD) -c qman.c

clean:
	rm -f *.o qman
