include custom.mk
CC=gcc
CFLAGS=-Wall -Wextra -pedantic -Wconversion -Wunreachable-code
RLFLAGS=`pkg-config --cflags --libs raylib`
EXE=./run

all: clean main

main: main.c
	$(call print_in_color,$(YELLOW),\nCompiling to $(EXE) ...\n)
	$(CC) $(CFLAGS) main.c -o $(EXE) $(RLFLAGS) -lncurses

clean:
	$(call print_in_color,$(RED),Cleaning ...\n)
	rm -rf $(EXE) *.o *.i *.asm
