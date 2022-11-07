#define _POSIX_SOURCE 1
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s address\n", argv[0]);
        exit(1);
    }

    const char *address = argv[1];

    char* host, file, username, password;

    srand(time(NULL));

    printf("Downloading file %s\n", address);

    return 0;
}
