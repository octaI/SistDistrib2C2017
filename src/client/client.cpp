#include "stdio.h"
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <messages/message.h>
#include <ipc/communicationqueue.h>
#include "../../include/ipc/communicationqueue.h"

int client_connect_to_cinema(commqueue communication) {
    auto client_id = (int)getpid();

    q_message request_connect{};
    request_connect.message_type = TYPE_CONNECTION_REQUEST;
    request_connect.message_choice.m1.client_id = client_id;

    send_message(communication,request_connect);
    printf("[CLIENT %d] Attemp to connect with cinema\n",client_id);

    receive_message(communication,client_id);
    printf("[CLIENT %d] Connection accepted\n",client_id);

    return client_id;
}

void client_start() {
    commqueue communication = create_commqueue(QUEUE_COMMUNICATION_FILE,QUEUE_COMMUNICATION_CHAR);
    communication.orientation = COMMQUEUE_AS_CLIENT;

    int client_id = client_connect_to_cinema(communication);
    communication.id = client_id;

    //1.1 request rooms
    //1.2 show rooms

    //2 select room

    //3.1 fork listener
    //3.2 show room seating information
    //3.3 option to see information and option to select
    //3.4 if see information refresh and show seeting information else request to select seat

    //4.0 If not succes goto 3
    //4.1 else show seating information and exit.

}


int main() {
    client_start();
}
