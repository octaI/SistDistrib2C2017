#include <cstring>
#include <vector>
#include <messages/message.h>
#include <ipc/communicationqueue.h>
#include "../../include/cinema/cinema_handler.h"

q_message cinema_handle(commqueue admin_comunication, q_message message, int *exit) {
    q_message response{};
    switch (message.message_choice_number) {
        case CHOICE_ROOMS_REQUEST:
        case CHOICE_SEATS_REQUEST:
            printf("[CINEMA] Request seats for ROOM %d\n", message.message_choice.m3.room_id);
        case CHOICE_SEAT_SELECT_REQUEST:
            send_message(admin_comunication, message);
            admin_comunication.id = message.client_id;
            response = receive_message(admin_comunication);
            break;
        case CHOICE_EXIT:
            response.message_choice_number = CHOICE_EXIT;
            *exit = CINEMA_LISTENER_EXIT;
            break;
        default:
            response.message_choice_number = CHOICE_INVALID_REQUEST;
    }

    return response;
}

