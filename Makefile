PROGS=DataTransfer ChildProgram

CC=gcc
CFLAGS=-Wall -pedantic  
DEPS = lib.h morselib.h
OBJ = morselib.o lib.o
OBJ1 = main.o $(OBJ)
OBJ2 = childprogram.o $(OBJ)

all: $(PROGS)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

DataTransfer: $(OBJ1)
	$(CC) -o $@ $^ $(CFLAGS)

ChildProgram: $(OBJ2)
	$(CC) -o $@ $^ $(CFLAGS)
