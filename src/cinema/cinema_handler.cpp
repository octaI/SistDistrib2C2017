#include <messages/message.h>
#include <string.h>
#include "../../include/cinema/cinema_handler.h"

q_message cinema_handle(q_message message, int *exit) {
    q_message response{};

    switch (message.message_choice_number) {
        case CHOICE_ROOMS_REQUEST:
            response.message_choice_number = CHOICE_ROOMS_RESPONSE;
            // Request to ADMIN
            int rooms_id[MAX_ROOMS] = {1,2,3,4,5};
            response.message_choice.m2.count = 5;
            memset(response.message_choice.m2.ids,0,MAX_ROOMS);
            memcpy(response.message_choice.m2.ids,rooms_id,5);
        case CHOICE_SEATS_REQUEST:
            // Request to ADMIN and send user id, room_id.
            //      In this event user get into room and ADMIN send to user the seats
            /** MOCK **/
            int seats_id[MAX_SEATS] = {1,2,3,4,5};
            int f = SEAT_STATUS_FREE;
            int o = SEAT_STATUS_OCCUPED;
            int seats_status[MAX_SEATS] = {f,o,f,f,o};
            response.message_choice.m4.count = 5;
            memset(response.message_choice.m4.seats_id,0,MAX_SEATS);
            memcpy(response.message_choice.m4.seats_id,seats_id,5);
            memset(response.message_choice.m4.seats_status,0,MAX_SEATS);
            memcpy(response.message_choice.m4.seats_status,seats_status,5);
        case CHOICE_SEAT_SELECT_REQUEST:
            // Request to ADMIN and send user id, seat_id.
            response.message_choice_number = CHOICE_SEAT_SELECT_RESPONSE;
            response.message_choice.m6.success = SEAT_SELECTED_SUCCESS;
            memset(response.message_choice.m6.information,'\0',MAX_LENGTH);
            strcpy(response.message_choice.m6.information,"Selected seat success");
        default:
            response.message_choice_number = CHOICE_INVALID_REQUEST;
        case CHOICE_EXIT:
            response.message_choice_number = CHOICE_EXIT;
            *exit = 1;
    }

    return response;
}

