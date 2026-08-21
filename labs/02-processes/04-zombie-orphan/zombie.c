// TODO: one sentence — why are zombies harmless in small numbers but
// a real problem for a long-running process that never reaps children?

#include <stdio.h>
#include <unistd.h>

int main(void) {
    pid_t pid = fork();

    if (pid == 0) {
        // TODO: print child's own PID, then _exit(0) immediately
    } else {
        // TODO: parent — print a message telling the user to check `ps` now,
        //       then sleep(30) WITHOUT calling wait()
    }

    return 0;
}
