#define _POSIX_SOURCE 1
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include "url.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s url\n", argv[0]);
        exit(1);
    }

    const char *url = argv[1];

    char username[256], password[256], host[256], path[256];
    uint16_t port = 21;

    int r = parse_url(url, username, password, host, &port, path);

    if (r < 0)
        fprintf(stderr, "Invalid url!\n");
    else
        printf("username: %s\npassword: %s\nhost: %s\nport: %hu\npath: %s\n",
               username, password, host, port, path);

    printf("Downloading file %s\n", url);

    return 0;
}
