#include <cstdio>
#include <unistd.h>
#include <cstring>

#include <messages/message.h>
#include <constants.h>

#include <ipc/communicationqueue.h>
#include <ipc/sharedmemory.h>
#include <ipc/semaphore.h>

#include <csignal>
#include <utility>
#include <client/client_utils.h>
#include <middleware/client_interface.h>


#define REFRESH_SEATS (-500)
#define GO_BACK (-135)

#define POLLING_INTERVAL_SEAT 1

/**
 * SIMPLE TERMINAL UI
 */


int client_select_room() {
    char str[MAX_CLIENT_INPUT];

    printf("Enter ROOM id or [B]ack to go back: ");
    fgets(str, sizeof str, stdin);

    if (strcmp(str,"Back\n") == 0 || strcmp(str,"back\n") == 0 || strcmp(str,"b\n") == 0 || strcmp(str,"B\n") == 0) {
        return GO_BACK;
    }

    auto room_id = (int)strtol(str, nullptr,10);//atoi(str);

    printf("You Select ROOM %d\nPress <<Enter>> to show seats",room_id);
    fgets(str, sizeof str, stdin); //Only to press ENTER
    return room_id;
}

int client_select_seat() {
    char str[MAX_CLIENT_INPUT];

    printf("Enter SEAT id, [R]efresh to reload or [B]ack to go back: ");
    fgets(str, sizeof str, stdin);
    if (strcmp(str,"Refresh\n") == 0 || strcmp(str,"refresh\n") == 0 || strcmp(str,"r\n") == 0 || strcmp(str,"R\n") == 0) {
        return REFRESH_SEATS;
    }

    if (strcmp(str,"Back\n") == 0 || strcmp(str,"back\n") == 0 || strcmp(str,"b\n") == 0 || strcmp(str,"B\n") == 0) {
        return GO_BACK;
    }

    auto room_id = (int)strtol(str, nullptr,10);//atoi(str);

    printf("You Select SEAT %d\nPress <<Enter>> to reserve",room_id);
    fgets(str, sizeof str, stdin); //Only to press ENTER
    return room_id;
}

#define OPTION_MAKE_RESERVATION 1
#define OPTION_SEE_RESERVATION 2
#define OPTION_BUY_RESERVATION 3
#define OPTION_EXIT 4

int client_menu(int count_reservations) {
    char str[MAX_CLIENT_INPUT];
    auto msg = const_cast<char *>("");
    bool selected_valid_option;
    int option;
    do {
        printf("%s",msg);
        printf("\n[%d] Make reservation\n",OPTION_MAKE_RESERVATION);
        if (count_reservations > 0) {
            printf("[%d] See %d reservation%s\n",OPTION_SEE_RESERVATION,count_reservations, (count_reservations == 1) ? "" : "s");
            printf("[%d] Buy reservations\n",OPTION_BUY_RESERVATION);
        }
        printf("[%d] Exit\n",OPTION_EXIT);
        printf("Select an option: ");

        fgets(str, sizeof str, stdin);

        option = (int)strtol(str, nullptr,10);//atoi(str);
        selected_valid_option = option == OPTION_MAKE_RESERVATION || option == OPTION_EXIT;
        if (count_reservations > 0) {
            selected_valid_option = selected_valid_option || option == OPTION_BUY_RESERVATION || option == OPTION_SEE_RESERVATION;
        }
        msg = const_cast<char *>("** Please enter a valid option **\n");
    } while (!selected_valid_option);


    return option;
}


/**
 * END UI
 */

/*
typedef struct {
    void* update_flag{};
    int update_flag_position{};
    int mutex_flag_id{};
    int client_id{};
    commqueue cinema_communication{};
    commqueue update_communication{};
    std::vector<Reservation> reservations;
} client_components;

int client_read_flag_update(client_components client) {

    semaphore_wait(client.mutex_flag_id);

    int value = static_cast<int*>(client.update_flag)[client.update_flag_position];

    semaphore_signal(client.mutex_flag_id);

    return value;
}

void client_write_flag_update(client_components client, int new_value) {

    semaphore_wait(client.mutex_flag_id);

    static_cast<int*>(client.update_flag)[client.update_flag_position] = new_value;

    semaphore_signal(client.mutex_flag_id);

}

void client_add_update(client_components client) {

    semaphore_wait(client.mutex_flag_id);

    static_cast<int*>(client.update_flag)[client.update_flag_position] = static_cast<int*>(client.update_flag)[client.update_flag_position] + 1;

    semaphore_signal(client.mutex_flag_id);
}

int client_connect_to_cinema(commqueue communication) {
    auto client_id = (int)getpid();

    q_message request_connect{};
    request_connect.message_type = TYPE_CONNECTION_CINEMA_REQUEST;
    request_connect.message_choice.m1.client_id = client_id;

    send_message(communication,request_connect);
    printf("[CLIENT %d] Attemp to connect with cinema\n",client_id);

    communication.id = client_id;
    q_message response = receive_message(communication);

    if (response.message_choice_number == CHOICE_CONNECTION_ACCEPTED) {
        client_id = response.message_choice.m1.client_id;
        printf("[CLIENT %d] Connection accepted\n",client_id);
    }

    return client_id;
}

int update_seat_from_client(client_components client, bool print = true) {
    int updates = client_read_flag_update(client);
    int return_flag = NOT_UPDATED;
    if (updates > 0) {
        q_message seats = receive_message(client.update_communication);
        for (int i = 1; i < updates; i++) {
            seats = receive_message(client.update_communication);

        }
        client_write_flag_update(client, NO_SEAT_UPDATE);
        if (seats.message_choice_number == CHOICE_SEATS_RESPONSE && print) {
            print_seats(
                    seats.message_choice.m4.seats_id,
                    seats.message_choice.m4.seats_status,
                    seats.message_choice.m4.count
            );
        }
        return_flag = UPDATE_SUCCESS;
    }
    return return_flag;
}

void async_listener(client_components client) {
    client.update_communication.orientation = COMMQUEUE_AS_SERVER;

    commqueue admin_communication = create_commqueue(QUEUE_ACTIVITY_FILE,QUEUE_ACTIVITY_CHAR);
    admin_communication.id = client.client_id;
    admin_communication.orientation = COMMQUEUE_AS_CLIENT;

    client_write_flag_update(client, NO_SEAT_UPDATE);

    int close_listener = 0;
    while (close_listener == 0) {
        q_message seats_update = receive_message(admin_communication);
        printf("[CLIENT-D] An updated of seats detected (refresh to view)\n");
        if (seats_update.message_choice_number == CHOICE_SEATS_RESPONSE) {
            //1. Update shared memory
            client_add_update(client);
            //2. Send info to the father
            send_message(client.update_communication, seats_update);
        } else if (seats_update.message_choice_number == CHOICE_EXIT) {
            close_listener = 1;
        } else {
            printf("[CLIENT-D] Cannot read message\n");
        }
    }
    shm_dettach(client.update_flag);
    exit(0);
}

int client_start_async_seat_listener(client_components &client) {
    pid_t listener = fork();
    if (listener == 0) {
        async_listener(client);
    }
    return listener;
}

int client_close(client_components client) {
    update_seat_from_client(client, false);

    q_message exit{};
    exit.message_choice_number = CHOICE_EXIT;
    send_message(client.cinema_communication,exit);
    q_message exit_from_cinema = receive_message(client.cinema_communication);
    if (exit_from_cinema.message_choice_number != CHOICE_EXIT) {
        printf("[CLIENT] Error after exited from cinema\n");
        return 1;
    }

    printf("Goodbye\n");
    return 0;
}



int client_buy_reservations(client_components client) {
    q_message request{};
    request.client_id = client.client_id;
    request.message_choice_number = CHOICE_PAY_RESERVATION_REQUEST;
    request.message_choice.m7.count = (int)client.reservations.size();
    std::copy(client.reservations.begin(),client.reservations.end(),request.message_choice.m7.list);
    printf("Sending payment, please wait...\n");
    for (auto reservation : client.reservations) {
        printf("(OK) SEAT: %d - ROOM: %d\n",reservation.seat_num,reservation.room);
        sleep(1);
    }
    send_message(client.cinema_communication, request);
    q_message response = receive_message(client.cinema_communication);
    if (response.message_choice_number != CHOICE_PAY_RESERVATION_RESPONSE) {
        return NOT_SUCCESS;
    }
    if (response.message_choice.m6.success == NOT_SUCCESS) {
        char *information = response.message_choice.m6.information;
        printf("Error on Buy: %s\n", information);
    }
    return response.message_choice.m6.success;
}


client_components client_init() {
    commqueue cinema_communication = create_commqueue(QUEUE_COMMUNICATION_FILE,QUEUE_COMMUNICATION_CHAR);
    cinema_communication.orientation = COMMQUEUE_AS_CLIENT;

    commqueue client_communication = create_commqueue(QUEUE_CLIENT_FILE, QUEUE_CLIENT_CHAR);
    client_communication.orientation = COMMQUEUE_AS_CLIENT;

    int mutex_shared_memory = semaphore_get(MUTEX_CLIENT_FILE, MUTEX_CLIENT_CHAR);

    int client_id = client_connect_to_cinema(cinema_communication);
    cinema_communication.id = client_id;
    client_communication.id = client_id;

    void* flag = shm_get_data(SHM_CLIENT_FILE, SHM_CLIENT_CHAR, SHM_CLIENT_SIZE);



    client_components client;
    client.cinema_communication =   cinema_communication;
    client.update_communication =   client_communication;
    client.client_id =              client_id;
    client.mutex_flag_id =          mutex_shared_memory;
    client.update_flag =            flag;
    client.update_flag_position =   client_id % MAX_CLIENTS;

    return client;

}


int client_start() {

    client_components client = client_init();

    printf("Welcome CLIENT %d\n", client.client_id);


    Menu:
    int menu_selection = client_menu((int)client.reservations.size());
    if (menu_selection == OPTION_MAKE_RESERVATION) {
        goto Room_Request;
    }
    if (menu_selection == OPTION_EXIT) {
        return client_close(client);
    }
    if (menu_selection == OPTION_SEE_RESERVATION) {
        list_reservations(client.reservations);
        goto Menu;
    }
    if (menu_selection == OPTION_BUY_RESERVATION) {
        if (client_buy_reservations(client) != SUCCESS) {
            goto Menu;
        }
        const char *info = (client.reservations.size() == 1) ? "reservation was" : "reservations were";
        printf("OK - %d %s purchased\n",(int)client.reservations.size(),info);
        return client_close(client);
    }

    //1.1 request rooms
    Room_Request:
    q_message rooms_request{};
    rooms_request.message_choice_number = CHOICE_ROOMS_REQUEST;
    send_message(client.cinema_communication,rooms_request);

    //1.2 show rooms
    q_message rooms = receive_message(client.cinema_communication);
    if ( rooms.message_choice_number != CHOICE_ROOMS_RESPONSE ) {
        if (rooms.message_choice_number == CHOICE_EXIT) {
            update_seat_from_client(client, false);
            printf("[CLIENT] Cinema close connection\n");
            return 0;
        }
        printf("[CLIENT] Unexpected Error after CHOICE_ROOMS_REQUEST\n");
        goto Room_Request;
    }
    Print_Rooms:
    print_rooms(rooms.message_choice.m2.ids, rooms.message_choice.m2.count);

    //2 select room
    Select_Room:
    int room_id = client_select_room();
    
    if (room_id == GO_BACK) {
        goto Menu;
    }
    
    q_message select_room_message{};
    select_room_message.message_choice_number = CHOICE_SEATS_REQUEST;
    select_room_message.message_choice.m3.room_id = room_id;
    send_message(client.cinema_communication,select_room_message);

    q_message room_information = receive_message(client.cinema_communication);
    if (room_information.message_choice_number != CHOICE_SEATS_RESPONSE) {
        if (room_information.message_choice_number == CHOICE_EXIT) {
            update_seat_from_client(client, false);
            printf("[CLIENT] Cinema close connection\n");
            return 0;
        }
        printf("[CLIENT] Unexpected Error after CHOICE_SEATS_REQUEST\n");
        goto Select_Room;
    } else {
        if (room_information.message_choice.m4.success == NOT_SUCCESS) {
            printf("Not succes after select room: \"%s\"\n", room_information.message_choice.m4.information);
            goto Select_Room;
        }
    }

    //3.1 fork listener
    int listener_pid = client_start_async_seat_listener(client);
    Start_Seat_Listener:

    //3.2 show room seating information
    print_seats(
            room_information.message_choice.m4.seats_id,
            room_information.message_choice.m4.seats_status,
            room_information.message_choice.m4.count
    );

    Seat_Select:
    //3.3 option to see information and option to select
    int selected_seat_id = client_select_seat();
    //3.4-a if see information refresh and show seeting information
    if (selected_seat_id == REFRESH_SEATS) {
        if (update_seat_from_client(client) == NOT_UPDATED) {
            goto Start_Seat_Listener;
        }
        printf("No update found\n");
        goto Seat_Select;

    }
    if (selected_seat_id == GO_BACK) {
        goto Print_Rooms;
    }
    
    //3,4-b else request to select Seat
    q_message seat_select{};
    seat_select.message_choice_number = CHOICE_SEAT_SELECT_REQUEST;
    seat_select.message_choice.m5.seat_id = selected_seat_id;
    send_message(client.cinema_communication, seat_select);

    //4.0 If not succes goto 3
    q_message seat_select_response = receive_message(client.cinema_communication);
    if (seat_select_response.message_choice_number != CHOICE_SEAT_SELECT_RESPONSE) {
        if (seat_select_response.message_choice_number == CHOICE_EXIT) {
            update_seat_from_client(client, false);
            printf("[CLIENT] Cinema close connection\n");
            return 0;
        }
        printf("[CLIENT] Unexpected error when select seat\n");
        goto Seat_Select;
    }
    if (seat_select_response.message_choice.m6.success != SUCCESS) {
        char* information = seat_select_response.message_choice.m6.information;
        printf("Error on select seat: %s\n",information);
        goto Seat_Select;
    }

    // Kill listener
    kill(listener_pid, SIGKILL);

    //4.1 else show seating information and exit.
    printf("SUCCESS: Reserved SEAT %d in ROOM %d\n",selected_seat_id, room_id);
    Reservation reservation{};
    reservation.room = room_id;
    reservation.seat_num = selected_seat_id;
    client.reservations.push_back(reservation);
    goto Menu;
}
*/


int client_close(int fd) {
    return end_mom(fd) ? 0 : 1;
}

bool client_look_error(int fd) {
    if (is_error(fd)) {
        MomError error = get_error(fd);
        printf("%s\n",error.info.c_str());
        return true;
    }
    return false;
}


typedef struct {
    std::vector<Seat> seats{};
    bool an_update{};
} on_update_arguments;

void on_update(std::vector<Seat> seats_update, void* arguments) {
    auto data = (on_update_arguments*)arguments;
    data->seats = seats_update;
    data->an_update = true;
    printf("\nThere is an update on the state of the seats (%d). Please refresh\n Select option: ", (int)seats_update.size());
}

int client_start() {

    int mom_fd = init_mom();
    if (client_look_error(mom_fd)) {
        return 1;
    }

    if (!login(mom_fd)) {
        client_look_error(mom_fd);
        return 1;
    }
    printf("Welcome CLIENT\n");


    Menu:
    int reservations_count = (int)get_reservations(mom_fd).size();
    int menu_selection = client_menu(reservations_count);
    if (menu_selection == OPTION_MAKE_RESERVATION) {
        goto Room_Request;
    }
    if (menu_selection == OPTION_EXIT) {
        return client_close(mom_fd);;
    }
    if (menu_selection == OPTION_SEE_RESERVATION) {
        list_reservations(get_reservations(mom_fd));
        goto Menu;
    }
    if (menu_selection == OPTION_BUY_RESERVATION) {
        if (!pay_seats(mom_fd)) {
            goto Menu;
        }
        const char *info = (reservations_count == 1) ? "reservation was" : "reservations were";
        printf("OK - %d %s purchased\n",reservations_count,info);
        return client_close(mom_fd);
    }

    //1.1 request rooms
    Room_Request:
    std::vector<Room> rooms = get_rooms(mom_fd);
    if (client_look_error(mom_fd)) {
        if (!is_connected(mom_fd)) {
            return client_close(mom_fd);
        }
        goto Menu;
    }

    Print_Rooms:
    print_rooms(rooms);


    //2 select room
    Select_Room:
    int selected_room_id = client_select_room();
    if (selected_room_id == GO_BACK) {
        goto Menu;
    }

    Room selected_room{};
    selected_room.id = selected_room_id;


    on_update_arguments update_args;
    update_args.an_update = false;

    on_seat_update_data update_data{};
    update_data.polling_interval_sec = POLLING_INTERVAL_SEAT;
    update_data.on_seat_update = &on_update;
    update_data.arguments = &update_args;

    std::vector<Seat> seats = get_seats(mom_fd, selected_room, &update_data);
    if (client_look_error(mom_fd)) {
        if (!is_connected(mom_fd)) {
            return client_close(mom_fd);
        }
        goto Menu;
    }


    //3.2 show room seating information
    Print_Seats:
    print_seats(seats);

    Seat_Select:
    //3.3 option to see information and option to select
    int selected_seat_id = client_select_seat();
    if (selected_seat_id == GO_BACK) {
        goto Print_Rooms;
    }
    //3.4-a if see information refresh and show seeting information
    if (selected_seat_id == REFRESH_SEATS) {
        if (!update_args.an_update) {
            printf("No update\n");
            goto Seat_Select;
        }
        seats = update_args.seats;
        update_args.an_update = false;
        goto Print_Seats;

    }


    Seat selected_seat{};
    selected_seat.id = selected_seat_id;
    selected_seat.room_id = selected_room.id;

    if ( reserve_seat(mom_fd, selected_seat) ) {
        goto Menu;
    }
    if (client_look_error(mom_fd) && !is_connected(mom_fd)) {
        return client_close(mom_fd);
    }
    printf("ERROR: Cannot reserve SEAT %d in ROOM %d\n",selected_seat_id, selected_room.id);
    goto Seat_Select;


}



int main() {
    return client_start();
}
