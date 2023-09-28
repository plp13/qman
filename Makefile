# Configurable variables
CC=gcc -O0 -g

COMPILER_CMD=$(CC)
LINKER_CMD=$(CC)
ONESTEP_CMD=$(CC)

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
