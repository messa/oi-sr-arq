/*
This project was created within course Operating Systems and Networks
at FEE CTU, fall semester 2010/2011.

Authors: Petr Messner, Jan Fabian
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "configuration.h"
#include "networking.h"
#include "window.h"
#include "util.h"


/**
 * Returns hexadecimal representation of some binary string.
 * Only for debug purposes.
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


void run_server(int listenSocket, int verbosity) {
    int waitingForSeq = 0;

    Window window;
    init_window(&window);

    for (;;) {
        char buf[SEQ_NUMBER_SIZE + MESSAGE_SIZE + 1];
        int seq;
        ssize_t n;
        struct sockaddr_storage addr;
        socklen_t addrLen = sizeof(addr);

        /* receive frame */
        n = recvfrom(listenSocket, buf, sizeof(buf), 0, (struct sockaddr*) &addr, &addrLen);

        if (verbosity > 1) {
            fprintf(stderr, "Received %s from %s\n", hexdump(buf, n),
                name_from_addr((struct sockaddr *) &addr, addrLen));
        }

        if (n < SEQ_NUMBER_SIZE) {
            fprintf(stderr, "Frame is too short\n");
            continue;
        }
        if (n == sizeof(buf)) {
            fprintf(stderr, "Frame is too long\n");
            continue;
        }

        seq = read_seq(buf);
        if (seq == -1) {
            fprintf(stderr, "Invalid seq\n");
            continue;
        }
        if (verbosity > 0) {
            fprintf(stderr, "Got frame with seq %d\n", seq);
        }

        if (seq_ge(seq, waitingForSeq) /* seq >= waitingForSeq */ &&
            seq_lt(seq, waitingForSeq + WINDOW_SIZE) /* seq < waitingForSeq + WINDOW_SIZE */) {
            if (! window_has_seq(&window, seq)) {
                if (verbosity > 0) {
                    fprintf(stderr, "Saving seq %d to the window\n", seq);
                }
                window_store(&window, seq, buf+SEQ_NUMBER_SIZE, n-SEQ_NUMBER_SIZE);
            }
        }

        while (window_has_seq(&window, waitingForSeq)) {
            window_print_message(&window, waitingForSeq, stdout);
            if (window_get_message_length(&window, waitingForSeq) == 0) {
                fclose(stdout);
            }
            waitingForSeq = seq_inc(waitingForSeq);
        }
        fflush(stdout);

        /* send ACK */
        {
            char ackBuf[SEQ_NUMBER_SIZE];

            write_seq(ackBuf, waitingForSeq);

            if (random_number() < PROBABILITY) {
            	n = sendto(listenSocket, ackBuf, sizeof(ackBuf), 0, (struct sockaddr*) &addr, addrLen);
            }
        }
    }
}


struct ServerOptions {
    const char *host;
    const char *port;
    int verbosity;
};


static void process_arguments(int argc, char *argv[], struct ServerOptions *);


int main(int argc, char *argv[]) {
    struct ServerOptions options;
    memset(&options, 0, sizeof(options));
    options.host = NULL;
    options.port = NULL;
    options.verbosity = 0;

    process_arguments(argc, argv, &options);

    int s = udp_server_socket(options.host, options.port);
    run_server(s, options.verbosity);

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
    printf("  -v            Be more verbose; can be used multiple times\n");
}


static void process_arguments(int argc, char *argv[],
                              struct ServerOptions *options)
{
    int ch;
    while ((ch = getopt(argc, argv, "hb:v")) != -1) {
        switch (ch) {
            case 'h':
                print_help(argv[0]);
                exit(EXIT_SUCCESS);
                break;
            case 'b':
                options->host = optarg;
                break;
            case 'v':
                options->verbosity++;
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

