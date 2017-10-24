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
#include <network/network_comm.h>
#include <map>

#define TIMER_CLOSE (-134)
#define MAX_TIME_TO_WAIT_SECONDS 100
#define TIMER_POLLING 2
#define CONNECTION_TIMEDOUT -1

typedef struct {
    commqueue client_comm, admin_comm;
    network_comm cinema_net_info;

    int client_id;

    std::map<int,network_comm> client_address;

    int timer_mutex;
    int timer_position;
} Cinema;

void send_confirmation(Cinema cinema) {
    q_message confirmation{};
    confirmation.message_choice_number = CHOICE_CONNECTION_ACCEPTED;
    confirmation.message_choice.m1.client_id = cinema.client_id;
    send_message(cinema.client_comm,confirmation);
    printf("[CINEMA] Connected with CLIENT %d who send me with token %d\n", cinema.client_id, cinema.client_comm.id);
}

int cinema_take_client(Cinema cinema) {
    //TAKE CLIENT FROM QUEUE
    printf("[CINEMA] Waiting connection request\n");
    q_message message = receive_message(cinema.client_comm,TYPE_CONNECTION_CINEMA_REQUEST);
    int client_id = message.message_choice.m1.client_id;
    printf("[CINEMA] Receive CONNECTION_REQUEST from %d\n", client_id);
    cinema.client_comm.id = client_id;

    q_message register_client{};
    register_client.client_id = client_id;
    register_client.message_choice_number = CHOICE_CONNECTION_ACCEPTED;
    send_message(cinema.admin_comm, register_client);

    cinema.admin_comm.id = client_id;
    q_message register_response = receive_message(cinema.admin_comm);
    if (register_response.message_choice_number != CHOICE_CONNECTION_ACCEPTED) {
        THROW_UTIL("[CINEMA] Cannot register CLIENT " + std::to_string(client_id));
    }

    cinema.client_id = register_response.message_choice.m1.client_id;

    send_confirmation(cinema);

    pid_t pid;
    while ((pid = waitpid(-1, nullptr, WNOHANG)) > 1) {
        //Listener (pid) destroyed
        printf("[CINEMA] ** CHILD FINISHED **\n");
    }
    return cinema.client_id; //client_pid
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
                printf("[CINEMA-TIMER] [CLIENT %d] Close connection %d seconds of client inactivity\n", communication.id , difference);
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


void cinema_listen_client(Cinema cinema) {
    cinema.client_comm.id = cinema.client_id;

    printf("[CINEMA] [LISTENING TO CLIENT %d] \n",cinema.client_id);

    int mutex_shared_memory = semaphore_get(MUTEX_CINEMA_FILE, MUTEX_CINEMA_CHAR);

    int timer_pid = -1;
    void* timer = start_timer(cinema.client_comm, mutex_shared_memory, &timer_pid);
    printf("[CINEMA-CLIENT_%d] Timer Initialized with pid %d\n",cinema.client_id, timer_pid);
    int exit = 0;
    int clien_position = cinema.client_id % MAX_CLIENTS;
    while (exit == 0) {
        //1 - Receive message from queue (client id)
        q_message request = receive_message(cinema.client_comm);
        printf("[CINEMA] [RECEIVE MESSAGE FROM CLIENT %d] MSG_CHOICE: %d\n",cinema.client_id, (int)request.message_choice_number);

        //1-bis Update timer
        if (get_timestamp(timer,clien_position,mutex_shared_memory) != CONNECTION_TIMEDOUT)
            update_timestamp(timer, clien_position, mutex_shared_memory);

        //2 - Process message
        //2.1 - Generate response after process
        request.client_id = cinema.client_id;
        q_message response = cinema_handle(cinema.admin_comm, request, &exit);

        //3 - Send response
        printf("[CINEMA] [SENT RESPONSE TO CLIENT %d] MSG_CHOICE: %d\n",cinema.client_comm.id,response.message_choice_number);
        send_message(cinema.client_comm,response);
    }
    kill(timer_pid, SIGKILL);

    if (get_timestamp(timer,clien_position,mutex_shared_memory) == CONNECTION_TIMEDOUT) {
        // Cinema close the connection. If client send a message, receive it and finish
        printf("[CINEMA] Wait a message from CLIENT %d after close\n",cinema.client_id);
        receive_message(cinema.client_comm);
    } else {
        // Client finish the connection. Close timer if it wasn't killed
        update_timestamp(timer, clien_position, mutex_shared_memory, TIMER_CLOSE);
    }
    shm_dettach(timer);
    printf("[CINEMA] Finish client_communication with CLIENT %d\n", cinema.client_id);
}

void network_listen(Cinema cinema) {
    if (fork() == 0) {
        Accept_Connection:
        printf("[CINEMA-NETWORK] Listen connections in %s:%d\n",CINEMA_IP_ADDR,CINEMA_PORT);
        network_comm accept_fd = network_accept_connection(cinema.cinema_net_info);
        if (fork() == 0) {
            cinema.client_comm.orientation = COMMQUEUE_AS_CLIENT;
            cinema.client_comm.id = -1;
            while (true) {
                q_message msg_to_receive{},msg_to_send{};

                receive_packet(accept_fd.sock_fd,msg_to_receive);

                send_message(cinema.client_comm,msg_to_receive);

                int type = (msg_to_receive.message_type == TYPE_CONNECTION_CINEMA_REQUEST) ? msg_to_receive.message_choice.m1.client_id : msg_to_receive.message_type;
                msg_to_send = receive_message(cinema.client_comm, type);

                send_packet(accept_fd.sock_fd,msg_to_send);

            }
        }
        goto Accept_Connection;
    }
}

void cinema_start() {
    Cinema cinema;
    /* Create comunication with client */
    cinema.client_comm = create_commqueue(QUEUE_COMMUNICATION_FILE,QUEUE_COMMUNICATION_CHAR);
    cinema.client_comm.orientation = COMMQUEUE_AS_SERVER;

    /* Create communication with admin */
    cinema.admin_comm = create_commqueue(QUEUE_CINEMA_ADMIN_FILE, QUEUE_CINEMA_ADMIN_CHAR);
    cinema.admin_comm.orientation = COMMQUEUE_AS_CLIENT;
    cinema.admin_comm.id = ADMIN_REQUEST;

    /*Setup network component*/
    network_newconn(cinema.cinema_net_info,CINEMA_IP_ADDR,CINEMA_PORT);
    network_prepare_accept(cinema.cinema_net_info);
    network_listen(cinema);
    printf("======= CINEMA ======\n");
    while (1) {
        int client_id = cinema_take_client(cinema);
        if (fork() == 0) {
            cinema.client_id = client_id;
            cinema_listen_client(cinema);
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
