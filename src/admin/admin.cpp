#include <sys/unistd.h>
#include <iostream>
#include <cstring>
#include <messages/message.h>
#include <ipc/communicationqueue.h>
#include "../../include/ipc/communicationqueue.h"
#include "../../include/db/db_api.h"
#include "../../include/constants.h"
#include "../../include/messages/message.h"


void show_vector(std::vector<int> result){
    for ( auto i = result.begin(); i != result.end(); i++){
        std::cout << *i << " ";
    }
    std::cout << std::endl;
}

void admin_handle_request(sqlite3 *handle,commqueue channel,q_message request) {
    q_message response{};
    response.client_id = request.client_id;
    channel.id = response.client_id;
    switch (request.message_choice_number) {
        case CHOICE_CONNECTION_ACCEPTED:
            response.message_choice_number = CHOICE_CONNECTION_ACCEPTED;
            response.message_choice.m1.client_id = db_insert_user(handle);
            printf("[CINEMA-ADMIN] Register user who start with type %d, user id: %d\n",response.client_id, response.message_choice.m1.client_id);
            break;
        case CHOICE_ROOMS_REQUEST: {//expects an array of room ids
            std::vector<int> room_ids = {};
            room_ids = db_select_room(handle);
            /*
            if (room_ids.empty()) {
                response.message_choice_number = CHOICE_INVALID_REQUEST;
                break;
            }
             */
            response.message_choice_number = CHOICE_ROOMS_RESPONSE;
            response.message_choice.m2.count = (int) room_ids.size();
            std::copy(room_ids.begin(), room_ids.end(), response.message_choice.m2.ids);
            break;
        }
        case CHOICE_SEATS_REQUEST:{
            std::vector<int> seat_ids = {};
            std::vector<int> taken_seats = {};
            seat_ids = db_select_room_seats(handle,request.message_choice.m3.room_id);
            printf("[CINEMA-ADMIN] Receive request for seats for room %d\n", request.message_choice.m3.room_id);
            if(seat_ids.empty()) {
                response.message_choice_number = CHOICE_INVALID_REQUEST;
                break;
            }
            std::vector<int> seat_status(seat_ids.size(),SEAT_STATUS_FREE);
            taken_seats = db_select_reservations(handle,request.message_choice.m3.room_id);
            std::cout << "[ADMIN-CINEMA] This is the taken seats Vector " ;
            show_vector(taken_seats);

            std::cout << "[ADMIN-CINEMA] Seat status vector length: " << seat_status.size() << std::endl;
            for  (auto &index : taken_seats ) {
                std::cout << "SEAT " << index << "IS TAKEN"  << std::endl;
                seat_status[index-1] = SEAT_STATUS_OCCUPED;
            }
            show_vector(seat_status);
            response.message_choice_number = CHOICE_SEATS_RESPONSE;
            response.message_choice.m4.count = (int)seat_ids.size();
            std::copy(seat_ids.begin(),seat_ids.end(),response.message_choice.m4.seats_id);
            std::copy(seat_status.begin(),seat_status.end(),response.message_choice.m4.seats_status);
            db_insert_user_in_room(handle,request.client_id,request.message_choice.m3.room_id);
            break;
        }

        case CHOICE_SEAT_SELECT_REQUEST: {
            printf("PASO 1\n");
            int current_room = db_select_user_current_room(handle,request.client_id);
            if(current_room == 0) { //client not in any room
                printf("[CINEMA-ADMIN] ERROR: CLIENT %d not in room\n", request.client_id);
                response.message_choice_number = CHOICE_INVALID_REQUEST;
                break;
            }
            printf("PASO 2\n");
            if(db_insert_reservation(handle,request.client_id,current_room,request.message_choice.m5.seat_id) == 0){
                printf("[CINEMA-ADMIN] Error on generate reservation to CLIENT %d\n",response.client_id);
                response.message_choice_number = CHOICE_SEAT_SELECT_RESPONSE;
                response.message_choice.m6.success = NOT_SUCCESS;
                strcpy(response.message_choice.m6.information,"You have entered an invalid seat. ");
                break;
            }
            printf("PASO 3\n");
            db_remove_user_in_room(handle,request.client_id);
            std::vector<int> users_to_update = db_select_users_in_room(handle,current_room);
            if (!users_to_update.empty()) {
                printf("PASO 3.1\n");
                commqueue client_channel = create_commqueue(QUEUE_ACTIVITY_FILE,QUEUE_ACTIVITY_CHAR);
                client_channel.orientation = COMMQUEUE_AS_SERVER;
                std::vector<int> seats = db_select_room_seats(handle,current_room);
                std::vector<int> taken_seats = db_select_reservations(handle,current_room);
                std::vector<int> seat_status(seats.size(),SEAT_STATUS_FREE);
                for (auto &index: taken_seats) {
                    seat_status[index-1] = SEAT_STATUS_OCCUPED;
                }
                for (auto userid : users_to_update){
                    printf("[CINEMA-ADMIN] Updating User ID: %d with taken seats: ",userid);
                    show_vector(seat_status);
                    q_message update_msg{};
                    client_channel.id = userid;
                    update_msg.client_id = userid;
                    update_msg.message_choice_number = CHOICE_SEATS_RESPONSE;
                    update_msg.message_choice.m4.success = SUCCESS;
                    update_msg.message_choice.m4.count = (int)seats.size();
                    std::copy(seats.begin(),seats.end(),update_msg.message_choice.m4.seats_id);
                    std::copy(seat_status.begin(),seat_status.end(),update_msg.message_choice.m4.seats_status);
                    send_message(client_channel,update_msg);
                }
            }
            printf("PASO 4\n");
            printf("[CINEMA-ADMIN] Reservation succesfully made to client %d\n", response.client_id);
            db_remove_user_in_room(handle,request.client_id);
            response.client_id = request.client_id;
            response.message_choice_number = CHOICE_SEAT_SELECT_RESPONSE;
            response.message_choice.m6.success = SUCCESS;
            std::string succ_msg = "Succesfully made seat reservation";
            strcpy(response.message_choice.m6.information,succ_msg.c_str());
            break;
        }

        default: {
            response.message_choice_number = CHOICE_INVALID_REQUEST;
        }
    }

    send_message(channel,response);
}

void admin_listen_requests(sqlite3 *handle,commqueue channel){
    printf("[CINEMA-ADMIN] Waiting for an incoming request\n");
    q_message client_req = receive_message(channel,ADMIN_REQUEST);
    //received a request
    if (fork() == 0) { //forking to handle request
        printf("[CINEMA-ADMIN] Handler request from %d | MSG_CHOICE: %d\n", client_req.client_id, client_req.message_choice_number);
        admin_handle_request(handle,channel,client_req);
        exit(0);
    }

}

void admin_daemon(){
    sqlite3* handle;
    db_create(handle,DATABASE_FILENAME);
    db_initialize(handle);

    commqueue admin_channel = create_commqueue(QUEUE_CINEMA_ADMIN_FILE,QUEUE_CINEMA_ADMIN_CHAR);
    admin_channel.orientation = COMMQUEUE_AS_SERVER;

    while(true){
        admin_listen_requests(handle,admin_channel);
    }
}

/*
int main(){
    admin_daemon();
    return 0;
}
 */