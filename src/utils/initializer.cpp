#include <ipc/sharedmemory.h>
#include <ipc/communicationqueue.h>
#include "stdio.h"
#include "../../include/constants.h"
#include "../../include/ipc/communicationqueue.h"

int main() {
    printf("==== IPC~Initializer ====\n");

    commqueue activity = create_commqueue(QUEUE_ACTIVITY_FILE,QUEUE_ACTIVITY_CHAR);
    printf("[OK] [%d,%d] QUEUE_ACTIVITY [CINEMA_ADMIN-CLIENT]\n",activity.id_firstqueue,activity.id_secondqueue);

    commqueue cinema_admin = create_commqueue(QUEUE_CINEMA_ADMIN_FILE,QUEUE_CINEMA_ADMIN_CHAR);
    printf("[OK] [%d,%d] QUEUE_CINEMA_ADMIN [CINEMA-CINEMA_ADMIN]\n", activity.id_firstqueue, activity.id_secondqueue);

    int shm_timer = shm_create(SHM_CINEMA_TIMER_FILE, SHM_CINEMA_TIMER_CHAR, SHM_CINEMA_TIMER_SIZE);
    printf("[OK] [%d] SHM_CINEMA_TIMER\n", shm_timer);

    commqueue client = create_commqueue(QUEUE_CLIENT_FILE,QUEUE_CLIENT_CHAR);
    printf("[OK] [%d,%d] QUEUE_CLIENT [CLIENT-CLIENT]\n",client.id_firstqueue,client.id_secondqueue);

    int shm_client = shm_create(SHM_CLIENT_FILE, SHM_CLIENT_CHAR, SHM_CLIENT_SIZE);
    printf("[OK] [%d] SHM_CLIENT\n", shm_client);

    commqueue communication = create_commqueue(QUEUE_COMMUNICATION_FILE,QUEUE_COMMUNICATION_CHAR);
    printf("[OK] [%d,%d]QUEUE_COMMUNICATION_CHAR [CINEMA-CLIENT]\n", communication.id_firstqueue, communication.id_secondqueue);

    return 0;
}


