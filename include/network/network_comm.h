

#ifndef DISTRIBUIDOS_NETWORK_COMM_H
#define DISTRIBUIDOS_NETWORK_COMM_H

#include <string>
#include <network/socket.h>

typedef struct {
    int sock_fd;
    std::string ip_addr;
    unsigned short port;
}network_comm;

void network_newconn(network_comm &net_com, std::string ip_addr, unsigned short port);

void network_prepare_accept(network_comm net_com);

network_comm network_accept_connection(network_comm net_com);

void network_connect(network_comm net_com);

void network_delete(network_comm net_com);

void network_send_data(network_comm net_com, q_message message_to_send);

void network_receive_data(network_comm net_com, q_message &received_msg);

#endif //DISTRIBUIDOS_NETWORK_COMM_H
