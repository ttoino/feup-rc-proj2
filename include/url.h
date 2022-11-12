#ifndef _URL_H_
#define _URL_H_

#include <stdint.h>

int parse_url(const char *url, char *username, char *password, char *host,
              char *port, char *path);

void set_default_url_parts(char *username, char *port);

#endif // _URL_H_
