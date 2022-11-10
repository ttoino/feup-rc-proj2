#include "url.h"

#include <stdio.h>
#include <string.h>

typedef enum {
    START,
    READ_SCHEME,
    READ_USERNAME,
    READ_HOST,
    READ_PORT,
    READ_PATH,
    END
} AddressState;

int parse_url(const char *url, char *username, char *password, char *host,
              uint16_t *port, char *path) {
    AddressState state = START;

    while (state != END) {
        int n = 0;

        switch (state) {
        case START:
            state = READ_SCHEME;
            break;

        case READ_SCHEME:
            sscanf(url, "ftp://%n", &n);
            url += n;
            state = READ_USERNAME;
            break;

        case READ_USERNAME:
            if (sscanf(url, "%255[^:@/]:%255[^:@/]@%n", username, password,
                       &n) &&
                n > 0) {
                url += n;
            } else if (sscanf(url, "%255[^:@/]@%n", username, &n) && n > 0) {
                url += n;
                password[0] = '\0';
            } else {
                username[0] = '\0';
                password[0] = '\0';
            }

            state = READ_HOST;
            break;

        case READ_HOST:
            if (sscanf(url, "%255[^:@/]:%n", host, &n) && n > 0)
                state = READ_PORT;
            else if (sscanf(url, "%255[^:@/]/%n", host, &n) && n > 0)
                state = READ_PATH;
            else if (sscanf(url, "%255[^:@/]%n", host, &n) && n > 0)
                state = END;
            else
                return -1;

            url += n;
            break;

        case READ_PORT:
            if (sscanf(url, "%hu/%n", port, &n) && n > 0) {
                url += n;
                state = READ_PATH;
            } else if (sscanf(url, "%hu%n", port, &n) && n > 0) {
                url += n;
                state = END;
            } else {
                return -1;
            }

            break;

        case READ_PATH:
            if (sscanf(url, "%255[^:@]%n", path, &n) == -1)
                return -1;

            url += n;

        default:
            state = END;
            break;
        }
    }

    if (strlen(url) > 0)
        return -1;

    return 0;
}
