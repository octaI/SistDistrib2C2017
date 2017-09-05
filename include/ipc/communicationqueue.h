#ifndef DISTRIBUIDOS_COMMUNICATIONQUEUE_H
#define DISTRIBUIDOS_COMMUNICATIONQUEUE_H

#include "../messages/message.h"
#include <string>

typedef struct {
    unsigned int id_firstqueue;
    unsigned int id_secondqueue;
    int orientation;
} commqueue;

commqueue create_commqueue(std::string filename, char key_char);

void send_message(commqueue channel, q_message output);

q_message receive_message(commqueue channel, long message_type );

void delete_queue(commqueue channel);


#endif //DISTRIBUIDOS_COMMUNICATIONQUEUE_H
