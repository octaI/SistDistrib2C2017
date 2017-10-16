#include <network/socket.h>
#include <messages/message.h>
#include <constants.h>



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

void serialize_chars (char*& destination, char* data, int size) {
     for(int i = 0; i < size; i++){
         *destination = data[i];
         destination++;
     }
}

void serialize_ints (int*& destination, int* data, int size) {
    for(int i = 0; i < size; i++){
        *destination = htonl((uint32_t )data[i]);
        destination++;
    }
}

void serialize_reservation(int*& destination, Reservation* data, int count) {
    /* |room | seat_num | */
    for(int i = 0; i < count; i++) {
        *destination = htonl(data[i].room);
        destination++;
        *destination = htonl(data[i].seat_num);
        destination++;
    }
}

void serialize_message(q_message sent_msg, char* data_to_serialize) {
    /* | size | clientid | msgchoicenum | DATA | */
    int s_clientid = htonl(sent_msg.client_id); //convert to network endianness
    char *data_as_char;
    int *data_as_int = (int*) data_to_serialize;
    *data_as_int = htonl(sizeof(q_message)); //first send msg size
    data_as_int++; //move pointer 4 bytes to next position
    *data_as_int = s_clientid; //write client id
    data_as_int++; //move pointer 4bytes
    *data_as_int = htonl(sent_msg.message_choice_number); //write msg choice num
    data_as_int++; //move pointer 4 bytes
    switch( sent_msg.message_choice_number) {
        case CHOICE_CONNECTION_ACCEPTED: {
            *data_as_int = htonl(sent_msg.message_choice.m1.client_id);
            break;
        }
        case CHOICE_ROOMS_REQUEST: {
            break;
        }

        case CHOICE_ROOMS_RESPONSE: {
            *data_as_int = htonl(sent_msg.message_choice.m2.count);
            data_as_int++;
            serialize_ints(data_as_int,sent_msg.message_choice.m2.ids,sent_msg.message_choice.m2.count);
            break;
        }

        case CHOICE_SEATS_REQUEST: {
            *data_as_int = htonl(sent_msg.message_choice.m3.room_id);
            break;
        }

        case CHOICE_SEATS_RESPONSE: {
            /* |success | count | {seat_ids} | {seat_status} | {information} |    */
            *data_as_int = htonl(sent_msg.message_choice.m4.success);
            data_as_int++;
            *data_as_int = htonl(sent_msg.message_choice.m4.count);
            data_as_int++;
            serialize_ints(data_as_int,sent_msg.message_choice.m4.seats_id,sent_msg.message_choice.m4.count);
            data_as_char = (char*) data_as_int;
            serialize_chars(data_as_char,sent_msg.message_choice.m4.seats_status,sent_msg.message_choice.m4.count);
            serialize_chars(data_as_char,sent_msg.message_choice.m4.information,MAX_LENGTH_STRING);
            break;
        }
        case CHOICE_SEAT_SELECT_REQUEST: {
            *data_as_int = htonl(sent_msg.message_choice.m5.seat_id);
            break;
        }

        case CHOICE_SEAT_SELECT_RESPONSE: case CHOICE_PAY_RESERVATION_RESPONSE:{
            /* |success | information | */
            *data_as_int = sent_msg.message_choice.m6.success;
            data_as_int++;
            data_as_char = (char*) data_as_int;
            serialize_chars(data_as_char,sent_msg.message_choice.m6.information,MAX_LENGTH_STRING);
            break;
        }

        case CHOICE_PAY_RESERVATION_REQUEST: {
            *data_as_int = htonl(sent_msg.message_choice.m7.count);
            data_as_int++;
            serialize_reservation(data_as_int,sent_msg.message_choice.m7.list,sent_msg.message_choice.m7.count);
            break;
        }


    }
}

void deserialize_ints(int *&data, int* destination, int count) {
    for (int i = 0; i < count; i++) {
        destination[i] = ntohl(*data);
        data++;
    }
}

void deserialize_chars(char *&data, char* destination, int count) {
    for (int i = 0; i < count; i++){
        destination[i] = *data;
        data++;
    }
}

void deserialize_reservations(int* &data, q_message &rec_msg, int count) {
    /* |room | seat_num | */
    for (int i = 0;i < count; i++ ) {
        Reservation new_reservation;
        new_reservation.room = ntohl(*data);
        data++;
        new_reservation.seat_num = ntohl(*data);
        data++;
        rec_msg.message_choice.m7.list[i] = new_reservation;
    }
}

void deserialize_message(q_message &rec_msg, char* data_to_deserialize) {
    int* data_as_int = (int*) data_to_deserialize;
    char* data_as_char;
    int size = ntohl(*data_as_int);
    data_as_int++;
    rec_msg.client_id = ntohl(*data_as_int);
    data_as_int++;
    rec_msg.message_choice_number = ntohl(*data_as_int);
    data_as_int++;

    switch (rec_msg.message_choice_number) {
        case CHOICE_CONNECTION_ACCEPTED :  {
            rec_msg.message_choice.m1.client_id = ntohl(*data_as_int);
            break;
        }

        case CHOICE_ROOMS_REQUEST : {
            break;
        }
        case CHOICE_ROOMS_RESPONSE : {
            rec_msg.message_choice.m2.count = ntohl(*data_as_int);
            data_as_int++;
            deserialize_ints(data_as_int, rec_msg.message_choice.m2.ids,rec_msg.message_choice.m2.count);
            break;
        }

        case CHOICE_SEATS_REQUEST : {
            rec_msg.message_choice.m3.room_id = ntohl(*data_as_int);
            break;
        }

        case CHOICE_SEATS_RESPONSE : {
            /* |success | count | {seat_ids} | {seat_status} | {information} |    */
            rec_msg.message_choice.m4.success = ntohl(*data_as_int);
            data_as_int++;
            rec_msg.message_choice.m4.count = ntohl(*data_as_int);
            data_as_int++;
            deserialize_ints(data_as_int,rec_msg.message_choice.m4.seats_id,rec_msg.message_choice.m4.count);
            data_as_char = (char*) data_as_int;
            deserialize_chars(data_as_char,rec_msg.message_choice.m4.seats_status,rec_msg.message_choice.m4.count);
            deserialize_chars(data_as_char,rec_msg.message_choice.m4.information,MAX_LENGTH_STRING);
            break;
        }

        case CHOICE_SEAT_SELECT_REQUEST: {
            rec_msg.message_choice.m5.seat_id = ntohl(*data_as_int);
            break;
        }

        case CHOICE_SEAT_SELECT_RESPONSE: case CHOICE_PAY_RESERVATION_RESPONSE: {
            /* |success | information | */
            rec_msg.message_choice.m6.success = ntohl(*data_as_int);
            data_as_int++;
            data_as_char = (char*) data_as_int;
            deserialize_chars(data_as_char,rec_msg.message_choice.m6.information,MAX_LENGTH_STRING);
            break;
        }
        case CHOICE_PAY_RESERVATION_REQUEST: {
            rec_msg.message_choice.m7.count = ntohl(*data_as_int);
            data_as_int++;
            deserialize_reservations(data_as_int,rec_msg,rec_msg.message_choice.m7.count);
            break;
        }
    }
}

