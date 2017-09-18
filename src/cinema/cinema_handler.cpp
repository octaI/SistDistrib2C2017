#include <cstring>
#include <vector>
#include <cstdio>
#include <messages/message.h>
#include <cstdlib>
#include "../../include/cinema/cinema_handler.h"

int check(const int *array, int element) {
    for (int i = 0; i < sizeof(array)/sizeof(int); i++) {
        if (array[i] == element) {
            return i;
        }
    }
    return -1;
}


q_message cinema_handle(q_message message, int *exit) {
    q_message response{};
    int rooms_id[] = {3,6,7,2,12};
    int seats_id[] = {14,42,73,264,55,67,87,228};
    int seats_status[] = {
            SEAT_STATUS_FREE,
            SEAT_STATUS_FREE,
            SEAT_STATUS_OCCUPED,
            SEAT_STATUS_FREE,
            SEAT_STATUS_OCCUPED,
            SEAT_STATUS_FREE,
            SEAT_STATUS_OCCUPED,
            SEAT_STATUS_OCCUPED
    };
    int seat_index = -1;
    switch (message.message_choice_number) {
        case CHOICE_ROOMS_REQUEST:
            response.message_choice_number = CHOICE_ROOMS_RESPONSE;
            // Request to ADMIN
            response.message_choice.m2.count = 5;
            memset(response.message_choice.m2.ids,0,MAX_ROOMS * sizeof(int));
            memcpy(response.message_choice.m2.ids,rooms_id,5 * sizeof(int));
            break;
        case CHOICE_SEATS_REQUEST:
            // Request to ADMIN and send user id, room_id.
            //      In this event user get into room and ADMIN send to user the seats
            /** MOCK **/
            response.message_choice_number = CHOICE_SEATS_RESPONSE;
            response.message_choice.m4.count = 8;
            if (check(rooms_id, message.message_choice.m3.room_id) == -1) {
                response.message_choice.m4.success = NOT_SUCCESS;
                memset(response.message_choice.m4.information,'\0',MAX_LENGTH);
                strcpy(response.message_choice.m4.information,"Invalid ROOM ID");
            } else {
                response.message_choice.m4.success = SUCCESS;
                memset(response.message_choice.m4.seats_id,0,MAX_SEATS * sizeof(int));
                memcpy(response.message_choice.m4.seats_id,seats_id,8 * sizeof(int));
                memset(response.message_choice.m4.seats_status,0,MAX_SEATS * sizeof(int));
                memcpy(response.message_choice.m4.seats_status,seats_status,8 * sizeof(int));
            }
            break;
        case CHOICE_SEAT_SELECT_REQUEST:
            // Request to ADMIN and send user id, seat_id.
            response.message_choice_number = CHOICE_SEAT_SELECT_RESPONSE;
            memset(response.message_choice.m6.information,'\0',MAX_LENGTH);
            seat_index = check(seats_id,message.message_choice.m5.seat_id);
            if (seat_index == -1) {
                response.message_choice.m6.success = NOT_SUCCESS;
                strcpy(response.message_choice.m6.information, "Invalid SEAT ID");
            } else {
                if (seats_status[seat_index] == SEAT_STATUS_FREE) {
                    response.message_choice.m6.success = SUCCESS;
                    strcpy(response.message_choice.m6.information,"Selected seat success");
                } else {
                    response.message_choice.m6.success = NOT_SUCCESS;
                    strcpy(response.message_choice.m6.information, "The seat is not FREE");
                }
            }
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

