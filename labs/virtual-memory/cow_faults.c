#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>

#define BUF_SIZE (100UL * 1024 * 1024) // 100 MB

int main(void) {
    char *buf = malloc(BUF_SIZE);
    memset(buf, 'A', BUF_SIZE); // touch every page now, before fork

    pid_t pid = fork();

    if (pid == 0) {
        struct rusage before, after;
        // TODO: getrusage(RUSAGE_SELF, &before) right after fork,
        //       before touching buf again

        memset(buf, 'B', BUF_SIZE); // triggers one COW fault per page

        // TODO: getrusage(RUSAGE_SELF, &after)
        // TODO: print after.ru_minflt - before.ru_minflt
        //       (the "child minor faults caused by this write")

        _exit(0);
    } else {
        struct rusage before, after;
        // TODO: getrusage(RUSAGE_SELF, &before) for the PARENT, before waiting

        int status;
        // TODO: waitpid(pid, &status, 0)

        // TODO: getrusage(RUSAGE_SELF, &after) for the PARENT again
        // TODO: print after.ru_minflt - before.ru_minflt
        //       (should be small — the parent never wrote to buf after forking)
    }

    free(buf);
    return 0;
}
