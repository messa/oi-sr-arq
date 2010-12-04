
#include "window.h"

#include <string.h>


void init_window(Window *window) {
    int i;
    for (i = 0; i < WINDOW_SIZE; i++) {
        (*window)[i].seq = -1;
    }
}


int window_has_seq(Window *window, int seq) {
    if ((*window)[seq % WINDOW_SIZE].seq == seq) {
        return 1;
    } else {
        return 0;
    }
}


void window_store(Window *window, int seq, char *message, int messageLength) {
    struct WindowItem *wi = &((*window)[seq % WINDOW_SIZE]);
    wi->seq = seq;
    wi->length = messageLength;
    memcpy(wi->message, message, messageLength);
}


void window_print_message(Window *window, int seq, FILE *out) {
    struct WindowItem *wi = &((*window)[seq % WINDOW_SIZE]);
    if (wi->seq != seq) {
        return;
    }
    fwrite(wi->message, wi->length, 1, out);
}


const char* window_get_message(Window *window, int seq) {
    struct WindowItem *wi = &((*window)[seq % WINDOW_SIZE]);
    if (wi->seq != seq) {
        return NULL;
    }
    return wi->message;
}

int window_get_message_length(Window *window, int seq) {
    struct WindowItem *wi = &((*window)[seq % WINDOW_SIZE]);
    if (wi->seq != seq) {
        return 0;
    }
    return wi->length;
}




