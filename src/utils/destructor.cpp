#include <ipc/sharedmemory.h>
#include <ipc/semaphore.h>
#include "stdio.h"

#include "../../include/ipc/communicationqueue.h"
#include "../../include/constants.h"

int main() {
    printf("==== IPC~Destructor ====\n");

    delete_queue(create_commqueue(QUEUE_COMMUNICATION_FILE,QUEUE_COMMUNICATION_CHAR));
    printf("[OK] QUEUE_COMMUNICATION_CHAR [CINEMA-CLIENT]\n");

    delete_queue(create_commqueue(QUEUE_ACTIVITY_FILE,QUEUE_ACTIVITY_CHAR));
    printf("[OK] QUEUE_ACTIVITY [CINEMA_ADMIN-CLIENT]\n");

    delete_queue(create_commqueue(QUEUE_CINEMA_ADMIN_FILE,QUEUE_CINEMA_ADMIN_CHAR));
    printf("[OK] QUEUE_CINEMA_ADMIN [CINEMA-CINEMA_ADMIN]\n");

    delete_queue(create_commqueue(QUEUE_MOM_FILE,QUEUE_MOM_CHAR));
    printf("[OK] QUEUE_MOM_ADMIN [CLIENT-MOM]\n");

    shm_remove(shm_create(SHM_CINEMA_TIMER_FILE, SHM_CINEMA_TIMER_CHAR, SHM_CINEMA_TIMER_SIZE));
    printf("[OK] SHM_CINEMA_TIMER\n");

    semaphore_delete(semaphore_get(MUTEX_CLIENT_FILE,MUTEX_CLIENT_CHAR));
    printf("[OK] MUTEX_CINEMA\n");

    shm_remove(shm_create(SHM_CLIENT_FILE, SHM_CLIENT_CHAR, SHM_CLIENT_SIZE));
    printf("[OK] SHM_CLIENT\n");

    semaphore_delete(semaphore_get(MUTEX_CINEMA_FILE,MUTEX_CINEMA_CHAR));
    printf("[OK] MUTEX_CINEMA\n");
}
