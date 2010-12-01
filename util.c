
#include "util.h"

#include "configuration.h"


/* I want the seq numbers to be human readable and writable, for easy testing */
int read_seq(const char *data) {
    int i, seq = 0;
    for (i = 0; i < SEQ_NUMBER_SIZE; i++) {
        if (data[i] < '0' || data[i] > '9') {
            return -1; /* invalid number */
        }
        seq *= 10;
        seq += data[i] - '0';
    }
    return seq;
}


void write_seq(char *buffer, int seq) {
    int i;
    for (i = SEQ_NUMBER_SIZE - 1; i >= 0; i--) {
        buffer[i] = (seq % 10) + '0';
        seq /= 10;
    }
}

