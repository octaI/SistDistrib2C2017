#ifndef DISTRIBUIDOS_CONSTANTS_H
#define DISTRIBUIDOS_CONSTANTS_H

/* -------------------------------------------------------*/
// Model constants

#define MAX_LENGTH_STRING 500
#define MAX_ROOMS 20
#define MAX_SEATS 50

#define SEAT_STATUS_FREE 0
#define SEAT_STATUS_OCCUPED 1

#define NOT_SUCCESS 1
#define SUCCESS 0

typedef struct {
    int id;
} Room;

typedef struct {
    int id;
    int room_id;
    char status;
} Seat;

typedef struct {
    char update;
    int count;
    Seat seats[MAX_SEATS];
} Seats_update;


typedef struct {
    int room;
    int seat_num;
} Reservation;


/* -------------------------------------------------------*/

//Max clients connected to cinema
#define MAX_CLIENTS 100

//Max clients in one host
#define MAX_CLIENTS_HOST 50

#define THROW_UTIL(message) \
    std::cerr << std::string(__FILE__) << "::" << std::string(std::to_string(__LINE__)) << ": " << std::string(message) << std::endl; \
    exit(1);

// Queue to communicate cinema and client
#define QUEUE_COMMUNICATION_FILE "/bin/bash"
#define QUEUE_COMMUNICATION_CHAR 'a'

// Queue to communicate cinema with admin
#define QUEUE_CINEMA_ADMIN_FILE "/bin/bash"
#define QUEUE_CINEMA_ADMIN_CHAR 'e'

// Queue to async communicate admin and clientes in a room (aka. update seats)
#define QUEUE_ACTIVITY_FILE "/bin/bash"
#define QUEUE_ACTIVITY_CHAR 'h'

// To store update information in shared memory client-side (seats update)
#define SHM_CLIENT_FILE "/bin/ls"
#define SHM_CLIENT_CHAR 'c'
#define SHM_CLIENT_SIZE sizeof(Seats_update)*MAX_CLIENTS_HOST

#define MUTEX_CLIENT_FILE "/bin/bash"
#define MUTEX_CLIENT_CHAR 'v'
#define MUTEX_CLIENT_INIT_VALUE 1

// To store update information in shared memory server-side (timer)
#define SHM_CINEMA_TIMER_FILE "/bin/bash"
#define SHM_CINEMA_TIMER_CHAR 't'
#define SHM_CINEMA_TIMER_SIZE sizeof(int)*MAX_CLIENTS

#define MUTEX_CINEMA_FILE "/bin/bash"
#define MUTEX_CINEMA_CHAR 'x'
#define MUTEX_CINEMA_INIT_VALUE 1

//Queue to use with mom and client interface
#define QUEUE_MOM_FILE "/bin/ls"
#define QUEUE_MOM_CHAR 'a'

#define DATABASE_FILENAME "main_db.db"


/*----------------------------------------------------------------------------*/
/* SOCKET COMMUNICATION PROTOCOL DEFINITION :
 * | msg type (int) | client id (int) | msg choice num (int) | specific data (...) |
 * */

//NETWORK CONSTANTS

#define MAX_BACKLOG_CONN 20

#define CINEMA_IP_ADDR "192.168.1.102"
#define CINEMA_PORT 8080
#define ADMIN_PORT 8090

#endif //DISTRIBUIDOS_CONSTANTS_H
