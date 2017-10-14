
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


int create_sock_fd();

int connect_socket(int sock_fd,std::string ip_addr,unsigned short port);

int bind_socket(int sock_fd, std::string ip_addr, unsigned short port, int size);

int accept_connection(int sock_fd, sockaddr_in &client_info);

int listen_socket(int sock_fd);

int close_socket(int sock_fd);




#endif //DISTRIBUIDOS_SOCKET_H
