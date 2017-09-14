#include "stdio.h"

#include "../../include/ipc/communicationqueue.h"
#include "../../include/constants.h"

int main() {
    printf("IPC = Destructor\n");

    delete_queue(create_commqueue(QUEUE_ACTIVITY_FILE,QUEUE_ACTIVITY_CHAR));
    printf("[OK] QUEUE_ACTIVITY [CINEMA_ADMIN-CLIENT]\n");

    delete_queue(create_commqueue(QUEUE_CINEMA_ADMIN_FILE,QUEUE_CINEMA_ADMIN_CHAR));
    printf("[OK] QUEUE_CINEMA_ADMIN [CINEMA-CINEMA_ADMIN]\n");

    delete_queue(create_commqueue(QUEUE_CLIENT_FILE,QUEUE_CLIENT_CHAR));
    printf("[OK] QUEUE_CLIENT [CLIENT-CLIENT]\n");

    delete_queue(create_commqueue(QUEUE_COMMUNICATION_FILE,QUEUE_COMMUNICATION_CHAR));
    printf("[OK] QUEUE_COMMUNICATION_CHAR [CINEMA-CLIENT]\n");
}
