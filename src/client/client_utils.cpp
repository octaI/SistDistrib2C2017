#include <client/client_utils.h>
#include <cstdio>
#include <utils/TextTable.h>

void list_reservations(const std::vector<reservation> &reservations) {
    int count = 1;
    for (auto &reservation : reservations) {
        printf("%d | Seat %d in Room %d\n", count, reservation.seat_num, reservation.room);
        count++;
    }
    printf("Press <<Enter>> to continue\n");
    char str[MAX_CLIENT_INPUT];
    fgets(str, sizeof str, stdin);
}

void print_rooms(const int *rooms, int count) {
    TextTable table;
    table.add("Room ID  ");
    table.endOfRow();

    for (int room_index = 0; room_index < count; room_index++) {
        table.add(std::to_string(rooms[room_index]));
        table.endOfRow();
    }
    table.setAlignment(0, TextTable::Alignment::RIGHT);
    std::cout << table << std::endl;
}

void print_seats(const int *seats, const int *seats_status, int count) {
    TextTable table;
    table.add("Seat Id  ");
    table.add("Status   ");
    table.endOfRow();
    for (int seat_index = 0; seat_index < count; seat_index++) {
        int seat_id = seats[seat_index];
        int seat_status = seats_status[seat_index];
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
