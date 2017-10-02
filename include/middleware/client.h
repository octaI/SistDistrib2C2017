#ifndef DISTRIBUIDOS_CLIENT_H
#define DISTRIBUIDOS_CLIENT_H

#include <constants.h>
#include <vector>

int init_mom();

bool login(int client_fd);

std::vector<Room> get_rooms(int client_fd);

std::vector<Seat> get_seats(int client_fd, Room aRoom);

std::vector<Seat> update_seats(int client_fd);

bool reserve_seat(int client_fd, Seat aSeat);

std::vector<Reservation> get_reservations(int client_fd);

bool pay_seats(int client_fd);

bool is_connected(int client_fd);

bool end_mom(int client_fd);

#endif //DISTRIBUIDOS_CLIENT_H
