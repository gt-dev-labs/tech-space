CC = gcc
CFLAGS = -Wall -Wextra -g

all: demand_paging cow_faults

demand_paging: demand_paging.c
	$(CC) $(CFLAGS) -o demand_paging demand_paging.c

cow_faults: cow_faults.c
	$(CC) $(CFLAGS) -o cow_faults cow_faults.c

run-demand: demand_paging
	./demand_paging

run-cow: cow_faults
	./cow_faults

clean:
	rm -f demand_paging cow_faults

.PHONY: all run-demand run-cow clean
