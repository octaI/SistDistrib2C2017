#ifndef DISTRIBUIDOS_SHAREDMEMORY_H
#define DISTRIBUIDOS_SHAREDMEMORY_H

#include <cstdio>

int shm_create(const char* file, char letter, size_t data_size);

void* shm_get_data(const char* file, char letter, size_t data_size);

void shm_dettach(void* prt_data);

void shm_remove(int shm_id);

#endif //DISTRIBUIDOS_SHAREDMEMORY_H
