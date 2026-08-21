#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    int counter = 100;

    pid_t pid = fork();

    // TODO: handle pid == -1 (fork failed): perror("fork") and exit(1)

    if (pid == 0) {
        // TODO: child — print own PID (getpid()) and PPID (getppid()),
        //       then counter += 1, then print the new value of counter
    } else {
        // TODO: parent — print own PID and the child's PID (pid),
        //       sleep(1), then print counter
    }

    return 0;
}
