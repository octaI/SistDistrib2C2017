#include <network/socket.h>


int create_sock_fd() {
    int ret_sockfd;
    ret_sockfd = socket(AF_INET,SOCK_STREAM,0);

    return ret_sockfd;
}

int connect_socket(int sock_fd,std::string ip_addr,unsigned short port) {
    sockaddr_in conn_info;
    inet_pton(AF_INET,ip_addr.c_str(),&conn_info.sin_addr);
    conn_info.sin_family = AF_INET;
    conn_info.sin_port = htons(port);
    memset(conn_info.sin_zero,0,sizeof(conn_info.sin_zero));

    return connect(sock_fd,(struct sockaddr*) &conn_info, sizeof(conn_info));
}

int bind_socket(int sock_fd, std::string ip_addr, unsigned short port, int size) {
    sockaddr_in server_info;
    inet_pton(AF_INET,ip_addr.c_str(),&server_info.sin_addr.s_addr);
    server_info.sin_port = htons(port);
    server_info.sin_family = AF_INET;

    return bind(sock_fd,(struct sockaddr*) &server_info, sizeof(server_info));
}

int listen_socket(int sock_fd) {
    return listen(sock_fd,MAX_BACKLOG_CONN);
}


int accept_connection(int sock_fd, sockaddr_in &client_info) {
    int val;
    socklen_t len = sizeof(val);
    int res =getsockopt(sock_fd,SOL_SOCKET,SO_ACCEPTCONN,&val,&len);
    if ( res == -1) {
        printf("Provided socket descriptor is not a socket. \n");
        return -1;
    } else if (!val) {
        printf("Provided socket is not in listening state. \n");
        return -1;
    }

    int c_length = sizeof(struct sockaddr_in);
    return accept(sock_fd,(struct sockaddr *) &client_info,(socklen_t*) &c_length);
}

int close_socket(int sock_fd) {
    return close(sock_fd);
}