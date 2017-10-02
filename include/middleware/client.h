#ifndef DISTRIBUIDOS_CLIENT_H
#define DISTRIBUIDOS_CLIENT_H

#include <constants.h>
#include <vector>

int init_mom();

bool login();

std::vector<room> get_rooms();

std::vector<seat> get_seats(room aRoom);

bool reserve_seat(seat aSeat);

std::vector<reservation> get_reservations();

bool pay_seat();

#endif //DISTRIBUIDOS_CLIENT_H
