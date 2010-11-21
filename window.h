#ifndef WINDOW_H
#define WINDOW_H

#include "configuration.h"

#include <stdio.h>


struct WindowItem {
    int seq;
    int length;
    char message[MESSAGE_SIZE];
};

typedef struct WindowItem Window[WINDOW_SIZE];


extern void init_window(Window *window);

extern int window_has_seq(Window *window, int seq);

extern void window_store(Window *window, int seq, char *message, int messageLength);

extern void window_print_message(Window *window, int seq, FILE *out);


#endif  /* WINDOW_H */


