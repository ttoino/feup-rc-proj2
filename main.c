#define _POSIX_SOURCE 1
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include "app.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s url\n", argv[0]);
        exit(1);
    }

    return run(argv[1]);
}
