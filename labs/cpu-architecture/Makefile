CC = gcc
CFLAGS = -Wall -Wextra -g -O0

all: hello crash raw

hello: hello.c
	$(CC) $(CFLAGS) -o hello hello.c

crash: crash.c
	$(CC) $(CFLAGS) -o crash crash.c

raw.o: raw.s
	as raw.s -o raw.o

raw: raw.o
	ld raw.o -o raw

clean:
	rm -f hello crash raw raw.o

.PHONY: all clean
