#ifndef _URL_H_
#define _URL_H_

#include <stdint.h>

int parse_url(const char *url, char *username, char *password, char *host,
              uint16_t *port, char *path);

#endif // _URL_H_
