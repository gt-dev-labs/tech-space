#include <stdio.h>
#include <unistd.h>

int main(void) {
    pid_t pid = fork();

    if (pid == 0) {
        // TODO: child — print own PID and PPID,
        //       sleep(2),
        //       print PPID again (should now be 1 / a reaper, not the original parent)
    } else {
        // TODO: parent — print own PID, then exit immediately (no wait, no sleep)
    }

    return 0;
}
