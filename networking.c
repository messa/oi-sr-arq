/*
This project was created within course Operating Systems and Networks
at FEE CTU, fall semester 2010/2011.

Authors: Petr Messner, Jan Fabian
*/

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
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(e));
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



int udp_client_socket(const char *host, const char *port) {
    struct addrinfo hints;
    struct addrinfo *result, *r;
    int e;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;  /* allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    e = getaddrinfo(host, port, &hints, &result);
    if (e != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(e));
        exit(EXIT_FAILURE);
    }

    for (r = result; r != NULL; r = r->ai_next) {
        int s = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
        if (s == -1) {
            continue;
        }

        if (connect(s, r->ai_addr, r->ai_addrlen) != 0) {
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


const char* name_from_addr(const struct sockaddr *sa, socklen_t saLength) {
    static char buffer[NI_MAXHOST + 1 + NI_MAXSERV + 1];
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    int e = getnameinfo(
        sa, saLength,
        hbuf, sizeof(hbuf),
        sbuf, sizeof(sbuf),
        NI_NUMERICHOST | NI_NUMERICSERV);
    if (e != 0) {
        fprintf(stderr, "getnameinfo failed: %s\n", gai_strerror(e));
        return NULL;
    }
    snprintf(buffer, sizeof(buffer), "%s:%s", hbuf, sbuf);
    return buffer;
}



