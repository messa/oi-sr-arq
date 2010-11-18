
#include "networking.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


int udp_server_socket(const char *host, const char *port) {
    struct addrinfo hints;
    struct addrinfo *result, *r;
    int e;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;  /* allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    e = getaddrinfo(host, port, &hints, &result);
    if (e != 0) {
        fprintf(stderr, "getaddrinf: %s\n", gai_strerror(e));
        exit(EXIT_FAILURE);
    }

    for (r = result; r != NULL; r = r->ai_next) {
        int s = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
        if (s == -1) {
            continue;
        }

        if (bind(s, r->ai_addr, r->ai_addrlen) != 0) {
            close(s);
            continue;
        }

        freeaddrinfo(result);
        return s;
    }

    freeaddrinfo(result);
    fprintf(stderr, "Failed\n");
    exit(EXIT_FAILURE);
}




