CC = gcc
CFLAGS = -Wall -Wextra -g

mini_shell: mini_shell.c
	$(CC) $(CFLAGS) -o mini_shell mini_shell.c

run: mini_shell
	./mini_shell

clean:
	rm -f mini_shell

.PHONY: run clean
