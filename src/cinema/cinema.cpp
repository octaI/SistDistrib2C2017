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
#include <admin/admin.h>
#include <iostream>
#include <ipc/semaphore.h>

#define TIMER_CLOSE (-134)
#define MAX_TIME_TO_WAIT_SECONDS 100
#define TIMER_POLLING 2
#define CONNECTION_TIMEDOUT -1


void send_confirmation(commqueue communication, int client_id) {
    q_message confirmation{};
    confirmation.message_choice_number = CHOICE_CONNECTION_ACCEPTED;
    confirmation.message_choice.m1.client_id = client_id;
    send_message(communication,confirmation);
    printf("[CINEMA] Connected with CLIENT %d who send me with token %d\n", client_id, communication.id);
}

int cinema_take_client(commqueue client_communication, commqueue admin_communication) {
    //TAKE CLIENT FROM QUEUE
    printf("[CINEMA] Waiting connection request\n");
    q_message message = receive_message(client_communication,TYPE_CONNECTION_CINEMA_REQUEST);
    int client_id = message.message_choice.m1.client_id;
    client_communication.id = client_id;

    q_message register_client{};
    register_client.client_id = client_id;
    register_client.message_choice_number = CHOICE_CONNECTION_ACCEPTED;
    send_message(admin_communication, register_client);

    admin_communication.id = client_id;
    q_message register_response = receive_message(admin_communication);
    if (register_response.message_choice_number != CHOICE_CONNECTION_ACCEPTED) {
        THROW_UTIL("[CINEMA] Cannot register CLIENT " + std::to_string(client_id));
    }
    client_id = register_response.message_choice.m1.client_id;

    send_confirmation(client_communication, client_id);

    pid_t pid;
    while ((pid = waitpid(-1, nullptr, WNOHANG)) > 1) {
        //Listener (pid) destroyed
        printf("[CINEMA] ** CHILD FINISHED **\n");
    }
    return client_id; //client_pid
}

int update_timestamp(void* timer, int client_position, int mutex_id, int new_value = (int)time(nullptr)) {
    semaphore_wait(mutex_id);
    static_cast<int*>(timer)[client_position] = new_value;
    semaphore_signal(mutex_id);
}

int get_timestamp(void* timer, int client_position, int mutex_id) {
    semaphore_wait(mutex_id);
    int timestamp = static_cast<int*>(timer)[client_position];
    semaphore_signal(mutex_id);
    return timestamp;
}

void* start_timer(commqueue communication, int mutex_id, int *timer_pid) {

    void* timer = shm_get_data(SHM_CINEMA_TIMER_FILE, SHM_CINEMA_TIMER_CHAR, SHM_CINEMA_TIMER_SIZE);
    int client_position = communication.id % MAX_CLIENTS;

    update_timestamp(timer,client_position, mutex_id);

    pid_t timer_fork = fork();
    if (timer_fork == 0) {
        /* Set communication as client to simulate exit */
        communication.orientation = COMMQUEUE_AS_CLIENT;

        int stop = 0;
        while (stop == 0) {
            sleep(TIMER_POLLING);
            int last_time_stamp = get_timestamp(timer,client_position,mutex_id);
            auto timestamp = (int)time(nullptr);
            if (timestamp == TIMER_CLOSE) {
                printf("[CINEMA-TIMER] [CLIENT %d] Close Timer after cinema close\n", communication.id);
                stop = 1;
                continue;
            }
            int difference = timestamp - last_time_stamp;
            if (difference > MAX_TIME_TO_WAIT_SECONDS) {
                printf("[CINEMA-TIMER] [CLIENT %d] Close connectio %d seconds of client inactivity\n", communication.id , difference);
                q_message exit_message{};
                exit_message.message_choice_number = CHOICE_EXIT;
                // Send message "as client" to cinema to close the connection
                send_message(communication,exit_message);
                update_timestamp(timer,client_position,mutex_id, CONNECTION_TIMEDOUT);
                stop = 1;
            } else {
                printf("[CINEMA-TIMER] [CLIENT %d] %d seconds of inactivity\n", communication.id, difference);
            }
        }
        shm_dettach(timer);
        exit(0);
    }
    *timer_pid = (int)timer_fork;
    return timer;
}


void cinema_listen_client(commqueue client_communication, commqueue admin_communication, int client_id) {
    client_communication.id = client_id;

    int mutex_shared_memory = semaphore_get(MUTEX_CINEMA_FILE, MUTEX_CINEMA_CHAR);

    int timer_pid = -1;
    void* timer = start_timer(client_communication, mutex_shared_memory, &timer_pid);
    printf("[CINEMA-CLIENT_%d] Timer Initialized with pid %d\n",client_id, timer_pid);
    int exit = 0;
    int clien_position = client_id % MAX_CLIENTS;
    while (exit == 0) {
        //1 - Receive message from queue (client id)
        q_message request = receive_message(client_communication);
        printf("[CINEMA] [RECEIVE MESSAGE FROM CLIENT %d] MSG_CHOICE: %d\n",client_id, (int)request.message_choice_number);

        //1-bis Update timer
        if (get_timestamp(timer,clien_position,mutex_shared_memory) != CONNECTION_TIMEDOUT)
            update_timestamp(timer, clien_position, mutex_shared_memory);

        //2 - Process message
        //2.1 - Generate response after process
        request.client_id = client_id;
        q_message response = cinema_handle(admin_communication, request, &exit);

        //3 - Send response
        send_message(client_communication,response);
        printf("[CINEMA] [SENT RESPONSE TO CLIENT %d] MSG_CHOICE: %d\n",client_communication.id,response.message_choice_number);
    }
    kill(timer_pid, SIGKILL);

    if (get_timestamp(timer,clien_position,mutex_shared_memory) == CONNECTION_TIMEDOUT) {
        // Cinema close the connection. If client send a message, receive it and finish
        printf("[CINEMA] Wait a message from CLIENT %d after close\n",client_id);
        receive_message(client_communication);
    } else {
        // Client finish the connection. Close timer if it wasn't killed
        update_timestamp(timer, clien_position, mutex_shared_memory, TIMER_CLOSE);
    }
    shm_dettach(timer);
    printf("[CINEMA] Finish client_communication with CLIENT %d\n", client_id);
}

void cinema_start() {
    /* Create comunication with client */
    commqueue client_communication = create_commqueue(QUEUE_COMMUNICATION_FILE,QUEUE_COMMUNICATION_CHAR);
    client_communication.orientation = COMMQUEUE_AS_SERVER;
    /* Create communication with admin */
    commqueue admin_communication = create_commqueue(QUEUE_CINEMA_ADMIN_FILE, QUEUE_CINEMA_ADMIN_CHAR);
    admin_communication.orientation = COMMQUEUE_AS_CLIENT;
    admin_communication.id = ADMIN_REQUEST;

    printf("======= CINEMA ======\n");
    while (1) {
        int client_pid = cinema_take_client(client_communication, admin_communication);
        if (fork() == 0) {
            cinema_listen_client(client_communication, admin_communication, client_pid);
            exit(0);
        }
    }
}

void admin_start() {
    if (fork() == 0) {
        admin_daemon();
        exit(0);
    }
}

int main(int argc, char **argv) {
    admin_start();
    cinema_start();
    return 0;
}
