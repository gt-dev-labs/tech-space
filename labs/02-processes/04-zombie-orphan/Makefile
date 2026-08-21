CC = gcc
CFLAGS = -Wall -Wextra -g

all: zombie orphan

zombie: zombie.c
	$(CC) $(CFLAGS) -o zombie zombie.c

orphan: orphan.c
	$(CC) $(CFLAGS) -o orphan orphan.c

clean:
	rm -f zombie orphan

.PHONY: all clean
