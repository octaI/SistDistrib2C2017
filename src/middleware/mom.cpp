#include <ipc/communicationqueue.h>
#include <unistd.h>
#include <csignal>
#include <ipc/sharedmemory.h>
#include <ipc/semaphore.h>
#include <constants.h>
#include <messages/message.h>
#include <cstring>
#include <network/network_comm.h>

#define UNDEFINED_CINEMA_CLIENT_ID (-1) //User not connected
#define UNDEFINED_LOCAL_CLIENT_ID (-1)
#define LISTENER_NOT_INITIALIZED 0

typedef struct {
    commqueue client_queue{};
    commqueue cinema_queue{};
    commqueue listener_queue{};

    void*   update_flag{};
    int     update_flag_position{};
    int     update_flag_mutex{};

    int     listener_pid{};

    int client_local_id{};
    int client_cinema_id{};
    network_comm net_info;
} Mom;


/* IPC ACCESS */
Seats_update flag_update_read(Mom mom) {

    semaphore_wait(mom.update_flag_mutex);

    Seats_update value = static_cast<Seats_update*>(mom.update_flag)[mom.update_flag_position];

    semaphore_signal(mom.update_flag_mutex);

    return value;
}

void flag_update_write(Mom mom, Seats_update update) {

    semaphore_wait(mom.update_flag_mutex);

    static_cast<Seats_update*>(mom.update_flag)[mom.update_flag_position] = update;

    semaphore_signal(mom.update_flag_mutex);

}
/* END IPC ACCESS */

Mom start_mom() {
    // MESSAGE FROM CLIENT INTERFACE
    commqueue client = create_commqueue(QUEUE_MOM_FILE, QUEUE_MOM_CHAR);
    client.orientation = COMMQUEUE_AS_SERVER;

    //MESSAGE TO CINEMA
    commqueue cinema_communication = create_commqueue(QUEUE_COMMUNICATION_FILE,QUEUE_COMMUNICATION_CHAR);
    cinema_communication.orientation = COMMQUEUE_AS_CLIENT;

    //MESSAGES FROM ADMIN
    commqueue listener_communication = create_commqueue(QUEUE_ACTIVITY_FILE, QUEUE_ACTIVITY_CHAR);
    listener_communication.orientation = COMMQUEUE_AS_CLIENT;

    int mutex_shared_memory = semaphore_get(MUTEX_CLIENT_FILE, MUTEX_CLIENT_CHAR);

    void* flag = shm_get_data(SHM_CLIENT_FILE, SHM_CLIENT_CHAR, SHM_CLIENT_SIZE);

    Mom mom{};
    mom.client_queue = client;
    mom.listener_queue = listener_communication;
    mom.cinema_queue = cinema_communication;

    mom.update_flag = flag;
    mom.update_flag_mutex = mutex_shared_memory;

    mom.client_cinema_id = UNDEFINED_CINEMA_CLIENT_ID;
    mom.client_local_id = UNDEFINED_LOCAL_CLIENT_ID;

    mom.listener_pid = LISTENER_NOT_INITIALIZED;


    return mom;
}


int connect_client(Mom mom) {
    q_message connect_request = receive_message(mom.client_queue, TYPE_CONNECTION_MOM_REQUEST);
    int client_local_id = connect_request.message_choice.m1.client_id;
    printf("[MOM] Client %d send to connect\n", client_local_id);
    mom.client_queue.id = client_local_id;

    q_message connect_response{};
    connect_response.message_choice_number = CHOICE_CONNECTION_ACCEPTED;
    connect_response.message_choice.m1.client_id = client_local_id % MAX_CLIENTS_HOST;
    send_message(mom.client_queue, connect_response);

    return client_local_id;
}


void start_async_seat_listener(Mom *mom) {
    if (mom->listener_pid != LISTENER_NOT_INITIALIZED) {
        return;
    }
    pid_t listener = fork();
    if (listener == 0) {
        printf("[MOM-LISTENER] Seat listener for CLIENT %d started with PID: %d\n", mom->client_local_id, getpid());

        Seats_update no_update{};
        no_update.update = NOT_SUCCESS;
        no_update.count = 0;

        flag_update_write(*mom, no_update);

        int close_listener = 0;
        while (close_listener == 0) {
            q_message seats_update = receive_message(mom->listener_queue);
            printf("[MOM-LISTENER] An updated of seats detected. Turn over Shared Memory in POS = %d\n", mom->update_flag_position);
            if (seats_update.message_choice_number == CHOICE_SEATS_RESPONSE) {
                //1. Update shared memory
                Seats_update update{};
                update.update = SUCCESS;
                update.count = seats_update.message_choice.m4.count;
                for (int i = 0; i < update.count; i++) {
                    Seat seat{};
                    seat.status = seats_update.message_choice.m4.seats_status[i];
                    seat.id = seats_update.message_choice.m4.seats_id[i];
                    update.seats[i] = seat;
                }
                flag_update_write(*mom, update);
            } else if (seats_update.message_choice_number == CHOICE_EXIT) {
                close_listener = 1;
            }
        }
        printf("[MOM-LISTENER] Seat listener for CLIENT %d with PID: %d terminated\n", mom->client_local_id, getpid());
        shm_dettach(mom->update_flag);
        exit(0);
    }
    mom->listener_pid = listener;
}

void finish_async_seat_listener(Mom *mom) {
    if (mom->listener_pid != LISTENER_NOT_INITIALIZED) {
        printf("[MOM] Attemp to kill listener with PID: %d\n",mom->listener_pid);
        if ( kill(SIGKILL, mom->listener_pid) == -1) {
            printf("[MOM] Cannot finish listener. Wait to be finish with Admin (%s)\n", strerror(errno));
        }
        mom->listener_pid = LISTENER_NOT_INITIALIZED;
    }
}


void process_message(Mom *mom, q_message cinema_message, bool *exit) {
    switch (cinema_message.message_choice_number) {
        case CHOICE_CONNECTION_ACCEPTED: {
            mom->client_cinema_id = cinema_message.message_choice.m1.client_id;
            mom->cinema_queue.id = mom->client_cinema_id;
            mom->listener_queue.id = mom->client_cinema_id;
            break;
        }

        case CHOICE_SEATS_RESPONSE: {
            start_async_seat_listener(mom);
            break;
        }
        case CHOICE_SEAT_SELECT_RESPONSE: {
            if(cinema_message.message_choice.m6.success == SUCCESS) {
                finish_async_seat_listener(mom);
            }
            break;
        }
        case CHOICE_ROOMS_RESPONSE:
            finish_async_seat_listener(mom);
            break;

        case CHOICE_EXIT : {
            finish_async_seat_listener(mom);
            *exit = true;
            break;
        }
        default:
            break;
    }
}

void fork_client(Mom mom, int client_local_id) {
    if (fork() != 0)
        return;

    mom.client_local_id = client_local_id;
    mom.client_queue.id = mom.client_local_id;
    mom.update_flag_position = client_local_id % MAX_CLIENTS_HOST;
    printf("[MOM] Connected and waiting request from client %d in PID: %d\n", client_local_id, (int)getpid());
    bool exit_listener = false;
    while (!exit_listener) {
        q_message client_request = receive_message(mom.client_queue);

        send_message(mom.cinema_queue, client_request);

        q_message cinema_response = (mom.client_cinema_id != UNDEFINED_CINEMA_CLIENT_ID) ?
                    receive_message(mom.cinema_queue) : receive_message(mom.cinema_queue, client_local_id);

        send_message(mom.client_queue, cinema_response);

        process_message(&mom, cinema_response, &exit_listener);

    }
    printf("[MOM] End connection with client %d in PID: %d\n", client_local_id, (int)getpid());
    exit(0);
}

int main() {
    Mom mom = start_mom();
    while (true) {
        printf("[MOM-SERVICE] Waiting for init_mom()\n");
        int client_local_id = connect_client(mom);
        fork_client(mom, client_local_id);
    }
}

