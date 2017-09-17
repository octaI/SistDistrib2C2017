#include <stdio.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <messages/message.h>
#include <ipc/communicationqueue.h>
#include <constants.h>
#include <cinema/cinema_handler.h>
#include "../../include/ipc/communicationqueue.h"


int cinema_take_client(commqueue communication) {
    //TAKE CLIENT FROM QUEUE
    printf("[CINEMA] Waiting connection request\n");
    q_message message = receive_message(communication,TYPE_CONNECTION_REQUEST);
    //Trivial to know message.message_type (unique)
    int client_id = message.message_choice.m1.client_id;

    pid_t pid;
    while ((pid = waitpid(-1, 0, WNOHANG)) > 1) {
        //Listener (pid) destroyed
    }
    return client_id; //client_pid
}

void send_confirmation(commqueue communication, int client_id) {
    q_message confirmation{};
    confirmation.message_type = client_id;
    confirmation.message_choice_number = CHOICE_CONNECTION_ACCEPTED;
    send_message(communication,confirmation);
    printf("[CINEMA] Connected with CLIENT %d\n", client_id);
}

void cinema_listen_client(commqueue communication, int client_id) {
    // TODO: timer

    // Attendant for a client. I'm going to tell the client that he's being served
    send_confirmation(communication,client_id);
    communication.id = client_id;

    int exit = 0;
    while (exit == 0) {
        //1 - Receive message from queue (client id)
        //  std::string request from queue (from client)
        q_message request = receive_message(communication);
        printf("[CINEMA] [RECEIVE MESSAGE FROM CLIENT %d] MSG_CHOICE: %d\n",client_id, (int)request.message_choice_number);

        //2 - Process message
        //2.1 - Generate response after process
        q_message response = cinema_handle(request, &exit);

        //3 - Send response
        //  queue send response to clien
        send_message(communication,response);
        printf("[CINEAM] Response sended to CLIENT %d\n",communication.id);
    }
    printf("[CINEMA] Finish communication with CLIENT %d", client_id);
}

void cinema_start() {
    commqueue communication = create_commqueue(QUEUE_COMMUNICATION_FILE,QUEUE_COMMUNICATION_CHAR);
    communication.orientation = COMMQUEUE_AS_SERVER;
    printf("======= CINEMA ======\n");
    while (1) {
        int client_pid = cinema_take_client(communication);
        if (fork() == 0) {
            cinema_listen_client(communication,client_pid);
            exit(0);
        }
    }
}

int main(int argc, char **argv) {
    cinema_start();
    return 0;
}
