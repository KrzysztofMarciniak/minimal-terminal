CC = gcc
CFLAGS = -Wall -std=c17 -pthread
LDFLAGS = -lX11

SRC = main.c render.c input.c ansi.c terminal.c
OBJ = $(SRC:.c=.o)
EXEC = mt
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(LDFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(OBJ) $(EXEC)

install: $(EXEC)
	install -d $(BINDIR)
	install -m 755 $(EXEC) $(BINDIR)
