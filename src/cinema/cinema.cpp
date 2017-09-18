#include <cstdio>
#include <sys/msg.h>
#include <unistd.h>
#include <cstdlib>
#include <sys/wait.h>
#include <messages/message.h>
#include <ipc/communicationqueue.h>
#include <constants.h>
#include <cinema/cinema_handler.h>
#include <ipc/sharedmemory.h>

#define TIMER_CLOSE (-134)
#define MAX_TIME_TO_WAIT_SECONDS 100

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

void* start_timer(commqueue communication) {
    void* timer = shm_get_data(SHM_CINEMA_TIMER_FILE, SHM_CINEMA_TIMER_CHAR, SHM_CINEMA_TIMER_SIZE);
    *static_cast<int*>(timer) = (int)time(nullptr);

    printf("------- %d\n",*static_cast<int*>(timer));

    if (fork() == 0) {
        /* Set communication as client to simulate exit */
        communication.orientation = COMMQUEUE_AS_CLIENT;

        int stop = 0;
        while (stop == 0) {
            sleep(15);
            printf("[CINEMA-TIMER] [CLIENT %d] Checking timestamp\n", communication.id);
            int last_time_stamp = *static_cast<int*>(timer);
            auto timestamp = (int)time(nullptr);
            if (timestamp == TIMER_CLOSE) {
                printf("[CINEMA-TIMER] [CLIENT %d] Close Timer after cinema close\n", communication.id);
                stop = 1;
            }
            int difference = timestamp - last_time_stamp;
            if (difference > MAX_TIME_TO_WAIT_SECONDS) {
                printf("[CINEMA-TIMER] [CLIENT %d] Request (as Client) to close cinema after %d seconds without request\n", communication.id , difference);
                q_message exit_message{};
                exit_message.message_choice_number = CHOICE_EXIT;
                send_message(communication,exit_message);
                q_message confirm_exit = receive_message(communication);
                if (confirm_exit.message_choice_number == CHOICE_EXIT) {
                    stop = 1;
                } else {
                    exit(1);
                }
            } else {
                printf("[CINEMA-TIMER] [CLIENT %d] %d seconds of inactivity\n", communication.id, difference);
            }
        }
        shm_dettach(timer);
        exit(0);
    }
    return timer;
}

void cinema_listen_client(commqueue communication, int client_id) {
    // Attendant for a client. I'm going to tell the client that he's being served
    send_confirmation(communication,client_id);
    communication.id = client_id;

    void* timer = start_timer(communication);

    int exit = 0;
    while (exit == 0) {
        //1 - Receive message from queue (client id)
        q_message request = receive_message(communication);
        printf("[CINEMA] [RECEIVE MESSAGE FROM CLIENT %d] MSG_CHOICE: %d\n",client_id, (int)request.message_choice_number);

        //1-bis Update timer
        *static_cast<int*>(timer) = (int)time(nullptr);

        //2 - Process message
        //2.1 - Generate response after process
        q_message response = cinema_handle(request, &exit);

        //3 - Send response
        send_message(communication,response);
        printf("[CINEMA] [SENDED RESPONSE TO CLIENT %d] MSG_CHOICE: %d\n",communication.id,response.message_choice_number);
    }
    *static_cast<int*>(timer) = TIMER_CLOSE;
    shm_dettach(timer);
    printf("[CINEMA] Finish communication with CLIENT %d\n", client_id);
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
