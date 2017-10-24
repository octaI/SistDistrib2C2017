#include <messages/message.h>
#include <constants.h>
#include "../../include/network/socket.h"

int socket_serialize_message_test1() {
    int res = 1;
    q_message test_message_s,test_message_r;
    test_message_s.client_id = 9100; //fake pid
    test_message_s.message_choice.m1.client_id = 5; //fake response
    test_message_s.message_choice_number = CHOICE_CONNECTION_ACCEPTED;
    char* data_buffer = (char*) malloc(sizeof(q_message) + sizeof(int)*2);
    serialize_message(test_message_s,data_buffer);
    deserialize_message(test_message_r,data_buffer);
    printf("Message client id: %d \n",test_message_r.message_choice.m1.client_id);
    if (test_message_s.message_choice.m1.client_id != test_message_r.message_choice.m1.client_id) res = -1;
    free(data_buffer);
    return res;
}

int socket_serialize_message_test2() {
    int res = 1;
    q_message test_message_s,test_message_r;
    test_message_s.client_id = 4;
    test_message_s.message_choice.m2.count = 5;
    test_message_s.message_choice_number = CHOICE_ROOMS_RESPONSE;
    int test_ids[5] = {1,2,3,4,5};
    memcpy(test_message_s.message_choice.m2.ids,test_ids,5*sizeof(int));
    char* data_buffer = (char*) malloc(sizeof(q_message) + sizeof(int)*2);
    serialize_message(test_message_s,data_buffer);
    deserialize_message(test_message_r,data_buffer);
    printf("Deserialized message has id : %d and number of rooms : %d \n",test_message_r.client_id,test_message_r.message_choice.m2.count);
    for (int i = 0; i < 5; i++){
        printf("%d ",test_message_r.message_choice.m2.ids[i]);
    }
    printf("\n");
    free(data_buffer);
    return res;
}

int socket_serialize_message_test3(){
    int res = 1;
    q_message test_message_s,test_message_r;
    test_message_s.client_id = 6;
    test_message_s.message_choice.m3.room_id = 2;
    test_message_s.message_choice_number = CHOICE_SEATS_REQUEST;
    char* data_buffer = (char*) malloc(sizeof(q_message) + sizeof(int)*2);
    serialize_message(test_message_s,data_buffer);
    deserialize_message(test_message_r,data_buffer);
    printf("Deserialized message has id: %d and requested seats for room: %d \n",test_message_r.client_id,test_message_r.message_choice.m3.room_id);
    if (test_message_s.message_choice.m3.room_id != test_message_r.message_choice.m3.room_id) res = -1;
    free(data_buffer);

    return res;
}

int socket_serialize_message_test4(){
    printf("INITIAL VALUES: \n");
    int res = 1;
    q_message test_message_s,test_message_r;
    test_message_s.client_id = 7;
    test_message_s.message_choice_number = CHOICE_SEATS_RESPONSE;
    test_message_s.message_choice.m4.count = 7;
    test_message_s.message_choice.m4.success = SUCCESS;
    strcpy(test_message_s.message_choice.m4.information,"This is a test");
    int test_ids[7] = {1,2,3,4,5,6,7};
    char test_status[7] = {'0','1','1','0','0','1','1'};
    memcpy(test_message_s.message_choice.m4.seats_id,test_ids,sizeof(int)*7);
    memcpy(test_message_s.message_choice.m4.seats_status,test_status,sizeof(char)*7);
    printf("Client ID: %d \n",test_message_s.client_id);
    printf("Seat count: %d \n",test_message_s.message_choice.m4.count);
    printf("Information field: %s \n",test_message_s.message_choice.m4.information);
    printf("SEAT IDS: \n");
    for (int i = 0; i < 7; i++){
        printf("%d ",test_message_s.message_choice.m4.seats_id[i]);
    }
    printf("\n");
    printf("SEAT STATUS: \n");
    for (int i = 0; i < 7; i++){
        printf("%c ",test_message_s.message_choice.m4.seats_status[i]);
    }
    printf("\n");
    char* data_buffer = (char*) malloc(sizeof(q_message) + sizeof(int)*2);
    serialize_message(test_message_s,data_buffer);
    deserialize_message(test_message_r,data_buffer);
    printf("FINAL VALUES \n");
    printf("Client ID: %d \n",test_message_r.client_id);
    printf("Seat count: %d \n",test_message_r.message_choice.m4.count);
    printf("Information field: %s \n",test_message_r.message_choice.m4.information);
    printf("Success matches: %d \n", test_message_r.message_choice.m4.success == test_message_s.message_choice.m4.success);
    printf("Seat ids: \n");
    for (int i = 0; i < 7; i++){
        printf("%d ",test_message_r.message_choice.m4.seats_id[i]);
    }
    printf("\n");
    printf("Seat status: \n");
    for (int i = 0; i < 7; i++){
        printf("%c ",test_message_r.message_choice.m4.seats_status[i]);
    }
    printf("\n");
    free(data_buffer);

    return res;
}

int socket_serialize_message_test5() {
    printf("INITIAL VALUES: \n");
    int res = 1;
    Reservation res1,res2;
    res1.room = 1;
    res1.seat_num = 2;
    res2.room = 3;
    res2.seat_num = 5;
    Reservation res_list[2] = {res1,res2};
    q_message test_message_s,test_message_r;
    test_message_s.client_id = 15;
    test_message_s.message_choice_number = CHOICE_PAY_RESERVATION_REQUEST;
    test_message_s.message_choice.m7.count = 2;

    memcpy(test_message_s.message_choice.m7.list,res_list,sizeof(Reservation)*2);
    printf("Client id is: %d \n",test_message_s.client_id);
    printf("Count is : %d \n",test_message_s.message_choice.m7.count);
    for(int i = 0; i < 2; i++) {
        printf("Reservation : %d \n",i);
        printf("Room id: %d, Seat Id: %d \n",test_message_s.message_choice.m7.list[i].room,test_message_s.message_choice.m7.list[i].seat_num);
    }
    printf("\n");


    char* data_buffer = (char*) malloc(sizeof(q_message) + sizeof(int)*2);
    serialize_message(test_message_s,data_buffer);
    deserialize_message(test_message_r,data_buffer);
    printf("FINAL VALUES: \n");

    printf("Client id is: %d \n",test_message_r.client_id);
    printf("Count is : %d \n",test_message_r.message_choice.m7.count);
    for(int i = 0; i < 2; i++) {
        printf("Reservation : %d \n",i);
        printf("Room id: %d, Seat Id: %d \n",test_message_r.message_choice.m7.list[i].room,test_message_r.message_choice.m7.list[i].seat_num);
    }
    printf("\n");

    free(data_buffer);

}

int socket_serialize_message_test6() {
    int res = 1;
    printf("INITIAL VALUES \n");
    char* data_buffer = (char*) malloc(sizeof(q_message) + sizeof(int)*2);
    q_message test_message_s,test_message_r;
    test_message_s.client_id = 27;
    test_message_s.message_choice_number = CHOICE_PAY_RESERVATION_RESPONSE;
    strcpy(test_message_s.message_choice.m6.information,"Succesful Payment");
    test_message_s.message_choice.m6.success = SUCCESS;
    printf("Information field: %s \n",test_message_s.message_choice.m6.information);
    serialize_message(test_message_s,data_buffer);
    deserialize_message(test_message_r,data_buffer);
    printf("FINAL VALUES \n");
    printf("Success matches initial value %d \n", test_message_s.message_choice.m6.success == test_message_r.message_choice.m6.success);
    printf("Information field : %s \n", test_message_r.message_choice.m6.information);
    free(data_buffer);
    return res;
}

int socket_serialize_message_test7() {
    int res = 1;
    printf("INITIAL VALUES \n");
    char* data_buffer = (char*) malloc(sizeof(q_message) + sizeof(int)*2);
    q_message test_message_s,test_message_r;
    test_message_s.client_id = 27;
    test_message_s.message_choice_number = CHOICE_SEATS_REQUEST;
    test_message_s.message_choice.m3.room_id = 2;
    printf("Room id: %d \n",test_message_s.message_choice.m3.room_id);
    serialize_message(test_message_s,data_buffer);
    deserialize_message(test_message_r,data_buffer);
    printf("FINAL VALUES \n");
    printf("Room id: %d \n",test_message_r.message_choice.m3.room_id);

    free(data_buffer);

    return res;
}

int socket_serialize_message_only_with_client_id_test8() { //FOR CONNECTION M_TYPE's
    printf("INITIAL VALUES \n");
    char* data_buffer = (char*) malloc(sizeof(q_message) + sizeof(int)*2);
    q_message test_message_s,test_message_r;
    test_message_s.message_choice.m1.client_id = 12041994;
    printf("Client id: %d \n",test_message_s.message_choice.m1.client_id);

    serialize_message(test_message_s,data_buffer);
    deserialize_message(test_message_r,data_buffer);

    free(data_buffer);
    printf("Client_s id: %d \n",test_message_r.message_choice.m1.client_id);
    return (test_message_s.message_choice.m1.client_id == test_message_r.message_choice.m1.client_id) ? 1 : 0;
}

int main () {
    printf("**** TEST 1***** \n");
    socket_serialize_message_test1();
    printf("-------------------------------- \n");
    printf("**** TEST 2***** \n");
    socket_serialize_message_test2();
    printf("-------------------------------- \n");
    printf("**** TEST 3***** \n");
    socket_serialize_message_test3();
    printf("-------------------------------- \n");
    printf("**** TEST 4***** \n");
    socket_serialize_message_test4();
    printf("-------------------------------- \n");
    printf("**** TEST 5***** \n");
    socket_serialize_message_test5();
    printf("-------------------------------- \n");
    printf("**** TEST 6***** \n");
    socket_serialize_message_test6();
    printf("-------------------------------- \n");
    printf("**** TEST 7***** \n");
    socket_serialize_message_test7();
    printf("-------------------------------- \n");
    printf("**** TEST 8***** \n");
    socket_serialize_message_only_with_client_id_test8();
    printf("-------------------------------- \n");
    return 0;
}
