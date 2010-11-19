
#ifndef NETWORKING_H
#define NETWORKING_H

#include <sys/socket.h>


extern int udp_server_socket(const char *host, const char *port);

extern const char* name_from_addr(const struct sockaddr *, socklen_t);


#endif /* NETWORKING_H */


