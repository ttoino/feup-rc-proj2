#include "app.h"
#include "log.h"
#include "url.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <netdb.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int get_address(char *host, char *port, struct addrinfo **res) {
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    return getaddrinfo(host, port, &hints, res);
}

int connect_to(char *host, char *port) {
    struct addrinfo *res;

    LOG("Getting address\n");
    int error = get_address(host, port, &res);
    if (error < 0) {
        ERROR("Could not get address info: %s\n", gai_strerror(error));
        return -1;
    }

    for (struct addrinfo *addr = res; addr != NULL; addr = addr->ai_next) {
        char addr_str[INET6_ADDRSTRLEN];
        inet_ntop(
            addr->ai_family,
            (addr->ai_family == AF_INET6)
                ? (void *)&((struct sockaddr_in6 *)addr->ai_addr)->sin6_addr
                : (void *)&((struct sockaddr_in *)addr->ai_addr)->sin_addr,
            addr_str, INET6_ADDRSTRLEN);

        LOG("Trying %s\n", addr_str);
        int fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (fd < 0 || connect(fd, addr->ai_addr, addr->ai_addrlen) < 0) {
            LOG("Error connecting to address '%s': %s\n", addr_str,
                strerror(errno));
        } else {
            INFO("Connected to address %s\n", addr_str);
            freeaddrinfo(res);
            return fd;
        }
    }

    ERROR("Could not connect to any address!\n");
    freeaddrinfo(res);
    return -1;
}

ssize_t recvall(int fd, char *buf, size_t len) {
    ssize_t bytes_read_total = 0, bytes_read = 0;

    usleep(100000);

    while ((bytes_read = recv(fd, buf + bytes_read_total,
                              len - bytes_read_total, MSG_DONTWAIT)) >= 0) {
        bytes_read_total += bytes_read;
        usleep(100000);
    }

    if (errno != EAGAIN && errno != EWOULDBLOCK) {
        ERROR("%s\n", strerror(errno));
        return -1;
    }

    if (bytes_read_total > 0)
        LOG("%s", buf);

    return bytes_read_total;
}

int get_code(int fd) {
    char buf[2048] = "";
    int code = -1;

    if (recvall(fd, buf, 2048) < 0)
        return -1;

    sscanf(buf, "%d", &code);
    return code;
}

int login(int fd, const char *username, const char *password) {
    char buf[512];
    int len, code;

    code = get_code(fd);
    if (code == 230) {
        INFO("Logged in\n");
        return 0;
    }

    snprintf(buf, 512, "user %s\n%n", username, &len);
    if (send(fd, buf, len, 0) < 0) {
        ERROR("Error sending user command: %s\n", strerror(errno));
        return -1;
    }

    code = get_code(fd);
    if (code == 230) {
        INFO("Logged in using username '%s'\n", username);
        return 0;
    } else if (code != 331) {
        ERROR("Could not login using username '%s'\n", username);
        return -1;
    }

    snprintf(buf, 512, "pass %s\n%n", password, &len);
    if (send(fd, buf, len, 0) < 0) {
        ERROR("Error sending pass command: %s\n", strerror(errno));
        return -1;
    }

    code = get_code(fd);
    if (code != 230) {
        ERROR("Could not login using username '%s' and password '%s'\n",
              username, password);
        return -1;
    }

    INFO("Logged in using username '%s' and password '%s'\n", username,
         password);
    return 0;
}

int get_passive(int fd, char *host, char *port) {
    char cmd[] = "pasv\n", buf[256];
    if (send(fd, cmd, sizeof(cmd), 0) < 0) {
        ERROR("Error sending 'pasv' command: %s\n", strerror(errno));
        return -1;
    }

    recvall(fd, buf, 256);
    int code = -1;
    uint8_t h1, h2, h3, h4, p1, p2;
    sscanf(buf, "%d %*[^(](%hhu,%hhu,%hhu,%hhu,%hhu,%hhu)\n", &code, &h1, &h2,
           &h3, &h4, &p1, &p2);
    if (code != 227) {
        ERROR("Could not enter passive mode, code=%d\n", code);
        return -1;
    }

    sprintf(host, "%hhu.%hhu.%hhu.%hhu", h1, h2, h3, h4);
    sprintf(port, "%hu", p1 * 256 + p2);
    INFO("Entering passive mode (%s:%s)\n", host, port);
    return 0;
}

int start_transfer(int fd, const char *path) {
    char buf[512];
    int len;

    send(fd, "\n", 1, 0);
    get_code(fd);

    snprintf(buf, 512, "retr %s\n%n", path, &len);
    LOG("Sending command '%s'", buf);
    if (send(fd, buf, len, 0) < 0) {
        ERROR("Error sending 'retr' command: %s\n", strerror(errno));
        return -1;
    }

    int code = get_code(fd);
    if (code != 150) {
        ERROR("Could not retrieve file\n");
        return -1;
    }

    LOG("Retrieving file\n");
    return 0;
}

int retrieve(int fd) {
    int out_fd = creat("received", 0744);
    uint8_t buf[256];
    ssize_t bytes_read;

    while ((bytes_read = recv(fd, buf, 256, 0)) > 0) {
        write(out_fd, buf, bytes_read);
        LOG("Written %lu bytes to file\n", bytes_read);
	LOG("%.*s\n", bytes_read, buf);
    }

    close(out_fd);

    return 0;
}

int run(const char *url) {
    char username[256] = "anonymous", password[256] = "", host[256] = "",
         port[6] = "21", path[256] = "", passive_host[INET_ADDRSTRLEN],
         passive_port[6];

    if (parse_url(url, username, password, host, port, path) < 0) {
        ERROR("Invalid url: %s!\n", url);
        return -1;
    }

    set_default_url_parts(username, port);

    INFO("username: %s\n", username);
    INFO("password: %s\n", password);
    INFO("host: %s\n", host);
    INFO("port: %s\n", port);
    INFO("path: %s\n", path);

    int fd = connect_to(host, port);
    if (fd < 0)
        return -1;

    if (login(fd, username, password) < 0)
        return -1;

    if (get_passive(fd, passive_host, passive_port) < 0)
        return -1;

    int passive_fd = connect_to(passive_host, passive_port);
    if (passive_fd < 0)
        return -1;

    if (start_transfer(fd, path) < 0)
        return -1;

    retrieve(passive_fd);

    close(fd);
    close(passive_fd);

    return 0;
}
