CC=gcc
CFLAGS=-Wall -pedantic
DEPS = lib.h morselib.h
OBJ = main.o lib.o morselib.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

DataTransfer: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
