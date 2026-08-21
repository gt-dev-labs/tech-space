CC = gcc
CFLAGS = -Wall -Wextra -g

fork_basics: fork_basics.c
	$(CC) $(CFLAGS) -o fork_basics fork_basics.c

run: fork_basics
	./fork_basics

clean:
	rm -f fork_basics

.PHONY: run clean
