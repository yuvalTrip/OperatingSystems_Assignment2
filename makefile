#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
 
# Compilation of all files must be enabled using the command 'Make all'
# Make clean must delete all craft
# Exe files will called: mytee.c --> mytee, mync.c --> mync, myshell.c --> myshell
 
CC = gcc
AR = ar
FLAGS = -Wall -g

OBJECTS_Shell = myshell.o

all: myshell mytee mync

mytee: mytee.o
	$(CC) $(FLAGS) -o mytee mytee.o

mync: mync.o 
	$(CC) $(FLAGS) -o mync mync.o

myshell: $(OBJECTS_Shell)
	$(CC) $(FLAGS) -o myshell $(OBJECTS_Shell) 

.PHONY: clean all

clean:
	rm -f *.o *.so myshell mytee mync
