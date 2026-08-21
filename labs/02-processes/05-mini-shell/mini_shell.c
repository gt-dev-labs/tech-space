#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

int main(void) {
    char line[MAX_LINE];
    char *argv[MAX_ARGS];

    while (1) {
        printf("> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            break; // EOF
        }

        line[strcspn(line, "\n")] = '\0';

        if (strcmp(line, "exit") == 0) {
            break;
        }

        // TODO: tokenize `line` into argv[] using strtok(line, " ") / strtok(NULL, " ")
        //       NULL-terminate argv when done

        // TODO: if argv[0] is NULL (blank line), continue

        // TODO: bonus built-in — if strcmp(argv[0], "cd") == 0,
        //       chdir(argv[1]) and continue (don't fork for this)

        // TODO: fork(); in the child, execvp(argv[0], argv), then _exit(127) on failure
        // TODO: in the parent, waitpid() and print the child's exit status
    }

    return 0;
}
