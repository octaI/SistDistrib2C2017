
#ifndef DISTRIBUIDOS_MESSAGE_H
#define DISTRIBUIDOS_MESSAGE_H

typedef struct{
    int message_type;
    union {
        //TODO: DEFINIR LOS UNIONS
    } message_choice;
} q_message;
#endif //DISTRIBUIDOS_MESSAGE_H
