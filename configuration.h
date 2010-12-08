/*
This project was created within course Operating Systems and Networks
at FEE CTU, fall semester 2010/2011.

Authors: Petr Messner, Jan Fabian
*/

#ifndef CONFIGURATION_H
#define CONFIGURATION_H


#define SEQ_NUMBER_SIZE 2
#define MESSAGE_SIZE 200
#define TIMEOUT 1000

#define WINDOW_SIZE 8
#define PROBABILITY 90

/* Whether to use human readable and readable sequence numbers, so the server
and client are easily testable from tools like netcat */
#define USE_NICE_SEQ


#endif /* CONFIGURATION_H */


