#include <stdio.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include "../../include/ipc/communicationqueue.h"
#include "../../include/constants.h"
#include "../../include/messages/message.h"


int cinema_take_client(commqueue communication) {
    //TAKE CLIENT FROM QUEUE
    q_message message = receive_message(communication,TYPE_CONNECTION_REQUEST);
    //Trivial to know message.message_type (unique)
    int client_id = message.message_choice.m1.client_id;
    pid_t pid;
    while ((pid = waitpid(-1, 0, WNOHANG)) > 1) {
        //Listener (pid) destroyed
    }
    return client_id; //client_pid
}

void cinema_listen_client(commqueue communication, int client_id) {
    // TODO: set de id of client to send all message
    // TODO: timer
    int exit = 0;
    while (exit == 0) {
        //1 - Receive message from queue (client id)
        //  std::string request from queue (from client)
        q_message request = receive_message(communication,client_id);

        //2 - Precess message
        //  std::message response = cinema_process_message(message)
        q_message response{};
        response.message_type = 0;
        m0_test dummy{};
        strcpy(dummy.test_msg,"Respuesta mock del servidor");
        response.message_choice.m0 = dummy;

        //2.1 -

        //3 - Send response
        //  queue send response to clien
        send_message(communication,response);
    }
}

void cinema_start() {
    commqueue communication = create_commqueue(QUEUE_COMMUNICATION_FILE,QUEUE_COMMUNICATION_CHAR);
    communication.orientation = COMMQUEUE_AS_SERVER;
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
