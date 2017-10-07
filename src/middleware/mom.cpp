#include <ipc/communicationqueue.h>
#include <unistd.h>
#include <csignal>
#include <ipc/sharedmemory.h>
#include <ipc/semaphore.h>

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
} Mom;


/* IPC ACCESS */
int flag_update_read(Mom mom) {

    semaphore_wait(mom.update_flag_mutex);

    int value = static_cast<int*>(mom.update_flag)[mom.update_flag_position];

    semaphore_signal(mom.update_flag_mutex);

    return value;
}

void flag_update_write(Mom mom, int new_value) {

    semaphore_wait(mom.update_flag_mutex);

    static_cast<int*>(mom.update_flag)[mom.update_flag_position] = new_value;

    semaphore_signal(mom.update_flag_mutex);

}

void flag_update_increment(Mom mom) {

    semaphore_wait(mom.update_flag_mutex);

    static_cast<int*>(mom.update_flag)[mom.update_flag_position] = static_cast<int*>(mom.update_flag)[mom.update_flag_position] + 1;

    semaphore_signal(mom.update_flag_mutex);
}
/* END IPC ACCESS */

Mom start_mom() {
    commqueue client = create_commqueue(QUEUE_MOM_FILE, QUEUE_MOM_CHAR);
    client.orientation = COMMQUEUE_AS_SERVER;

    commqueue cinema_communication = create_commqueue(QUEUE_COMMUNICATION_FILE,QUEUE_COMMUNICATION_CHAR);
    cinema_communication.orientation = COMMQUEUE_AS_CLIENT;

    commqueue listener_communication = create_commqueue(QUEUE_CLIENT_FILE, QUEUE_CLIENT_CHAR);
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
    q_message connect_request = receive_message(mom.client_queue, TYPE_CONNECTON_MOM_REQUEST);
    int client_local_id = connect_request.message_choice.m1.client_id;
    mom.client_queue.id = client_local_id;

    q_message connect_response{};
    connect_response.message_choice_number = CHOICE_CONNECTION_ACCEPTED;
    send_message(mom.client_queue, connect_response);

    return client_local_id;
}


#define NO_SEAT_UPDATE 0

void start_async_seat_listener(Mom *mom) {
    pid_t listener = fork();
    if (listener == 0) {
        mom->listener_queue.orientation = COMMQUEUE_AS_SERVER;

        commqueue admin_communication = create_commqueue(QUEUE_ACTIVITY_FILE,QUEUE_ACTIVITY_CHAR);
        admin_communication.id = mom->client_cinema_id;
        admin_communication.orientation = COMMQUEUE_AS_CLIENT;

        flag_update_write(*mom, NO_SEAT_UPDATE);

        int close_listener = 0;
        while (close_listener == 0) {
            q_message seats_update = receive_message(admin_communication);
            printf("[CLIENT-D] An updated of seats detected (refresh to view)\n");
            if (seats_update.message_choice_number == CHOICE_SEATS_RESPONSE) {
                //1. Update shared memory
                flag_update_increment(*mom);
                //2. Send info to the father
                send_message(mom->listener_queue, seats_update);
            } else if (seats_update.message_choice_number == CHOICE_EXIT) {
                close_listener = 1;
            } else {
                printf("[CLIENT-D] Cannot read message\n");
            }
        }
        shm_dettach(mom->update_flag);
        exit(0);
    }
    mom->listener_pid = listener;
}

void finish_async_seat_listener(Mom *mom) {
    if (mom->listener_pid != LISTENER_NOT_INITIALIZED) {
        if ( kill(SIGKILL, mom->listener_pid) == -1) {
            printf("[CLIENT-MIDDLEWARE] Cannot finish listener. Wait to be finish with Admin\n");
        }
        mom->listener_pid = LISTENER_NOT_INITIALIZED;
    }
}


bool process_message(Mom *mom, q_message cinema_message) {
    bool return_value = false;
    int choice = cinema_message.message_choice_number;
    switch (choice) {
        case CHOICE_CONNECTION_ACCEPTED: {
            mom->client_cinema_id = cinema_message.message_choice.m1.client_id;
            mom->update_flag_position = mom->client_cinema_id % MAX_CLIENTS;
            break;
        }

        case CHOICE_SEAT_SELECT_REQUEST: {
            start_async_seat_listener(mom);
        }
        case CHOICE_SEAT_SELECT_RESPONSE: {
            if(cinema_message.message_choice.m6.success == SUCCESS) {
                finish_async_seat_listener(mom);
            }
            break;
        }

        case CHOICE_EXIT : {
            finish_async_seat_listener(mom);
            return_value = true;
        }
        default:
            break;
    }
    return return_value;
}

void fork_client(Mom mom, int client_local_id) {
    if (fork() != 0)
        return;

    mom.client_local_id = client_local_id;
    mom.client_queue.id = mom.client_local_id;
    bool exit_listener = false;
    while (!exit_listener) {
        q_message client_request = receive_message(mom.client_queue);
        send_message(mom.cinema_queue, client_request);

        q_message cinema_response = receive_message(mom.cinema_queue);
        send_message(mom.client_queue, cinema_response);

        exit_listener = process_message(&mom, cinema_response);
    }
    exit(0);
}

int main() {
    Mom mom = start_mom();
    while (true) {
        int client_local_id = connect_client(mom);
        fork_client(mom, client_local_id);
    }
}

