/*
This project was created within course Operating Systems and Networks
at FEE CTU, fall semester 2010/2011.

Authors: Petr Messner, Jan Fabian
*/

#ifndef NETWORKING_H
#define NETWORKING_H

#include <sys/socket.h>


extern int udp_server_socket(const char *host, const char *port);

extern int udp_client_socket(const char *host, const char *port);


/*
 * Get human-readable address and service (port) from given sockaddr.
 *
 * If error occurres, prints error message to standard error output and
 * returns NULL,
 *
 * Returns a pointer to static buffer; this means it must not be freeed,
 * but the value stored in that buffer is valid until the next call of
 * this function.
 */
extern const char* name_from_addr(const struct sockaddr *, socklen_t);


#endif /* NETWORKING_H */


