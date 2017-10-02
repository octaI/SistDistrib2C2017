#include <middleware/client.h>
#include <ipc/communicationqueue.h>
#include <map>
#include <ipc/semaphore.h>
#include <ipc/sharedmemory.h>
#include <unistd.h>
#include <signal.h>
#include <constants.h>

#define UNDEFINED_CLIENT_ID -404
#define LISTENER_NOT_INITIALIZED -1

typedef struct {
    void* update_flag{};
    int update_flag_position{};
    int mutex_flag_id{};
    int client_id{};
    commqueue cinema_communication{};
    commqueue update_communication{};
    std::vector<reservation> reservations;
    pid_t async_listener{};
} Client;

std::map<int, Client> CLIENTS;

int _get_key() {
    return getpid();
}

Client* _getClient(bool checkConnection = true) {
    int client_key = _get_key();
    if ( CLIENTS.find(client_key) == CLIENTS.end() ) {
        // not found
        printf("[CLIENT-MIDDLEWAR] MOM not initialized\n");
        return nullptr;
    }
    Client* client = &CLIENTS[_get_key()];
    if (checkConnection && client->client_id == UNDEFINED_CLIENT_ID) {
        printf("[CLIENT-MIDDLEWAR] Client not connected with cinema\n");
        return nullptr;
    }
    return client;

}

void start_async_seat_listener() {
    Client* client = _getClient();
    if (client == nullptr) {
        return;
    }
    pid_t listener = fork();
    if (listener == 0) {
        client->update_communication.orientation = COMMQUEUE_AS_SERVER;

        commqueue admin_communication = create_commqueue(QUEUE_ACTIVITY_FILE,QUEUE_ACTIVITY_CHAR);
        admin_communication.id = client->client_id;
        admin_communication.orientation = COMMQUEUE_AS_CLIENT;

        //client_write_flag_update(client, NO_SEAT_UPDATE);

        int close_listener = 0;
        while (close_listener == 0) {
            q_message seats_update = receive_message(admin_communication);
            printf("[CLIENT-D] An updated of seats detected (refresh to view)\n");
            if (seats_update.message_choice_number == CHOICE_SEATS_RESPONSE) {
                //1. Update shared memory
                //client_add_update(client);
                //2. Send info to the father
                send_message(client->update_communication, seats_update);
            } else if (seats_update.message_choice_number == CHOICE_EXIT) {
                close_listener = 1;
            } else {
                printf("[CLIENT-D] Cannot read message\n");
            }
        }
        shm_dettach(client->update_flag);
        exit(0);
    }
    client->async_listener = listener;
}

void finish_async_seat_listener() {
    Client* client = _getClient();
    if (client != nullptr && client->async_listener != LISTENER_NOT_INITIALIZED) {
        if ( kill(SIGKILL, client->async_listener) != 0) {
            printf("[CLIENT-MIDDLEWARE] Canot finish litener. Wait to be finish with Admin");
        }
        client->async_listener = LISTENER_NOT_INITIALIZED;
    }
}

int _connect_to_cinema(commqueue cinema_comunication) {
    auto client_id = (int)getpid();

    q_message request_connect{};
    request_connect.message_type = TYPE_CONNECTION_REQUEST;
    request_connect.message_choice.m1.client_id = client_id;

    send_message(cinema_comunication,request_connect);
    printf("[CLIENT-MIDDLEWARE %d] Attemp to connect with cinema\n",client_id);

    cinema_comunication.id = client_id;
    q_message response = receive_message(cinema_comunication);

    if (response.message_choice_number == CHOICE_CONNECTION_ACCEPTED) {
        client_id = response.message_choice.m1.client_id;
        printf("[CLIENT-MIDDLEWARE %d] Connection accepted\n",client_id);
    }

    return client_id;
}

int init_mom() {
    commqueue cinema_communication = create_commqueue(QUEUE_COMMUNICATION_FILE,QUEUE_COMMUNICATION_CHAR);
    cinema_communication.orientation = COMMQUEUE_AS_CLIENT;

    commqueue client_communication = create_commqueue(QUEUE_CLIENT_FILE, QUEUE_CLIENT_CHAR);
    client_communication.orientation = COMMQUEUE_AS_CLIENT;

    int mutex_shared_memory = semaphore_get(MUTEX_CLIENT_FILE, MUTEX_CLIENT_CHAR);

    void* flag = shm_get_data(SHM_CLIENT_FILE, SHM_CLIENT_CHAR, SHM_CLIENT_SIZE);

    Client client;
    client.cinema_communication =   cinema_communication;
    client.update_communication =   client_communication;
    client.client_id =              UNDEFINED_CLIENT_ID;
    client.mutex_flag_id =          mutex_shared_memory;
    client.update_flag =            flag;
    client.update_flag_position =   0;
    client.async_listener =         LISTENER_NOT_INITIALIZED;

    CLIENTS[_get_key()] = client;
}

bool login() {
    Client* client = _getClient(false);

    client->client_id = _connect_to_cinema(client->cinema_communication);

    client->cinema_communication.id = client->client_id;
    client->update_communication.id = client->client_id;

    client->update_flag_position = client->client_id % MAX_CLIENTS;

    return true;

}

std::vector<room> get_rooms() {
    Client* client = _getClient();
    std::vector<room> rooms;
    if (client == nullptr) {
        return rooms;
    }

    q_message rooms_request{};
    rooms_request.message_choice_number = CHOICE_ROOMS_REQUEST;
    send_message(client->cinema_communication,rooms_request);

    //1.2 show rooms
    q_message rooms_response = receive_message(client->cinema_communication);
    if ( rooms_response.message_choice_number != CHOICE_ROOMS_RESPONSE ) {
        if (rooms_response.message_choice_number == CHOICE_EXIT) {
            //TODO: Close client
            //update_seat_from_client(client, false);
            printf("[CLIENT-MIDDLEWARE] Cinema close connection\n");
            return rooms;
        }
        printf("[CLIENT] Unexpected Error after CHOICE_ROOMS_REQUEST\n");
        return rooms;
    }
    for (int i = 0; i < rooms_response.message_choice.m2.count; i++) {
        room aRoom{};
        aRoom.id = rooms_response.message_choice.m2.ids[i];
        rooms.push_back(aRoom);
    }
    return rooms;
}

std::vector<seat> get_seats(room aRoom) {
    Client* client = _getClient();
    std::vector<seat> seats;
    if (client == nullptr) {
        return seats;
    }

    q_message select_room_message{};
    select_room_message.message_choice_number = CHOICE_SEATS_REQUEST;
    select_room_message.message_choice.m3.room_id = aRoom.id;
    send_message(client->cinema_communication,select_room_message);

    q_message room_information = receive_message(client->cinema_communication);
    if (room_information.message_choice_number != CHOICE_SEATS_RESPONSE) {
        if (room_information.message_choice_number == CHOICE_EXIT) {
            //TODO: Close client
            //update_seat_from_client(client, false);
            //printf("[CLIENT] Cinema close connection\n");
            return seats;
        }
        printf("[CLIENT] Unexpected Error after CHOICE_SEATS_REQUEST\n");
        return seats;
    } else {
        if (room_information.message_choice.m4.success == NOT_SUCCESS) {
            printf("Not succes after select room: \"%s\"\n", room_information.message_choice.m4.information);
            return seats;
        }
    }

    //3.1 fork listener
    start_async_seat_listener();

    return seats;
}

bool reserve_seat(seat aSeat) {
    Client* client = _getClient();
    if (client == nullptr) {
        return false;
    }
    q_message seat_select{};
    seat_select.message_choice_number = CHOICE_SEAT_SELECT_REQUEST;
    seat_select.message_choice.m5.seat_id = aSeat.id;
    send_message(client->cinema_communication, seat_select);

    //4.0 If not succes goto 3
    q_message seat_select_response = receive_message(client->cinema_communication);
    if (seat_select_response.message_choice_number != CHOICE_SEAT_SELECT_RESPONSE) {
        if (seat_select_response.message_choice_number == CHOICE_EXIT) {

            //update_seat_from_client(client, false);
            //printf("[CLIENT] Cinema close connection\n");
            return false;
        }
        printf("[CLIENT-MIDDLEWARE] Unexpected error when select seat\n");
        return false;
    }
    if (seat_select_response.message_choice.m6.success != SUCCESS) {
        char* information = seat_select_response.message_choice.m6.information;
        printf("[CLIENT-MIDDLEWARE] Error on select seat: %s\n",information);
        return false;
    }

    // Kill listener
    finish_async_seat_listener();

    //4.1 else show seating information and exit.
    printf("[CLIENT-MIDDLEWARE] SUCCESS: Reserved SEAT %d in ROOM %d\n",aSeat.id, aSeat.room_id);
    reservation reservation{};
    reservation.room = aSeat.room_id;
    reservation.seat_num = aSeat.id;
    client->reservations.push_back(reservation);
    return true;
}

std::vector<reservation> get_reservations() {
    Client* client = _getClient();
    std::vector<reservation> reservations;
    if (client == nullptr) {
        return reservations;
    }
    reservations = client->reservations;
    return reservations;
}


bool pay_seats() {
    Client* client = _getClient();
    if (client == nullptr) {
        return false;
    }
    q_message request{};
    request.client_id = client->client_id;
    request.message_choice_number = CHOICE_PAY_RESERVATION_REQUEST;
    request.message_choice.m7.count = (int)client->reservations.size();
    std::copy(client->reservations.begin(),client->reservations.end(),request.message_choice.m7.list);
    printf("Sending payment, please wait...\n");
    for (auto reservation : client->reservations) {
        printf("(OK) SEAT: %d - ROOM: %d\n",reservation.seat_num,reservation.room);
        sleep(1);
    }
    send_message(client->cinema_communication, request);
    q_message response = receive_message(client->cinema_communication);
    if (response.message_choice_number != CHOICE_PAY_RESERVATION_RESPONSE) {
        return false;
    }
    if (response.message_choice.m6.success == NOT_SUCCESS) {
        char *information = response.message_choice.m6.information;
        printf("[CLIENT-MIDDLEWARE]Error on Buy: %s\n", information);
        return false;
    }
    return true;
}