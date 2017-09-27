#ifndef DISTRIBUIDOS_CLIENT_UTILS_H
#define DISTRIBUIDOS_CLIENT_UTILS_H

#include <messages/message.h>
#include <vector>

#define MAX_CLIENT_INPUT 30

void list_reservations(const std::vector<reservation> &reservations);

void print_rooms(const int *rooms, int count);

void print_seats(const int *seats, const int *seats_status, int count);

#endif //DISTRIBUIDOS_CLIENT_UTILS_H
