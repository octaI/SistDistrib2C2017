#include <ipc/sharedmemory.h>
#include <ipc/communicationqueue.h>
#include <ipc/semaphore.h>
#include "stdio.h"
#include "../../include/constants.h"
#include "../../include/ipc/communicationqueue.h"

int main() {
    printf("==== IPC~Initializer ====\n");

    commqueue communication = create_commqueue(QUEUE_COMMUNICATION_FILE,QUEUE_COMMUNICATION_CHAR);
    printf("[OK] [%d,%d] QUEUE_COMMUNICATION_CHAR [CINEMA-CLIENT]\n", communication.id_firstqueue, communication.id_secondqueue);

    commqueue activity = create_commqueue(QUEUE_ACTIVITY_FILE,QUEUE_ACTIVITY_CHAR);
    printf("[OK] [%d,%d] QUEUE_ACTIVITY [CINEMA_ADMIN-CLIENT]\n",activity.id_firstqueue,activity.id_secondqueue);

    commqueue cinema_admin = create_commqueue(QUEUE_CINEMA_ADMIN_FILE,QUEUE_CINEMA_ADMIN_CHAR);
    printf("[OK] [%d,%d] QUEUE_CINEMA_ADMIN [CINEMA-CINEMA_ADMIN]\n", cinema_admin.id_firstqueue, cinema_admin.id_secondqueue);

    commqueue mom = create_commqueue(QUEUE_MOM_FILE,QUEUE_MOM_CHAR);
    printf("[OK] [%d,%d] QUEUE_MOM [CLIENT-MOM]\n", mom.id_firstqueue, mom.id_secondqueue);
    
    int shm_timer = shm_create(SHM_CINEMA_TIMER_FILE, SHM_CINEMA_TIMER_CHAR, SHM_CINEMA_TIMER_SIZE);
    printf("[OK] [%d] SHM_CINEMA_TIMER\n", shm_timer);

    int mutex_cinema = semaphore_create(MUTEX_CINEMA_FILE, MUTEX_CINEMA_CHAR, MUTEX_CINEMA_INIT_VALUE);
    printf("[OK] [%d] MUTEX_CINEMA\n", mutex_cinema);

    int shm_client = shm_create(SHM_CLIENT_FILE, SHM_CLIENT_CHAR, SHM_CLIENT_SIZE);
    printf("[OK] [%d] SHM_CLIENT\n", shm_client);

    int mutex_client = semaphore_create(MUTEX_CLIENT_FILE, MUTEX_CLIENT_CHAR, MUTEX_CLIENT_INIT_VALUE);
    printf("[OK] [%d] MUTEX_CLIENT\n", mutex_client);

    return 0;
}


