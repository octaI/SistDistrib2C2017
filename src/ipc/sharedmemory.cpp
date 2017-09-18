#include "../../include/ipc/sharedmemory.h"
#include <constants.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>

int shm_create(const char* file, char letter, size_t data_size) {
    // Create
    key_t key = ftok(file,letter);
    if (key == -1) {
        THROW_UTIL(strerror(errno));
    }
    int shm_id = shmget(key, data_size, IPC_CREAT | 0660 );
    if (shm_id == -1 ) {
        THROW_UTIL(std::string(strerror(errno)));
    }
    return shm_id;
}

void* shm_get_data(const char* file, char letter, size_t data_size) {
    int shm_id = shm_create(file,letter,data_size);

    void* data_ptr = shmat(shm_id, nullptr,0);
    if ( data_ptr == (void*) -1 ) {
        THROW_UTIL( std::string("Error en shmat(): ") + std::string(strerror(errno)) );
    }
    return data_ptr;
}

void shm_dettach(void* data_ptr) {
    int errorDt = shmdt((void *) data_ptr);
    if (errorDt == -1) {
        THROW_UTIL( std::string("Error on shmdt(): ") + std::string(strerror(errno)) );
    }
}

void shm_remove(int shm_id) {
    if ( shmctl(shm_id,IPC_RMID, nullptr) == -1 ) {
        THROW_UTIL( std::string("Error en shmctl(): ") + std::string(strerror(errno)) );
    }
}