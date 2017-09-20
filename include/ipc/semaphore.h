#ifndef DISTRIBUIDOS_SEMAPHORE_H
#define DISTRIBUIDOS_SEMAPHORE_H

int semaphore_create(const char* file, char letter, int init_value);

int semaphore_get(const char* file, char letter);

void semaphore_wait(int sem_id);

void semaphore_signal(int sem_id);

void semaphore_delete(int sem_id);

#endif //DISTRIBUIDOS_SEMAPHORE_H
