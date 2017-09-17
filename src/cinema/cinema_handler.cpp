#include <cstring>
#include <vector>
#include <cstdio>
#include <messages/message.h>
#include "../../include/cinema/cinema_handler.h"

q_message cinema_handle(q_message message, int *exit) {
    q_message response{};
    std::vector<int> rooms_id = {1,2,3,4,5};
    std::vector<int> seats_id = {1,2,3,4,5,6,7,8};
    std::vector<int> seats_status = {
            SEAT_STATUS_FREE,
            SEAT_STATUS_FREE,
            SEAT_STATUS_OCCUPED,
            SEAT_STATUS_FREE,
            SEAT_STATUS_OCCUPED,
            SEAT_STATUS_FREE,
            SEAT_STATUS_OCCUPED,
            SEAT_STATUS_OCCUPED
    };
    int f, o;
    switch (message.message_choice_number) {
        case CHOICE_ROOMS_REQUEST:
            response.message_choice_number = CHOICE_ROOMS_RESPONSE;
            // Request to ADMIN
            response.message_choice.m2.count = (int)rooms_id.size();
            memset(response.message_choice.m2.ids,0,MAX_ROOMS);
            memcpy(response.message_choice.m2.ids,rooms_id.data(),rooms_id.size());
            break;
        case CHOICE_SEATS_REQUEST:
            // Request to ADMIN and send user id, room_id.
            //      In this event user get into room and ADMIN send to user the seats
            /** MOCK **/
            response.message_choice.m4.count = (int)seats_id.size();
            memset(response.message_choice.m4.seats_id,0,MAX_SEATS);
            memcpy(response.message_choice.m4.seats_id,seats_id.data(),seats_id.size());
            memset(response.message_choice.m4.seats_status,0,MAX_SEATS);
            memcpy(response.message_choice.m4.seats_status,seats_status.data(),seats_status.size());
            break;
        case CHOICE_SEAT_SELECT_REQUEST:
            // Request to ADMIN and send user id, seat_id.
            response.message_choice_number = CHOICE_SEAT_SELECT_RESPONSE;
            response.message_choice.m6.success = SEAT_SELECTED_SUCCESS;
            memset(response.message_choice.m6.information,'\0',MAX_LENGTH);
            strcpy(response.message_choice.m6.information,"Selected seat success");
            break;
        case CHOICE_EXIT:
            response.message_choice_number = CHOICE_EXIT;
            *exit = CINEMA_LISTENER_EXIT;
            printf("Request exit\n");
            break;
        default:
            response.message_choice_number = CHOICE_INVALID_REQUEST;
            break;
    }

    return response;
}

