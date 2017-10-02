#include <client/client_utils.h>
#include <cstdio>
#include <utils/TextTable.h>
#include <constants.h>

void list_reservations(const std::vector<Reservation> &reservations) {
    int count = 1;
    for (auto &reservation : reservations) {
        printf("%d | Seat %d in Room %d\n", count, reservation.seat_num, reservation.room);
        count++;
    }
    printf("Press <<Enter>> to continue\n");
    char str[MAX_CLIENT_INPUT];
    fgets(str, sizeof str, stdin);
}

void print_rooms(const std::vector<Room> &rooms) {
    TextTable table;
    table.add("Room ID  ");
    table.endOfRow();

    for (const Room &aRoom : rooms) {
        table.add(std::to_string(aRoom.id));
        table.endOfRow();
    }
    table.setAlignment(0, TextTable::Alignment::RIGHT);
    std::cout << table << std::endl;
}

void print_seats(const std::vector<Seat> &seats) {
    TextTable table;
    table.add("Seat Id  ");
    table.add("Status   ");
    table.endOfRow();
    for (const Seat &aSeat : seats) {
        int seat_id = aSeat.id;
        int seat_status = aSeat.status;
        std::string status = (seat_status == SEAT_STATUS_FREE) ? "Free" : "Occupped";
        //printf("Seat id: %d | STATUS: %s\n", seat_id, status.c_str());
        table.add(std::to_string(seat_id));
        table.add(status);
        table.endOfRow();
    }
    table.setAlignment(0,TextTable::Alignment::RIGHT);
    table.setAlignment(1,TextTable::Alignment::RIGHT);
    std::cout << table << std::endl;

}
