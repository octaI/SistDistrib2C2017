#ifndef DISTRIBUIDOS_MESSAGE_H
#define DISTRIBUIDOS_MESSAGE_H


#include "../constants.h"

/* Definition message */

typedef struct {
    char test_msg [MAX_LENGTH] ;
} m0_test;

typedef struct {
    int client_id;
} m1_connection_request;

typedef struct {
    int connection_accepted;
} m2_connection_accepted;

/* Definition of message pattern */

typedef struct {
    int message_type;
    unsigned int message_choice_number;
    union {
        m0_test m0;
        m1_connection_request m1;
        m2_connection_accepted m2;
    } message_choice;
} q_message;

/* Definition message type id */
#define TYPE_CONNECTION_REQUEST     1

/* Definition message choice number */
//#define CHOICE_CONNECTION_REQUEST   1  not required because it has a specified type
#define CHOICE_CONNECTION_ACCEPTED    2

#endif //DISTRIBUIDOS_MESSAGE_H
