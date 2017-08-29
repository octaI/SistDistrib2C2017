#ifndef SISTDISTRIB2C2017_QUEUE_H
#define SISTDISTRIB2C2017_QUEUE_H

#include <string>

int queue_create(const std::string& file, const char letter);

void queue_send(int queue_id, const void* buf, int bufSize, int msgType);

void queue_receive(int queue_id, void* buf, int bufSize, int msgType);

void queue_destroy(int queue_id);

#endif //SISTDISTRIB2C2017_QUEUE_H
