#include "../../include/ipc/queue.h"
#include "../../include/constants.h"

#include <sys/msg.h>
#include <cstring>
#include "string"
#include <iostream>


int queue_create(const std::string& file, const char letter) {
    key_t key = ftok(file.c_str(),letter);
    if (key == -1) {
        THROW_UTIL(strerror(errno));
    }
    int queue_id = msgget(key, IPC_CREAT | 0660 );
    if (queue_id == -1 ) {
        THROW_UTIL(std::string(strerror(errno)));
    }
    return queue_id;
}

void queue_send(int queue_id, const void* buf, int bufSize, int msgType) {
    struct msgbuf* msg = (struct msgbuf*)malloc(sizeof(struct msgbuf) + bufSize);
    msg->mtype = msgType;
    memcpy(msg->mtext, buf, (size_t)bufSize);
    int status = msgsnd(queue_id, msg, (size_t)bufSize, 0);
    free(msg);
    if (status  == -1) {
        THROW_UTIL(std::string(strerror(errno)));
    }
}

void queue_receive(int queue_id, void* buf, int bufSize, int msgType) {
    struct msgbuf* msg = (struct msgbuf*)malloc(sizeof(struct msgbuf) + bufSize);
    int status = (int)msgrcv(queue_id, msg, (size_t)bufSize, msgType, 0);
    memcpy(buf, msg->mtext, (size_t) bufSize);
    free(msg);
    if (status  == -1) {
        THROW_UTIL(std::string(strerror(errno)));
    }
}



void queue_destroy(int queue_id) {
    int status = msgctl(queue_id, IPC_RMID, 0);
    if (status  == -1) {
        THROW_UTIL(std::string(strerror(errno)));
    }
}
