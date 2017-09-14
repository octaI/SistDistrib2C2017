#ifndef DISTRIBUIDOS_COMMUNICATIONQUEUE_H
#define DISTRIBUIDOS_COMMUNICATIONQUEUE_H

#include "../messages/message.h"
#include <string>

#define COMMQUEUE_AS_SERVER 0;
#define COMMQUEUE_AS_CLIENT 1;

typedef struct {
    unsigned int id_firstqueue;
    unsigned int id_secondqueue;
    int orientation;
    int id;
} commqueue;

commqueue create_commqueue(std::string filename, char key_char);

void send_message(commqueue channel, q_message message);

q_message receive_message(commqueue channel, int m_type);

q_message receive_message(commqueue channel);

void delete_queue(commqueue channel);


#endif //DISTRIBUIDOS_COMMUNICATIONQUEUE_H
