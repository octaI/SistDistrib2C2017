#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <messages/message.h>
#include <ipc/communicationqueue.h>
#include <constants.h>
#include <ipc/sharedmemory.h>
#include <utils/TextTable.h>
#include <signal.h>
#include <semaphore.h>
#include <ipc/semaphore.h>

#define MAX_CLIENT_INPUT 10
#define REFRESH_SEATS (-500)
#define GO_BACK (-135)

#define UPDATE_SUCCESS 1
#define NOT_UPDATED 0

#define NO_SEAT_UPDATE 0


int read_update(void* shm, int position, int sem_id) {
    /* Critic section */

    semaphore_wait(sem_id);

    int value = static_cast<int*>(shm)[position];

    semaphore_signal(sem_id);

    return value;
}

void write_update(void* shm, int position, int sem_id, int new_value) {

    semaphore_wait(sem_id);

    static_cast<int*>(shm)[position] = new_value;

    semaphore_signal(sem_id);

}

void add_update(void* shm, int position, int sem_id) {

    semaphore_wait(sem_id);

    static_cast<int*>(shm)[position] = static_cast<int*>(shm)[position] + 1;

    semaphore_signal(sem_id);

}

int client_connect_to_cinema(commqueue communication) {
    auto client_id = (int)getpid();

    q_message request_connect{};
    request_connect.message_type = TYPE_CONNECTION_REQUEST;
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

void print_rooms(const int *rooms, int count) {
    TextTable table;
    table.add("Room ID  ");
    table.endOfRow();

    for (int room_index = 0; room_index < count; room_index++) {
        table.add(std::to_string(rooms[room_index]));
        table.endOfRow();
    }
    table.setAlignment(0, TextTable::Alignment::RIGHT);
    std::cout << table << std::endl;
}

void print_seats(const int *seats, const int *seats_status, int count) {
    TextTable table;
    table.add("Seat Id  ");
    table.add("Status   ");
    table.endOfRow();
    for (int seat_index = 0; seat_index < count; seat_index++) {
        int seat_id = seats[seat_index];
        int seat_status = seats_status[seat_index];
        std::string status = (seat_status == SEAT_STATUS_FREE) ? "Free" : "Occupped";
        //printf("Seat id: %d | STATUS: %s\n", seat_id, status.c_str());
        table.add(std::to_string(seat_id));
        table.add(status);
        table.endOfRow();
    }
    table.setAlignment(0,TextTable::Alignment::RIGHT);
    table.setAlignment(1,TextTable::Alignment::RIGHT);
    std::cout << table << std::endl;

}

int update_seat_from_client(commqueue client_communication, int mutex, bool print = true) {
    void* flag_update = shm_get_data(SHM_CLIENT_FILE, SHM_CLIENT_CHAR, SHM_CLIENT_SIZE);
    int client_position = client_communication.id % MAX_CLIENTS;
    int updates = read_update(flag_update, client_position, mutex);
    int return_flag = NOT_UPDATED;
    if (updates > 0) {
        q_message seats = receive_message(client_communication);
        for (int i = 1; i < updates; i++) {
            seats = receive_message(client_communication);
        }
        write_update(flag_update, client_position, mutex, NO_SEAT_UPDATE);
        if (seats.message_choice_number == CHOICE_SEATS_RESPONSE && print) {
            print_seats(
                    seats.message_choice.m4.seats_id,
                    seats.message_choice.m4.seats_status,
                    seats.message_choice.m4.count
            );
        }
        return_flag = UPDATE_SUCCESS;
    }
    shm_dettach(flag_update);
    return return_flag;
}

void async_listener(commqueue client_communication, int mutex_shared_id) {
    client_communication.orientation = COMMQUEUE_AS_SERVER;

    commqueue admin_communication = create_commqueue(QUEUE_ACTIVITY_FILE,QUEUE_ACTIVITY_CHAR);
    admin_communication.id = client_communication.id;
    admin_communication.orientation = COMMQUEUE_AS_CLIENT;

    int client_position = client_communication.id % MAX_CLIENTS;

    void* flag_update = shm_get_data(SHM_CLIENT_FILE, SHM_CLIENT_CHAR, SHM_CLIENT_SIZE);

    write_update(flag_update,client_position,mutex_shared_id,NO_SEAT_UPDATE);

    int close_listener = 0;
    while (close_listener == 0) {
        q_message seats_update = receive_message(admin_communication);
        printf("[CLIENT-D] An updated of seats detected (refresh to view)\n");
        if (seats_update.message_choice_number == CHOICE_SEATS_RESPONSE) {
            //1. Update shared memory
            add_update(flag_update, client_position, mutex_shared_id);
            //2. Send info to the father
            send_message(client_communication, seats_update);
        } else if (seats_update.message_choice_number == CHOICE_EXIT) {
            close_listener = 1;
        } else {
            printf("[CLIENT-D] Cannot read message\n");
        }
    }
    shm_dettach(flag_update);
    exit(0);
}

int client_start_async_seat_listener(commqueue client_communication,int mutex_shared) {
    pid_t listener = fork();
    if (listener == 0) {
        async_listener(client_communication, mutex_shared);
    }
    return listener;

}


int client_select_room() {
    char str[MAX_CLIENT_INPUT];

    printf("Enter ROOM id or [B]ack to go back: ");
    fgets(str, sizeof str, stdin);

    if (strcmp(str,"Back\n") == 0 || strcmp(str,"back\n") == 0 || strcmp(str,"b\n") == 0 || strcmp(str,"B\n") == 0) {
        return GO_BACK;
    }

    int room_id = atoi(str);

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

    int room_id = atoi(str);

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

        option = atoi(str);
        selected_valid_option = option == OPTION_MAKE_RESERVATION || option == OPTION_EXIT;
        if (count_reservations > 0) {
            selected_valid_option = selected_valid_option || option == OPTION_BUY_RESERVATION || option == OPTION_SEE_RESERVATION;
        }
        msg = const_cast<char *>("** Please enter a valid option **\n");
    } while (!selected_valid_option);


    return option;

}

int close_client(commqueue communication, int mutex_id) {
    update_seat_from_client(communication,mutex_id, false);

    q_message exit{};
    exit.message_choice_number = CHOICE_EXIT;
    send_message(communication,exit);
    q_message exit_from_cinema = receive_message(communication);
    if (exit_from_cinema.message_choice_number != CHOICE_EXIT) {
        printf("[CLIENT] Error after exited from cinema\n");
        return 1;
    }

    printf("Goodbye\n");
    return 0;
}

void list_reservations(const std::vector<reservation> &reservations) {
    int count = 1;
    for (auto &reservation : reservations) {
        printf("%d | Seat %d in Room %d\n", count, reservation.seat_num, reservation.room);
        count++;
    }
    printf("Press <<Enter>> to continue\n");
    char str[MAX_CLIENT_INPUT];
    fgets(str, sizeof str, stdin);
}

int client_buy_reservations(commqueue communication, const std::vector<reservation> &reservations) {
    q_message request{};
    request.client_id = communication.id;
    request.message_choice_number = CHOICE_PAY_RESERVATION_REQUEST;
    request.message_choice.m7.count = (int)reservations.size();
    size_t mem_size = request.message_choice.m7.count*sizeof(reservation);
    memset(request.message_choice.m7.list,0,mem_size);
    memcpy(request.message_choice.m7.list,reservations.data(),mem_size);
    send_message(communication, request);
    q_message response = receive_message(communication);
    if (response.message_choice.m6.success == NOT_SUCCESS) {
        char *information = response.message_choice.m6.information;
        printf("Error on Buy: %s\n", information);
    }
    return response.message_choice.m6.success;
}

int client_start() {
    commqueue cinema_communication = create_commqueue(QUEUE_COMMUNICATION_FILE,QUEUE_COMMUNICATION_CHAR);
    cinema_communication.orientation = COMMQUEUE_AS_CLIENT;

    commqueue client_communication = create_commqueue(QUEUE_CLIENT_FILE, QUEUE_CLIENT_CHAR);
    client_communication.orientation = COMMQUEUE_AS_CLIENT;

    int mutex_shared_memory = semaphore_get(MUTEX_CLIENT_FILE, MUTEX_CLIENT_CHAR);

    int client_id = client_connect_to_cinema(cinema_communication);
    cinema_communication.id = client_id;
    client_communication.id = client_id;

    printf("Welcome CLIENT %d\n", client_id);

    std::vector<reservation> reservations;

    Menu:
    int menu_selection = client_menu((int)reservations.size());
    if (menu_selection == OPTION_MAKE_RESERVATION) {
        goto Room_Request;
    }
    if (menu_selection == OPTION_EXIT) {
        return close_client(cinema_communication, mutex_shared_memory);
    }
    if (menu_selection == OPTION_SEE_RESERVATION) {
        list_reservations(reservations);
        goto Menu;
    }
    if (menu_selection == OPTION_BUY_RESERVATION) {
        if (client_buy_reservations(cinema_communication,reservations) != SUCCESS) {
            goto Menu;
        }
        return close_client(cinema_communication, mutex_shared_memory);
    }

    //1.1 request rooms
    Room_Request:
    q_message rooms_request{};
    rooms_request.message_choice_number = CHOICE_ROOMS_REQUEST;
    send_message(cinema_communication,rooms_request);

    //1.2 show rooms
    q_message rooms = receive_message(cinema_communication);
    if ( rooms.message_choice_number != CHOICE_ROOMS_RESPONSE ) {
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
    send_message(cinema_communication,select_room_message);

    q_message room_information = receive_message(cinema_communication);
    if (room_information.message_choice_number != CHOICE_SEATS_RESPONSE) {
        if (room_information.message_choice_number == CHOICE_EXIT) {
            printf("[CLIENT] Cinema close connection\n");
            return close_client(client_communication, mutex_shared_memory);
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
    int listener_pid = client_start_async_seat_listener(client_communication, mutex_shared_memory);
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
        if (update_seat_from_client(client_communication, mutex_shared_memory) == NOT_UPDATED) {
            goto Start_Seat_Listener;
        }
        printf("No update found\n");
        goto Seat_Select;

    }
    if (selected_seat_id == GO_BACK) {
        goto Print_Rooms;
    }
    
    //3,4-b else request to select seat
    q_message seat_select{};
    seat_select.message_choice_number = CHOICE_SEAT_SELECT_REQUEST;
    seat_select.message_choice.m5.seat_id = selected_seat_id;
    send_message(cinema_communication, seat_select);

    //4.0 If not succes goto 3
    q_message seat_select_response = receive_message(cinema_communication);
    if (seat_select_response.message_choice_number != CHOICE_SEAT_SELECT_RESPONSE) {
        if (seat_select_response.message_choice_number == CHOICE_EXIT) {
            printf("[CLIENT] Cinema close connection\n");
            return close_client(client_communication, mutex_shared_memory);
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
    reservation reservation{};
    reservation.room = room_id;
    reservation.seat_num = selected_seat_id;
    reservations.push_back(reservation);
    goto Menu;
}


int main() {
    return client_start();
}
