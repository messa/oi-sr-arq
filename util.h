#ifndef UTIL_H
#define UTIL_H


extern int read_seq(const char *data);

extern void write_seq(char *buffer, int seq);

extern int seq_lt(int a, int b);

extern int seq_ge(int a, int b);

extern int seq_inc(int);


#endif  /* UTIL_H */
