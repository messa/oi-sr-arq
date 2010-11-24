/*
This project was created within course Operating Systems and Networks
at FEE CTU, fall semester 2010/2011.

Authors: Petr Messner, Jan Fabian
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


static void print_usage(const char *programName, FILE *out) {
    fprintf(out, "Usage: %s [<options>] <host> <port>\n", programName);
}


static void print_help(const char *programName) {
    print_usage(programName, stdout);
    printf("\n");
    printf("Options:\n");
    printf("  -h            Show this help\n");
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



int main(int argc, char *argv[]) {
    const char *host = NULL;
    const char *port = NULL;

    process_arguments(argc, argv, &host, &port);

    fprintf(stderr, "Connecting to %s %s\n", host, port);
}
