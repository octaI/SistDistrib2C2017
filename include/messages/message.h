#include "../constants.h"

#ifndef DISTRIBUIDOS_MESSAGE_H
#define DISTRIBUIDOS_MESSAGE_H

typedef struct{
    int message_type;
    union {
        struct {
            char test_msg [MAX_LENGTH] ;
        } test;
    } message_choice;
} q_message;
#endif //DISTRIBUIDOS_MESSAGE_H
