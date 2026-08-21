#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE (200UL * 1024 * 1024) // 200 MB
#define PAGE_SIZE 4096

void print_rss(const char *label) {
    // TODO: fopen("/proc/self/status", "r")
    // TODO: fgets() line by line until you find one starting with "VmRSS:"
    // TODO: print it (it already includes the "VmRSS:" prefix and " kB" suffix),
    //       prefixed with `label`
    // TODO: fclose()
}

int main(void) {
    print_rss("before malloc");

    char *buf = malloc(BUF_SIZE);
    print_rss("after malloc, before touching pages");

    // TODO: write one byte to every page in buf (a loop stepping by PAGE_SIZE
    //       is enough — touching any byte in a page faults the whole page in)

    print_rss("after touching every page");

    free(buf);
    return 0;
}
