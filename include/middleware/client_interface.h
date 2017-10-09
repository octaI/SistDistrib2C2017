#ifndef DISTRIBUIDOS_CLIENT_H
#define DISTRIBUIDOS_CLIENT_H

#include <constants.h>
#include <vector>
#include <string>

typedef struct {
    short       type{};
    std::string info{};
} MomError;

typedef struct {
    unsigned int polling_interval_sec;
    void (*on_seat_update)(std::vector<Seat>, void*);
    void* arguments;
} on_seat_update_data;

int init_mom();

bool login(int client_fd);

std::vector<Room> get_rooms(int client_fd);

std::vector<Seat> get_seats(int client_fd, Room aRoom, on_seat_update_data* on_update_data);

bool reserve_seat(int client_fd, Seat aSeat);

std::vector<Reservation> get_reservations(int client_fd);

bool pay_seats(int client_fd);

bool is_connected(int client_fd);

bool is_error(int client_fd);

MomError get_error(int client_fd);

bool end_mom(int client_fd);

#endif //DISTRIBUIDOS_CLIENT_H
