#ifndef DISTRIBUIDOS_MESSAGE_H
#define DISTRIBUIDOS_MESSAGE_H

/* definition of general constants in messages */

#define MAX_LENGTH 500
#define MAX_ROOMS 100
#define MAX_SEATS 300

#define SEAT_STATUS_FREE 0
#define SEAT_STATUS_OCCUPED 1

#define NOT_SUCCESS 1
#define SUCCESS 0

/* definition of other structs */

typedef struct {
    int room;
    int seat_num;
} reservation;

/* Definition message */

typedef struct {
    char test_msg [MAX_LENGTH] ;
} m0_test;

typedef struct {
    int client_id;
} m1_connection_request;


typedef struct {
    int count;
    int ids[MAX_ROOMS];
} m2_rooms;

typedef struct {
    int room_id;
} m3_seats_request;

typedef struct {
    int count;
    short success;
    char information[MAX_LENGTH];
    int seats_id[MAX_SEATS];
    int seats_status[MAX_SEATS];
} m4_seats;

typedef struct {
    int seat_id;

} m5_seats_select;

typedef struct {
    int success;
    char information[MAX_LENGTH];
} m6_atomic_response;

typedef struct {
    int count;
    reservation list[MAX_LENGTH];
} m7_seat_payments;

/* Definition of message pattern */

typedef struct {
    int message_type;
    int client_id;
    unsigned int message_choice_number;
    union {
        m0_test                 m0;
        m1_connection_request   m1;
        m2_rooms                m2;
        m3_seats_request        m3;
        m4_seats                m4;
        m5_seats_select         m5;
        m6_atomic_response      m6;
        m7_seat_payments        m7;
    } message_choice;
} q_message;

/* Definition message type id */
#define TYPE_CONNECTION_REQUEST     1

/* Definition message choice number
 * - Note: Some messages do not need more information than the type (choice_number) ej. CHOICE_ROOMS_REQUEST
 */
//#define CHOICE_CONNECTION_REQUEST     1  not required because it has a specified type
#define CHOICE_CONNECTION_ACCEPTED 2
#define CHOICE_ROOMS_REQUEST 3
#define CHOICE_ROOMS_RESPONSE 4

#define CHOICE_SEATS_REQUEST 5
#define CHOICE_SEATS_RESPONSE 6

#define CHOICE_SEAT_SELECT_REQUEST 7
#define CHOICE_SEAT_SELECT_RESPONSE 8

#define CHOICE_PAY_RESERVATION_REQUEST 11
#define CHOICE_PAY_RESERVATION_RESPONSE 12

#define CHOICE_EXIT 9

#define CHOICE_INVALID_REQUEST 10

#endif //DISTRIBUIDOS_MESSAGE_H
