CC = gcc
CFLAGS = -Wall -Wextra -g

pid_info: pid_info.c
	$(CC) $(CFLAGS) -o pid_info pid_info.c

run: pid_info
	./pid_info

clean:
	rm -f pid_info

.PHONY: run clean
