
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "networking.h"


/*
TODO:
  - random frame dropping
  - cycling seq numbers
*/


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


/* I want the seq numbers to be human readable and writable, for easy testing */
static int read_seq(const char *data) {
    int i, seq = 0;
    for (i = 0; i < 2; i++) {
        if (data[i] < '0' || data[i] > '9') {
            return -1; // invalid number
        }
        seq *= 10;
        seq += data[i] - '0';
    }
    return seq;
}


void write_seq(char *buffer, int seq) {
    buffer[0] = (seq / 10) + '0';
    buffer[1] = (seq % 10) + '0';
}


struct WindowItem {
    int seq;
    int length;
    char message[200];
};


void run_server(int listenSocket) {
    int waitingForSeq = 0;

    #define WINDOW_SIZE 8
    struct WindowItem window[WINDOW_SIZE];

    {
        int i;
        for (i = 0; i < WINDOW_SIZE; i++) {
            window[i].seq = -1;
        }
    }

    for (;;) {
        char buf[1500];
        int seq;
        ssize_t n;
        struct sockaddr_storage addr;
        socklen_t addrLen = sizeof(addr);

        // receive frame
        n = recvfrom(listenSocket, buf, sizeof(buf), 0, (struct sockaddr*) &addr, &addrLen);

        fprintf(stderr, "Received %s from %s\n", hexdump(buf, n),
            name_from_addr((struct sockaddr *) &addr, addrLen));

        if (n < 2) {
            fprintf(stderr, "Frame is too short\n");
            continue;
        }

        seq = read_seq(buf);
        if (seq == -1) {
            fprintf(stderr, "Invalid seq\n");
            continue;
        }
        fprintf(stderr, "Got frame with seq %d\n", seq);

        if (seq >= waitingForSeq && seq < waitingForSeq + WINDOW_SIZE) {
            /* save to the window */
            if (window[seq % WINDOW_SIZE].seq != seq) {
                fprintf(stderr, "Saving seq %d to the window\n", seq);
                window[seq % WINDOW_SIZE].seq = seq;
                window[seq % WINDOW_SIZE].length = n-2;
                memcpy(window[seq % WINDOW_SIZE].message, buf+2, n-2);
            }
        }

        while (window[waitingForSeq % WINDOW_SIZE].seq == waitingForSeq) {
            fwrite(window[waitingForSeq % WINDOW_SIZE].message, window[waitingForSeq % WINDOW_SIZE].length, 1, stdout);
            fflush(stdout);
            waitingForSeq++;
        }

        // send ACK
        {
            char ackBuf[2];
            write_seq(ackBuf, waitingForSeq);
            n = sendto(listenSocket, ackBuf, 2, 0, (struct sockaddr*) &addr, addrLen);
        }
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

