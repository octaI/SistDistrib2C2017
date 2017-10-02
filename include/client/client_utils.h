#ifndef DISTRIBUIDOS_CLIENT_UTILS_H
#define DISTRIBUIDOS_CLIENT_UTILS_H

#include <messages/message.h>
#include <vector>

#define MAX_CLIENT_INPUT 30

void list_reservations(const std::vector<Reservation> &reservations);

void print_rooms(const std::vector<Room> &rooms);

void print_seats(const std::vector<Seat> &seats);

#endif //DISTRIBUIDOS_CLIENT_UTILS_H
