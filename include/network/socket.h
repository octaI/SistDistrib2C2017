
#ifndef DISTRIBUIDOS_SOCKET_H
#define DISTRIBUIDOS_SOCKET_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <constants.h>
#include <messages/message.h>


int create_sock_fd();

int connect_socket(int sock_fd,std::string ip_addr,unsigned short port);

int bind_socket(int sock_fd, std::string ip_addr, unsigned short port, int size);

int accept_connection(int sock_fd, sockaddr_in &client_info);

int listen_socket(int sock_fd);

int close_socket(int sock_fd);

int send_packet(int sock_fd, q_message sent_msg);

void serialize_message(q_message sent_msg, char* data_to_serialize);

void deserialize_message(q_message &rec_msg, char* data_to_deserialize);


#endif //DISTRIBUIDOS_SOCKET_H
