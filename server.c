
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "networking.h"


/*
 * Returns hexadecimal representation of some binary string.
 */
static char* hexdump(const char *ptr, int size) {
    static char buf[1024];
    int pos = 0;
    int i;
    memset(&buf, 0, sizeof(buf));
    for (i = 0; i < size; i++) {
        if (pos > sizeof(buf) - 10) {
            buf[pos++] = '.';
            buf[pos++] = '.';
            buf[pos++] = '.';
            break;
        }
        if (i > 0) {
            buf[pos++] = ' ';
        }
        int n = snprintf(buf + pos, sizeof(buf) - pos, "%02x", (unsigned char) ptr[i]);
        pos += n;
    }
    buf[pos] = '\0';
    return buf;
}


void run_server(int listenSocket) {
    char buf[1500];
    ssize_t n;
    struct sockaddr_storage addr;
    socklen_t addrLen = sizeof(addr);

    for (;;) {
        n = recvfrom(listenSocket, buf, sizeof(buf), 0, (struct sockaddr*) &addr, &addrLen);

        printf("Received %s from %s\n", hexdump(buf, n),
            name_from_addr((struct sockaddr *) &addr, addrLen));
    }
}


struct ServerOptions {
    const char *host;
    const char *port;
};


static void process_arguments(int argc, char *argv[], struct ServerOptions *);


int main(int argc, char *argv[]) {
    struct ServerOptions options;
    memset(&options, 0, sizeof(options));
    options.host = NULL;
    options.port = NULL;

    process_arguments(argc, argv, &options);

    int s = udp_server_socket(options.host, options.port);
    run_server(s);

    return 0;
}


static void print_usage(const char *programName, FILE *out) {
    fprintf(out, "Usage: %s [<options>] <port>\n", programName);
}


static void print_help(const char *programName) {
    print_usage(programName, stdout);
    printf("\n");
    printf("Options:\n");
    printf("  -h            Show this help\n");
    printf("  -b <address>  Bind to a specific address\n");
}


static void process_arguments(int argc, char *argv[],
                              struct ServerOptions *options)
{
    int ch;
    while ((ch = getopt(argc, argv, "hb:")) != -1) {
        switch (ch) {
            case 'h':
                print_help(argv[0]);
                exit(EXIT_SUCCESS);
                break;
            case 'b':
                options->host = optarg;
                break;
            case '?':
            default:
                print_usage(argv[0], stderr);
                fprintf(stderr, "Run %s -h for help.\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind != argc - 1) {
        fprintf(stderr, "Port number must be provided as argument.\n");
        print_usage(argv[0], stderr);
        exit(EXIT_FAILURE);
    }

    options->port = argv[optind];
}

