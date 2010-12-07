#ifndef UTIL_H
#define UTIL_H


/**
 * Read sequence number from received frame.
 * Returns -1 if failed.
 */
extern int read_seq(const char *data);


/**
 * Write sequence number to the given buffer.
 */
extern void write_seq(char *buffer, int seq);


/**
 * Return 1 if sequence number a < b, else 0.
 */
extern int seq_lt(int a, int b);

/**
 * Return 1 if sequence number a > b, else 0.
 */
extern int seq_gt(int a, int b);

/**
 * Return 1 if sequence number a >= b, else 0.
 */
extern int seq_ge(int a, int b);

/**
 * Return sequence number incremented by 1.
 */
extern int seq_inc(int);

/*
* generate random number between 0 and 100
*/
extern int random_number();

#endif  /* UTIL_H */
