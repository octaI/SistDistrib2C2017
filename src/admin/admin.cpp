#include <sys/unistd.h>
#include <iostream>
#include <cstring>
#include <messages/message.h>
#include "../../include/ipc/communicationqueue.h"
#include "../../include/db/db_api.h"
#include "../../include/constants.h"
#include "../../include/messages/message.h"

void admin_handle_request(sqlite3 *handle,commqueue channel,q_message request) {
    q_message response;
    response.client_id = request.client_id;
    switch (request.message_choice_number) {
        case CHOICE_ROOMS_REQUEST: {//expects an array of room ids
            std::vector<int> room_ids = {};
            room_ids = db_select_room(handle);
            if (room_ids.size() < 1) {
                response.message_choice_number = CHOICE_INVALID_REQUEST;
                break;
            }
            response.message_choice_number = CHOICE_ROOMS_RESPONSE;
            response.message_choice.m2.count = (int) room_ids.size();
            std::copy(room_ids.begin(), room_ids.end(), response.message_choice.m2.ids);
            break;
        }
        case CHOICE_SEATS_REQUEST:{
            std::vector<int> seat_ids = {};
            std::vector<int> taken_seats = {};
            seat_ids = db_select_room_seats(handle,request.message_choice.m3.room_id);
            if(seat_ids.size() < 1) {
                response.message_choice_number = CHOICE_INVALID_REQUEST;
                break;
            }
            std::vector<int> seat_status(seat_ids.size(),0);
            taken_seats = db_select_reservations(handle,request.message_choice.m3.room_id);
            for  (auto &index : taken_seats ) {
                seat_status[index] = 1;
            }
            response.message_choice_number = CHOICE_SEATS_RESPONSE;
            response.message_choice.m4.count = (int)seat_ids.size();
            std::copy(seat_ids.begin(),seat_ids.end(),response.message_choice.m4.seats_id);
            std::copy(taken_seats.begin(),taken_seats.end(),response.message_choice.m4.seats_status);
        }

        case CHOICE_SEAT_SELECT_REQUEST: {
            //db_insert_reservation(handle,request.message_choice.m5.seat_id,request.message_choice.m5.)
        }
    }
}

void admin_listen_requests(sqlite3 *handle,commqueue channel){
    q_message client_req = receive_message(channel,ADMIN_REQUEST);
    //received a request
    if (fork() == 0) { //forking to handle request
        admin_handle_request(handle,channel,client_req);
        exit(0);
    }

}

void admin_daemon(){
    std::cout << "INITIATED ADMIN" << std::endl;
    sqlite3* handle = 0;
    db_create(handle,DATABASE_FILENAME);
    db_initialize(handle);
    commqueue admin_channel = create_commqueue(QUEUE_CINEMA_ADMIN_FILE,QUEUE_CINEMA_ADMIN_CHAR);
    admin_channel.orientation = ADMIN_TO_CINEMA;

    while(true){
        admin_listen_requests(handle,admin_channel);
    }
}

int main(){
    admin_daemon();
    return 0;
}