#include "../constants.h"

#ifndef DISTRIBUIDOS_MESSAGE_H
#define DISTRIBUIDOS_MESSAGE_H


/* Definition message */

typedef struct {
    char test_msg [MAX_LENGTH] ;
} m0_test;

typedef struct {
    int client_id;
} m1_connection_request;

/* Definition of message pattern */

typedef struct {
    int message_type;
    union {
        m0_test m0;
        m1_connection_request m1;
    } message_choice;
} q_message;

/* Definition message type id */

#define TYPE_CONNECTION_REQUEST 1

#endif //DISTRIBUIDOS_MESSAGE_H
