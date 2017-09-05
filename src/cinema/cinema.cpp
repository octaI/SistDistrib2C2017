#include <stdio.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>


int cinema_take_client() {
    //TAKE CLIENT FROM QUEUE
    pid_t pid;
    while ((pid = waitpid(-1, 0, WNOHANG)) > 1) {
        //Listener (pid) destroyed
    }
    return 0; //client_pid
}

void cinema_listen_client(unsigned int client_pid) {
    // TODO: set de id of client to send all message
    // TODO: timer
    bool exit = false;
    while (!exit) {
        //1 - Receive message from queue (client id)
        //  std::string request from queue (from client)

        //2 - Precess message
        //  std::message response = cinema_process_message(message)

        //2.1 -

        //3 - Send response
        //  queue send response to clien
    }
}

void cinema_start() {
    while (1) {
        unsigned int client_pid = (unsigned int)cinema_take_client();
        pid_t pid = fork();
        if (pid == 0) {
            cinema_listen_client(client_pid);
            exit(0);
        }
    }
}

int main(int argc, char **argv) {
    cinema_start();
    return 0;
}
