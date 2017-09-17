#include <sys/unistd.h>
#include <iostream>
#include <cstring>
#include "../../include/ipc/communicationqueue.h"
#include "../../include/constants.h"
#include "../../include/messages/message.h"

void admin_listen_requests(commqueue channel){
    q_message client_req = receive_message(channel,ADMIN_REQUEST);
    //received a request
    if (fork() == 0) { //forking to handle request
        //admin_handle_request(q_message)
        q_message mock_response;
        mock_response.client_id = client_req.client_id;
        strcpy(mock_response.message_choice.m0.test_msg,"This is a test");
        send_message(channel,mock_response);
        exit(0);
    }

}

void admin_daemon(){
    std::cout << "INITIATED ADMIN" << std::endl;
    commqueue admin_channel = create_commqueue(QUEUE_CINEMA_ADMIN_FILE,QUEUE_CINEMA_ADMIN_CHAR);
    admin_channel.orientation = ADMIN_TO_CINEMA;

    while(true){
        admin_listen_requests(admin_channel);
    }
}

int main(){
    admin_daemon();
    return 0;
}