#include <semaphore.h>
#include <sys/ipc.h>
#include <constants.h>
#include <cstdlib>
#include <iostream>
#include <sys/sem.h>
#include <cstring>

void sem_init(int sem_id, int init_value) {
    union semnum {
        int val;
        struct semid_ds* buf;
        ushort* array;
    };

    semnum init{};
    init.val = init_value;
    int result = semctl ( sem_id,0,SETVAL,init );
    if (result < 0) {
        THROW_UTIL( std::string("Error en semctl(): ") + std::string(strerror(errno)) );
    }
}


int semaphore_get(const char* file, char letter) {
    key_t key = ftok(file,letter);
    int sem_id = semget(key, 1, 0666 | IPC_CREAT );
    if (sem_id < 0){
        THROW_UTIL( std::string("Error en semget() (crear): ") + std::string(strerror(errno)) );
    }
    return sem_id;
}

int semaphore_create(const char* file, char letter, int init_value) {
    int sem_id = semaphore_get(file,letter);
    sem_init(sem_id, init_value);
    return sem_id;
}

void sem_op(int sem_id, int value) {
    struct sembuf operation{};

    operation.sem_num = 0;	// numero de semaforo
    operation.sem_op  = (short)value;
    operation.sem_flg = SEM_UNDO;

    int result = semop ( sem_id,&operation,1 );
    if (result < 0) {

        THROW_UTIL( "Hubo un error al setear el semaforo (v[" + std::to_string(value) + "]) " + std::to_string(sem_id) + " (" + std::string(strerror(errno)) + ")" );
    }

}

void semaphore_signal(int sem_id) {
    sem_op(sem_id, 1);
}

void semaphore_wait(int sem_id) {
    sem_op(sem_id, -1);
}

void semaphore_delete(int sem_id) {
    int result = semctl (sem_id,0,IPC_RMID );
    if (result < 0) {
        THROW_UTIL( std::string("Error en semctl(): ") + std::string(strerror(errno)) );
    }
}


