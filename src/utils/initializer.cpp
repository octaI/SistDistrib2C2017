#include "stdio.h"
#include "../../include/constants.h"
#include "../../include/ipc/communicationqueue.h"

int main() {
    printf("IPC=Initializer\n");

    create_commqueue(QUEUE_ACTIVITY_FILE,QUEUE_ACTIVITY_CHAR);
    printf("[OK] QUEUE_ACTIVITY [CINEMA_ADMIN-CLIENT]\n");

    create_commqueue(QUEUE_CINEMA_ADMIN_FILE,QUEUE_CINEMA_ADMIN_CHAR);
    printf("[OK] QUEUE_CINEMA_ADMIN [CINEMA-CINEMA_ADMIN]\n");

    create_commqueue(QUEUE_CLIENT_FILE,QUEUE_CLIENT_CHAR);
    printf("[OK] QUEUE_CLIENT [CLIENT-CLIENT]\n");

    create_commqueue(QUEUE_COMMUNICATION_FILE,QUEUE_COMMUNICATION_CHAR);
    printf("[OK] QUEUE_COMMUNICATION_CHAR [CINEMA-CLIENT]\n");

    return 0;
}


