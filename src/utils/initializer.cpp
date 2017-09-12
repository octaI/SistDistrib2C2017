#include "stdio.h"
#include "../../include/constants.h"
#include "../../include/ipc/communicationqueue.h"

int main() {
    printf("IPC=Initializer\n");


    printf("QUEUE_ACTIVITY [CINEMA_ADMIN-CLIENT]");
    create_commqueue(QUEUE_ACTIVITY_FILE,QUEUE_ACTIVITY_CHAR);
    printf("----(OK)\n");

    printf("QUEUE_CINEMA_ADMIN [CINEMA-CINEMA_ADMIN]");
    create_commqueue(QUEUE_CINEMA_ADMIN_FILE,QUEUE_CINEMA_ADMIN_CHAR);
    printf("----(OK)\n");

    printf("QUEUE_CLIENT [CLIENT-CLIENT]");
    create_commqueue(QUEUE_CLIENT_FILE,QUEUE_CLIENT_CHAR);
    printf("----(OK)\n");

    printf("QUEUE_COMMUNICATION_CHAR [CINEMA-CLIENT]");
    create_commqueue(QUEUE_COMMUNICATION_FILE,QUEUE_COMMUNICATION_CHAR);
    printf("----(OK)\n");

    return 0;
}


