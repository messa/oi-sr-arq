
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>

#include "configuration.h"
#include "networking.h"
#include "window.h"


static void print_usage(const char *programName, FILE *out) {
    fprintf(out, "Usage: %s [<options>] <host> <port>\n", programName);
}


static void print_help(const char *programName) {
    print_usage(programName, stdout);
    printf("\n");
    printf("Options:\n");
    printf("  -h  Show this help\n");
}


static void process_arguments(int argc, char *argv[], const char **host, const char **port) {
    int ch;
    while ((ch = getopt(argc, argv, "h")) != -1) {
        switch (ch) {
            case 'h':
                print_help(argv[0]);
                exit(EXIT_SUCCESS);
                break;
            case '?':
            default:
                print_usage(argv[0], stderr);
                fprintf(stderr, "Run %s -h for help.\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind != argc - 2) {
        fprintf(stderr, "Address and port number must be provided as arguments.\n");
        print_usage(argv[0], stderr);
        exit(EXIT_FAILURE);
    }

    *host = argv[optind];
    *port = argv[optind+1];
}


int max(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}


static void send_frame(int s, Window *window, int seq) {

    int n;
    char buf[SEQ_NUMBER_SIZE + MESSAGE_SIZE];

    if (!window_has_seq(window, seq)) {
        return;
    }

    write_seq(buf, seq);
    memcpy(buf + SEQ_NUMBER_SIZE, window_get_message(window, seq),
            window_get_message_length(window, seq));
    if(random_number()<PROBABILITY) {
        n = write(s, buf, SEQ_NUMBER_SIZE + window_get_message_length(window, seq));
        if (n == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }
    }
}


void run_client(int s) {
    int serverWaitingForSeq = 0;
    int seqToFill = 0;
    int stdinClosed = 0;
    struct timeval lastTime;

    Window window;
    init_window(&window);

    for (;;) {
        int n;
        int maxFd = 0;
        fd_set rdset, wrset;
        struct timeval selectTimeout;
        FD_ZERO(&rdset);
        FD_ZERO(&wrset);

        /* budeme cekat na data ze site */
        FD_SET(s, &rdset);
        maxFd = max(maxFd, s);

        /* cekame na standardni vstup, pokud neni okno pro odeslani plne */
        if (!stdinClosed && seq_lt(seqToFill, serverWaitingForSeq + WINDOW_SIZE)) {
            FD_SET(0, &rdset);
            maxFd = max(maxFd, 0);
        }

        selectTimeout.tv_sec = TIMEOUT / 1000;
        selectTimeout.tv_usec = (TIMEOUT % 1000) * 1000;
        n = select(maxFd+1, &rdset, &wrset, NULL, &selectTimeout);

        if (n == -1) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(0, &rdset)) {
            /* precteme data ze standardniho vstupu a ulozime do okna a odesleme je */
            char buf[MESSAGE_SIZE];
            int n = read(0, buf, sizeof(buf));
            if (n == -1) {
                perror("read");
                exit(EXIT_FAILURE);
            }

            if (n == 0) {
                stdinClosed = 1;
            }

            window_store(&window, seqToFill, buf, n);
            send_frame(s, &window, seqToFill);
            gettimeofday(&lastTime, NULL);
            seqToFill = seq_inc(seqToFill);
        }

        if (FD_ISSET(s, &rdset)) {
            /* prislo neco po siti (zrejme potvrzeni) - prijmeme to (recv, read...) */
            /* zvednout serverWaitingForSeq, pokud cislo, ktere prijde, bude vyssi */
            /*  + odeslani prvniho ramce v okne */

            char buf[SEQ_NUMBER_SIZE];
            int n = read(s, buf, sizeof(buf));
            if (n == -1) {
                perror("read");
                exit(EXIT_FAILURE);
            }
            if (n == SEQ_NUMBER_SIZE) {
                /* je to datagram spravne delky */
                int seq = read_seq(buf);
                if (seq_gt(seq, serverWaitingForSeq) /* seq > serverWaitingForSeq */) {
                    while (seq_gt(seq, serverWaitingForSeq)) {
                        serverWaitingForSeq = seq_inc(serverWaitingForSeq);
                    }
                    if (stdinClosed && serverWaitingForSeq == seqToFill) {
                        /* transfer finished successfully */
                        break;
                    }
                } else if (seq == serverWaitingForSeq) {
                    /* server potvrzuje neco, co uz potvrdil; posleme znovu prvni
                       ramec z okna */
                    send_frame(s, &window, serverWaitingForSeq);
                    gettimeofday(&lastTime, NULL);
                }
            }
        }

        if (serverWaitingForSeq != seqToFill) {  /* je neco k odeslani */
            struct timeval now;
            gettimeofday(&now, NULL);
            int delta = (now.tv_sec - lastTime.tv_sec) * 1000; /* in miliseconds */
            delta += (now.tv_usec - lastTime.tv_usec) / 1000;
            if (delta > TIMEOUT) {
                send_frame(s, &window, serverWaitingForSeq);
                gettimeofday(&lastTime, NULL);
            }
        }

    }
}


int main(int argc, char *argv[]) {
    const char *host = NULL;
    const char *port = NULL;
    int s;

    process_arguments(argc, argv, &host, &port);

    fprintf(stderr, "Connecting to %s %s...\n", host, port);

    s = udp_client_socket(host, port);

    run_client(s);
    exit(EXIT_SUCCESS);
}

