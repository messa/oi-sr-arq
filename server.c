
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "networking.h"


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

