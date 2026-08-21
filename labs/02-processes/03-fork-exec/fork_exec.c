#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <command> [args...]\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();

    if (pid == 0) {
        // TODO: execvp(argv[1], &argv[1])
        // TODO: if execvp returns, it failed:
        //       perror("execvp"); _exit(127);
    } else if (pid > 0) {
        int status;
        // TODO: waitpid(pid, &status, 0)
        // TODO: if WIFEXITED(status), print WEXITSTATUS(status)
        // TODO: else if WIFSIGNALED(status), print WTERMSIG(status)
    } else {
        perror("fork");
        return 1;
    }

    return 0;
}
