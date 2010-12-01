/*
This project was created within course Operating Systems and Networks
at FEE CTU, fall semester 2010/2011.

Authors: Petr Messner, Jan Fabian
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>

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


void run_client(int s) {
    int serverWaitingForSeq = 0;
    int seqToFill = 0;

    Window window;
    init_window(&window);

    for (;;) {
        int n;
        int maxFd = 0;
        fd_set rdset, wrset;
        FD_ZERO(&rdset);
        FD_ZERO(&wrset);

        /* budeme cekat na data ze site */
        FD_SET(s, &rdset);
        maxFd = max(maxFd, s);

        /* cekame na standardni vstup, pokud neni okno pro odeslani plne */
        if (seqToFill < serverWaitingForSeq + WINDOW_SIZE) {
            FD_SET(0, &rdset);
            maxFd = max(maxFd, 0);
        }

        n = select(maxFd+1, &rdset, &wrset, NULL, NULL);

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

            window_store(window, seqToFill, buf, n);

		    {
                int r;
                char buf2[SEQ_NUMBER_SIZE + n];
                write_seq(buf2, seqToFill);
                memcpy(buf2 + SEQ_NUMBER_SIZE, buf, n);

                r = write(s, buf2, SEQ_NUMBER_SIZE + n);
                if (r == -1) {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
	        }

            seqToFill ++;
        }

        if (FD_ISSET(s, &rdset)) {
            /* prislo neco po siti (zrejme potvrzeni) - prijmeme to (recv, read...) */
            /* zvednout serverWaitingForSeq, pokud cislo, ktere prijde, bude vyssi */
            /*   + smazat z okna to, co uz bylo potvrzeno */

            /*  + odeslani prvniho ramce v okne */
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
}

