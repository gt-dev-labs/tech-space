#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int initialized_global = 42;   // TODO: which segment is this in?
int uninitialized_global;      // TODO: which segment is this in?

int main(void) {
    // TODO: print PID and PPID using getpid()/getppid()

    int stack_var = 1;
    int *heap_var = malloc(sizeof(int));
    *heap_var = 1;

    // TODO: print the addresses of, in this order:
    //   - main (function pointer, cast to (void *))
    //   - initialized_global
    //   - uninitialized_global
    //   - heap_var (the pointed-to memory, not the pointer variable itself)
    //   - stack_var
    // use "%p" and cast each address to (void *)

    free(heap_var);
    return 0;
}
