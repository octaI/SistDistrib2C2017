#include <cstdio>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <messages/message.h>
#include <ipc/communicationqueue.h>
#include <constants.h>

#define MAX_CLIENT_INPUT 10
#define REFRESH_SEATS -500

int client_connect_to_cinema(commqueue communication) {
    auto client_id = (int)getpid();

    q_message request_connect{};
    request_connect.message_type = TYPE_CONNECTION_REQUEST;
    request_connect.message_choice.m1.client_id = client_id;

    send_message(communication,request_connect);
    printf("[CLIENT %d] Attemp to connect with cinema\n",client_id);

    q_message response = receive_message(communication,client_id);
    if (response.message_choice_number == CHOICE_CONNECTION_ACCEPTED) {
        printf("[CLIENT %d] Connection accepted\n",client_id);
    }

    return client_id;
}

void client_start_async_seat_listener(int client_id) {
    commqueue client_communication = create_commqueue(QUEUE_CLIENT_FILE, QUEUE_CLIENT_CHAR);
    client_communication.id = client_id;
    if (fork() == 0) {
        client_communication.orientation = COMMQUEUE_AS_SERVER;

    }

}

void client_end_async_seat_listener() {

}

int client_select_room() {
    char str[MAX_CLIENT_INPUT];

    printf("Enter a id of ROOM: ");
    scanf("%s\n", str);

    int room_id = atoi(str);

    printf("You Select ROOM %d\nPress <<Enter>> to continue",room_id);
    scanf("%s\n", str);
    printf("\n");
    return room_id;
}

int client_select_seat() {
    char str[MAX_CLIENT_INPUT];

    printf("Enter a id of SEAT or <<Refresh>> to reload: ");
    scanf("%s\n", str);
    if (strcmp(str,"Refresh") == 0 || strcmp(str,"refresh") == 0) {
        return REFRESH_SEATS;
    }
    int room_id = atoi(str);

    printf("\nYou Select SEAT %d\nPress <<Enter>> to continue",room_id);
    fgets(str, sizeof str, stdin); //Only to press ENTER
    return room_id;
}

void ____print_sleep(const std::string &message) {
    printf("%s\n",message.c_str());
    char s[200];
    fgets(s, sizeof s, stdin);
}

void client_start() {
    commqueue cinema_communication = create_commqueue(QUEUE_COMMUNICATION_FILE,QUEUE_COMMUNICATION_CHAR);
    cinema_communication.orientation = COMMQUEUE_AS_CLIENT;

    int client_id = client_connect_to_cinema(cinema_communication);
    cinema_communication.id = client_id;


    //1.1 request rooms
    Room_Request:
    //____print_sleep("PIDO ROOMS?");
    q_message rooms_request{};
    rooms_request.message_choice_number = CHOICE_ROOMS_REQUEST;
    send_message(cinema_communication,rooms_request);

    //1.2 show rooms
    q_message rooms = receive_message(cinema_communication);
    if ( rooms.message_choice_number != CHOICE_ROOMS_RESPONSE ) {
        printf("[CLIENT] Unexpected Error after CHOICE_ROOMS_REQUEST\n");
        goto Room_Request;
    }

    for (int room_index = 0; room_index < rooms.message_choice.m2.count; room_index++) {
        printf("ROOM ID: %d\n",rooms.message_choice.m2.ids[room_index]);
    }

    //2 select room
    Select_Room:
    int room_id = client_select_room();

    //____print_sleep("ELIJO ROOM");

    q_message select_room_message{};
    select_room_message.message_choice_number = CHOICE_SEATS_REQUEST;
    select_room_message.message_choice.m3.room_id = room_id;
    send_message(cinema_communication,select_room_message);

    q_message room_information = receive_message(cinema_communication);
    if (room_information.message_choice_number != CHOICE_SEATS_RESPONSE) {
        printf("[CLIENT] Unexpected Error after CHOICE_SEATS_REQUEST\n");
        goto Select_Room;
    }

    //3.1 fork listener
    Start_Seat_Listener:
    //client_start_async_seat_listener(client_id);

    //3.2 show room seating information
    for (int seat_index = 0; seat_index < room_information.message_choice.m4.count; seat_index++) {
        int seat_id = room_information.message_choice.m4.seats_id[seat_index];
        int seat_status = room_information.message_choice.m4.seats_status[seat_index];
        std::string status = (seat_status == SEAT_STATUS_FREE) ? "Free" : "Occupped";
        printf("Seat id: %d | STATUS: %s\n", seat_id, status.c_str());
    }

    Seat_Select:
    //3.3 option to see information and option to select
    int selected_seat_id = client_select_seat();
    //3.4-a if see information refresh and show seeting information
    if (selected_seat_id == REFRESH_SEATS) {
        goto Start_Seat_Listener;
    }
    //3,4-b else request to select seat
    q_message seat_select{};
    seat_select.message_choice_number = CHOICE_SEAT_SELECT_REQUEST;
    seat_select.message_choice.m5.seat_id = selected_seat_id;
    send_message(cinema_communication, seat_select);

    //4.0 If not succes goto 3
    q_message seat_select_response = receive_message(cinema_communication);
    if (seat_select_response.message_choice_number != CHOICE_SEAT_SELECT_RESPONSE) {
        printf("[CLIENT] Unexpected error when select seat\n");
        goto Seat_Select;
    }
    if (seat_select_response.message_choice.m6.success != SEAT_SELECTED_SUCCESS) {
        char* information = seat_select_response.message_choice.m6.information;
        printf("Error on select seat: %s\n",information);
        goto Seat_Select;
    }

    //client_end_async_seat_listener();

    //4.1 else show seating information and exit.
    printf("SUCCESS: Selected SEAT: %d in ROOM %d\n",selected_seat_id, room_id);
    q_message exit{};
    exit.message_choice_number = CHOICE_EXIT;
    send_message(cinema_communication,exit);
    q_message exit_from_cinema = receive_message(cinema_communication);
    if (exit_from_cinema.message_choice_number != CHOICE_EXIT) {
        printf("[CLIENT] Error after exited from cinema\n");
    }
    printf("Goodbye\n");

}


int main() {
    client_start();
    return 0;
}
