#include <middleware/client_interface.h>
#include <ipc/communicationqueue.h>
#include <map>
#include <unistd.h>
#include <csignal>
#include <ipc/semaphore.h>
#include <ipc/sharedmemory.h>
#include <pthread.h>
#include <constants.h>


#define ERROR_NO_ERROR 0
#define ERROR_CINEMA_CLOSE_CONNECTION 1
#define ERROR_CANNOT_CONNECT_TO_CINEMA 2
#define ERROR_ON_DISCONNECT_CINEMA 3
#define ERROR_GENERAL_ERROR 4
#define ERROR_MOM_ALREADY_INIT 5
#define ERROR_MOM_NOT_INITIALIZED 6

#define UNDEFINED_ID (-123)


typedef struct {
    commqueue                   mom_queue{};

    pthread_t                   listener_thread{};
    void*                       listener_updates{};
    int                         listener_updates_mutex{};
    int                         listener_updates_position{};
    on_seat_update_data*        listener_extern_data{};

    std::vector<Reservation>    reservations{};

    int                         local_id{};
    int                         cinema_id{};

    bool                        connected{};

    MomError                    error{};
} Client;

std::map<int, Client> CLIENTS;

int _get_local_id() {
    //Local Client ID for middleware
    return getpid();
}

Client* _getClient(int client_key, bool checkConnection = true, bool checkExists = true) {
    if ( CLIENTS.find(client_key) == CLIENTS.end() ) {
        // not found
        if (checkExists)
            printf("[CLIENT-INTERFACE] MOM not initialized\n");
        return nullptr;
    }
    Client* client = &CLIENTS[client_key];
    if (checkConnection && !client->connected) {
        printf("[CLIENT-INTERFACE] Client not connected with cinema\n");
        return nullptr;
    }
    return client;
}

void _clean_error(Client* client) {
    if (client != nullptr) {
        client->error.type = ERROR_NO_ERROR;
        client->error.info = "";
    }
}


int _connect_to_cinema(Client client) {
    int client_token = client.local_id;

    q_message request_connect_to_cinema{};
    request_connect_to_cinema.message_type = TYPE_CONNECTION_CINEMA_REQUEST;
    request_connect_to_cinema.message_choice.m1.client_id = client_token;

    send_message(client.mom_queue,request_connect_to_cinema);
    printf("[CLIENT-INTERFACE token: %d] Attemp to connect with cinema\n",client_token);

    q_message response = receive_message(client.mom_queue);
    int client_cinema_id;
    if (response.message_choice_number == CHOICE_CONNECTION_ACCEPTED) {
        client_cinema_id = response.message_choice.m1.client_id;
        printf("[CLIENT-INTERFACE token: %d] Connection accepted. cinema_id: %d\n",client_token,client_cinema_id);
    } else {
        client_cinema_id = UNDEFINED_ID;
    }

    return client_cinema_id;
}

int init_mom() {
    int client_local_id = _get_local_id();
    Client* client_exists = _getClient(client_local_id, false, false);
    if (client_exists != nullptr) {
        client_exists->error.type = ERROR_MOM_ALREADY_INIT;
        client_exists->error.info = "Error. MOM is already initialized";
        return client_local_id;
    }

    commqueue mom_queue = create_commqueue(QUEUE_MOM_FILE, QUEUE_MOM_CHAR);
    mom_queue.orientation = COMMQUEUE_AS_CLIENT;

    int listener_updates_mutex = semaphore_get(MUTEX_CLIENT_FILE, MUTEX_CLIENT_CHAR);

    void* listener_updates = shm_get_data(SHM_CLIENT_FILE, SHM_CLIENT_CHAR, SHM_CLIENT_SIZE);

    Client client;
    client.mom_queue =              mom_queue;
    client.connected =              false;
    client.error.info =             "";
    client.error.type =             ERROR_NO_ERROR;
    client.local_id =               client_local_id;
    client.cinema_id =              UNDEFINED_ID;
    client.listener_updates =       listener_updates;
    client.listener_updates_mutex = listener_updates_mutex;
    client.listener_extern_data =   nullptr;


    //Attemp to connect with mom
    q_message connection_mom_request{};
    connection_mom_request.message_type = TYPE_CONNECTION_MOM_REQUEST;
    connection_mom_request.message_choice.m1.client_id = client.local_id;
    send_message(mom_queue, connection_mom_request);

    mom_queue.id = client_local_id;
    q_message connection_mom_response = receive_message(mom_queue);
    if (connection_mom_response.message_choice_number == CHOICE_CONNECTION_ACCEPTED) {
        client.connected = true;
        client.mom_queue.id = client.local_id;
        client.listener_updates_position = connection_mom_response.message_choice.m1.client_id;
        CLIENTS[client.local_id] = client;
    }

    return client.local_id;
}

bool login(int client_fd) {
    Client* client = _getClient(client_fd,false);
    _clean_error(client);

    client->cinema_id = _connect_to_cinema(*client);

    if (client->cinema_id == UNDEFINED_ID) {
        client->error.type = ERROR_CANNOT_CONNECT_TO_CINEMA;
        client->error.info = "Cannot connect with cinema";
        return false;
    }

    client->connected = true;

    return true;

}


void _end_listener_thread(Client* client) {
    if (pthread_cancel(client->listener_thread) == 0) {
        client->listener_extern_data = nullptr;
    }
}

bool _disconnect(Client* client, bool send = true) {
    _end_listener_thread(client);
    if (send) {
        q_message exit{};
        exit.message_choice_number = CHOICE_EXIT;
        send_message(client->mom_queue,exit);
        q_message exit_from_cinema = receive_message(client->mom_queue);
        if (exit_from_cinema.message_choice_number != CHOICE_EXIT) {
            printf("[CLIENT-INTERFACE] Error after exited from cinema\n");
            client->error.info = "Error after exited from cinema";
            client->error.type = ERROR_ON_DISCONNECT_CINEMA;
            return false;
        }
    }
    client->connected = false;
    return true;
}

void* _listener_thread(void* data) {
    auto client = (Client*)data;
    if (client->listener_extern_data == nullptr) {
        return nullptr;
    }
    printf("[CLIENT-INTERFACE-LISTENER] CLIENT %d thread. Begin Poll every %d seconds\n", client->local_id, client->listener_extern_data->polling_interval_sec);
    bool poll = true;
    while (poll) {
        sleep(client->listener_extern_data->polling_interval_sec);
        //printf("[CLIENT-INTERFACE-LISTENER] Searching for a seat update for CLIENT %d\n", client->local_id);

        semaphore_wait(client->listener_updates_mutex);
            Seats_update update = static_cast<Seats_update*>(client->listener_updates)[client->listener_updates_position];
            static_cast<Seats_update*>(client->listener_updates)[client->listener_updates_position].update = NOT_SUCCESS;
            bool an_update = (update.update == SUCCESS);
        semaphore_signal(client->listener_updates_mutex);
        if (an_update) {
            //printf("[CLIENT-INTERFACE-LISTENER] An update detected for CLIENT %d\n", client->local_id);
            std::vector<Seat> seats;
            for (int i = 0; i < update.count; i++) {
                Seat seat = update.seats[i];
                seats.push_back(seat);
            }
            client->listener_extern_data->on_seat_update(seats,client->listener_extern_data->arguments);
        }
        poll = (update.update == SUCCESS || update.update == NOT_SUCCESS);
    }
    return nullptr;
}

void _start_listener_thread(Client* client) {
    _end_listener_thread(client);
    if (client == nullptr) {
        return;
    }
    printf("[CLIENT-INTERFACE] Attemp to create listener polling thread\n");
    if (pthread_create(&client->listener_thread, nullptr, _listener_thread, (void*)client) != 0) {
        printf("[CLIENT-INTERFACE] Cannot create thread\n");
        client->error.type = ERROR_GENERAL_ERROR;
        client->error.info = "Cannot start listener";
    }
}

std::vector<Room> get_rooms(int client_fd) {
    Client* client = _getClient(client_fd);
    _clean_error(client);
    _end_listener_thread(client);
    std::vector<Room> rooms;
    if (client == nullptr) {
        return rooms;
    }

    q_message rooms_request{};
    rooms_request.message_choice_number = CHOICE_ROOMS_REQUEST;
    send_message(client->mom_queue,rooms_request);

    //1.2 show rooms
    q_message rooms_response = receive_message(client->mom_queue);
    if ( rooms_response.message_choice_number != CHOICE_ROOMS_RESPONSE ) {
        if (rooms_response.message_choice_number == CHOICE_EXIT) {
            _disconnect(client,false);
            printf("[CLIENT-INTERFACE] Cinema close connection\n");
            client->error.type = ERROR_CINEMA_CLOSE_CONNECTION;
            client->error.info = "Cinema close connection";
            return rooms;
        }
        printf("[CLIENT] Unexpected Error after CHOICE_ROOMS_REQUEST\n");
        client->error.type = ERROR_GENERAL_ERROR;
        client->error.info = "Unexpected Error after CHOICE_ROOMS_REQUEST";
        return rooms;
    }
    for (int i = 0; i < rooms_response.message_choice.m2.count; i++) {
        Room aRoom{};
        aRoom.id = rooms_response.message_choice.m2.ids[i];
        rooms.push_back(aRoom);
    }
    return rooms;
}


std::vector<Seat> _seat_to_vector(m4_seats raw_seats) {
    std::vector<Seat> seats;
    for (int i = 0; i < raw_seats.count; i++) {
        Seat aSeat{};
        aSeat.id        = raw_seats.seats_id[i];
        aSeat.status    = raw_seats.seats_status[i];
        seats.push_back(aSeat);
    }
    return seats;
}

std::vector<Seat> get_seats(int client_fd,Room aRoom, on_seat_update_data* on_update_data = nullptr) {
    Client* client = _getClient(client_fd);
    _clean_error(client);
    _end_listener_thread(client);
    std::vector<Seat> seats;
    if (client == nullptr) {
        return seats;
    }

    q_message select_room_message{};
    select_room_message.message_choice_number = CHOICE_SEATS_REQUEST;
    select_room_message.message_choice.m3.room_id = aRoom.id;
    send_message(client->mom_queue,select_room_message);

    q_message room_information = receive_message(client->mom_queue);
    if (room_information.message_choice_number != CHOICE_SEATS_RESPONSE) {
        if (room_information.message_choice_number == CHOICE_EXIT) {
            _disconnect(client,false);
            printf("[CLIENT-INTERFACE] Cinema close connection\n");
            client->error.type = ERROR_CINEMA_CLOSE_CONNECTION;
            client->error.info = "Cinema close connection";
            return seats;
        }
        client->error.type = ERROR_GENERAL_ERROR;
        client->error.info = "Unexpected Error after CHOICE_SEATS_REQUEST";
        printf("[CLIENT-INTERFACE] Unexpected Error after CHOICE_SEATS_REQUEST\n");
        return seats;
    } else {
        if (room_information.message_choice.m4.success == NOT_SUCCESS) {
            printf("Not succes after select room: \"%s\"\n", room_information.message_choice.m4.information);
            client->error.type = ERROR_GENERAL_ERROR;
            client->error.info = "Not succes after select room: \"" + std::string(room_information.message_choice.m4.information) + "\"";
            return seats;
        }
    }

    // TODO: End thread when seat is reserve, request room or end connection
    if (on_update_data != nullptr) {
        client->listener_extern_data = on_update_data;
        _start_listener_thread(client);
    }

    return _seat_to_vector(room_information.message_choice.m4);
}

bool reserve_seat(int client_fd, Seat aSeat) {
    Client* client = _getClient(client_fd);
    _clean_error(client);
    if (client == nullptr) {
        return false;
    }
    q_message seat_select{};
    seat_select.message_choice_number = CHOICE_SEAT_SELECT_REQUEST;
    seat_select.message_choice.m5.seat_id = aSeat.id;
    send_message(client->mom_queue, seat_select);

    //4.0 If not succes goto 3
    q_message seat_select_response = receive_message(client->mom_queue);
    if (seat_select_response.message_choice_number != CHOICE_SEAT_SELECT_RESPONSE) {
        if (seat_select_response.message_choice_number == CHOICE_EXIT) {
            _disconnect(client,false);
            printf("[CLIENT-INTERFACE] Cinema close connection\n");
            client->error.type = ERROR_CINEMA_CLOSE_CONNECTION;
            client->error.info = "Cinema close connection";
            return false;
        }
        client->error.type = ERROR_GENERAL_ERROR;
        client->error.info = "Unexpected error when select seat";
        printf("[CLIENT-INTERFACE] Unexpected error when select seat\n");
        return false;
    }
    if (seat_select_response.message_choice.m6.success != SUCCESS) {
        char* information = seat_select_response.message_choice.m6.information;
        printf("[CLIENT-INTERFACE] Error on select seat: %s\n",information);
        client->error.type = ERROR_GENERAL_ERROR;
        client->error.info = std::string(information);
        return false;
    }
    _end_listener_thread(client);
    //4.1 else show seating information and exit.
    Reservation reservation{};
    reservation.room = aSeat.room_id;
    reservation.seat_num = aSeat.id;
    client->reservations.push_back(reservation);
    return true;
}

std::vector<Reservation> get_reservations(int client_fd) {
    Client* client = _getClient(client_fd);
    _clean_error(client);

    std::vector<Reservation> reservations;

    return (client == nullptr) ? reservations : client->reservations;
}



bool pay_seats(int client_fd) {
    Client* client = _getClient(client_fd);
    _clean_error(client);

    if (client == nullptr) {
        return false;
    }
    q_message request{};
    request.client_id = client->cinema_id;
    request.message_choice_number = CHOICE_PAY_RESERVATION_REQUEST;
    request.message_choice.m7.count = (int)client->reservations.size();
    std::copy(client->reservations.begin(),client->reservations.end(),request.message_choice.m7.list);
    printf("Sending payment, please wait...\n");
    for (auto reservation : client->reservations) {
        printf("(OK) SEAT: %d - ROOM: %d\n",reservation.seat_num,reservation.room);
        sleep(1);
    }
    send_message(client->mom_queue, request);
    q_message response = receive_message(client->mom_queue);
    if (response.message_choice_number != CHOICE_PAY_RESERVATION_RESPONSE) {
        if (response.message_choice_number == CHOICE_EXIT) {
            _disconnect(client,false);
            printf("[CLIENT-INTERFACE] Cinema close connection\n");
            client->error.type = ERROR_CINEMA_CLOSE_CONNECTION;
            client->error.info = "Cinema close connection";
        }
        printf("[CLIENT-INTERFACE] Error on pay reservations\n");
        client->error.type = ERROR_GENERAL_ERROR;
        client->error.info = "Error on pay reservations";
        return false;
    }
    if (response.message_choice.m6.success == NOT_SUCCESS) {
        char *information = response.message_choice.m6.information;
        printf("[CLIENT-INTERFACE] Error on Buy: %s\n", information);
        client->error.type = ERROR_GENERAL_ERROR;
        client->error.info = "Error on Buy: " + std::string(information);
        return false;
    }
    return true;
}

bool is_connected(int client_fd) {
    Client* client = _getClient(client_fd);
    return client != nullptr && client->connected;
}

bool is_error(int client_fd) {
    Client* client = _getClient(client_fd);
    return (client == nullptr || client->error.type != ERROR_NO_ERROR);
}

MomError get_error(int client_fd) {
    Client* client = _getClient(client_fd);
    MomError error;
    if (client == nullptr) {
        error.type = ERROR_MOM_NOT_INITIALIZED;
        error.info = "MOM not initialized";
    } else {
        error = client->error;
    }
    return error;
}

bool end_mom(int client_fd) {
    Client* client = _getClient(client_fd);
    _clean_error(client);
    if (client == nullptr) {
        return true;
    }

    //update_seats(client_fd);

    if (!_disconnect(client)) {
        client->error.type = ERROR_GENERAL_ERROR;
        client->error.info = "Error on en MOM";
        return false;
    }

    CLIENTS.erase(client_fd);

    return true;
}