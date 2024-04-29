include custom.mk
CC=gcc
CFLAGS=-Wall -Wextra -pedantic -Wconversion -Wunreachable-code
RLFLAGS=`pkg-config --cflags --libs raylib`
EXE=./run
OBJS=io.o world.o

all: clean main

main: world.o io.o main.c
	$(call print_in_color,$(GREEN),\nCompiling to $(EXE) ...\n)
	$(CC) $(CFLAGS) main.c -o $(EXE) $(RLFLAGS) $(OBJS) -O2

%.o: %.c
	$(call print_in_color,$(YELLOW),\nBuilding OBJ $@ ...\n)
	$(CC) $(CFLAGS) -c $< -o $@ -O2

clean:
	$(call print_in_color,$(RED),Cleaning ...\n)
	rm -rf $(EXE) *.o *.i *.asm
