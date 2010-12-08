/*
This project was created within course Operating Systems and Networks
at FEE CTU, fall semester 2010/2011.

Authors: Petr Messner, Jan Fabian
*/

#include "util.h"

#include <stdlib.h>

#include "configuration.h"


/* I want the seq numbers to be human readable and writable, for easy testing */
int read_seq(const char *data) {
    int i, seq = 0;
    for (i = 0; i < SEQ_NUMBER_SIZE; i++) {
    #ifdef USE_NICE_SEQ
        if (data[i] < '0' || data[i] > '9') {
            return -1; /* invalid number */
        }
        seq *= 10;
        seq += data[i] - '0';
    #else
        seq <<= 8;
        seq += (unsigned char) data[i];
    #endif
    }
    return seq;
}


void write_seq(char *buffer, int seq) {
    int i;
    for (i = SEQ_NUMBER_SIZE - 1; i >= 0; i--) {
    #ifdef USE_NICE_SEQ
        buffer[i] = (seq % 10) + '0';
        seq /= 10;
    #else
        buffer[i] = seq & 0xFF;
        seq >>= 8;
    #endif
    }
}


#ifdef USE_NICE_SEQ
    #define SEQMOD 100
    #define HALFSEQ 50
#else
    #define SEQMOD 0x10000
    #define HALFSEQ 0x8000
#endif


int seq_lt(int a, int b) {
    int d = a - b;
    if ((d < 0 && d > -HALFSEQ) || (d > HALFSEQ)) {
        return 1;
    } else {
        return 0;
    }
}

int seq_gt(int a, int b) {
    int d = a - b;
    if ((d > 0 && d < HALFSEQ) || (d < -HALFSEQ)) {
        return 1;
    } else {
        return 0;
    }
}

int seq_ge(int a, int b) {
    if (a == b) {
        return 1;
    }
    return seq_gt(a, b);
}

int seq_inc(int seq) {
    return (seq + 1) % SEQMOD;
}

int random_number() {
    int r = rand() % 100;
    return r;
}

