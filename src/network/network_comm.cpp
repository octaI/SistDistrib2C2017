#include <network/network_comm.h>

void network_newconn(network_comm &net_com, std::string ip_addr, unsigned short port) {
    net_com.port = port;
    net_com.ip_addr = ip_addr;
    net_com.sock_fd = create_sock_fd();
    return;
}

void network_prepare_accept(network_comm net_com) {
    bind_socket(net_com.sock_fd,net_com.ip_addr,net_com.port);
    listen_socket(net_com.sock_fd);
    return;
}

network_comm network_accept_connection(network_comm net_com) {
    network_comm client_data;
    char temp[INET_ADDRSTRLEN];
    sockaddr_in client_info_callback;
    client_data.sock_fd = accept_connection(net_com.sock_fd,client_info_callback);
    inet_ntop(client_info_callback.sin_family,&(client_info_callback.sin_addr.s_addr),temp,INET_ADDRSTRLEN);
    client_data.ip_addr = std::string(temp);
    client_data.port = client_info_callback.sin_port;

    return client_data;
}

void network_connect(network_comm net_com) {
    connect_socket(net_com.sock_fd,net_com.ip_addr,net_com.port);
    return;
}

void network_delete(network_comm net_com) {
    close_socket(net_com.sock_fd);
    return;
}

void network_send_data(network_comm net_com, q_message message_to_send) {
    send_packet(net_com.sock_fd,message_to_send);
}

void network_receive_data(network_comm net_com, q_message &received_msg) {
    receive_packet(net_com.sock_fd,received_msg);
}