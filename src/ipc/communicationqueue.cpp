#include "../../include/ipc/communicationqueue.h"
#include "../../include/ipc/queue.h"

#include <string>

commqueue create_commqueue(std::string filename, char key_char) {
    commqueue new_queue;
    new_queue.id_firstqueue = (unsigned int)queue_create(filename,key_char);
    new_queue.id_secondqueue = (unsigned int)queue_create(filename,key_char+1);
    return new_queue;
}

void send_message(commqueue channel, q_message input) {
    int queue_id = (channel.orientation == 0) ? channel.id_firstqueue : channel.id_secondqueue;
    queue_send(queue_id,(void*) &input,sizeof(input),input.message_type);
}

q_message receive_message(commqueue channel, long message_type) {
    int queue_id = (channel.orientation == 0) ? channel.id_secondqueue : channel.id_firstqueue;
    q_message received_msg;
    queue_receive(queue_id,(void*) &received_msg,sizeof(q_message),message_type);
    return received_msg;
}

void delete_queue(commqueue channel) {
    queue_destroy(channel.id_firstqueue);
    queue_destroy(channel.id_secondqueue);
}

void set_channel_orientation(commqueue channel, int FLAG) {
    channel.orientation = FLAG;
}