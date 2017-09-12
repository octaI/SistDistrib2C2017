#include <iostream>
#include <cstring>
#include <sys/unistd.h>
#include <sys/wait.h>
#include "../include/ipc/communicationqueue.h"
#include "../include/messages/message.h"


int test_msg_transmission(){
    std::string test_string = "Hola, soy un mensaje de prueba";
    commqueue test_queue;
    q_message test_msg;
    test_queue = create_commqueue("/bin/bash",'a');

    if (fork() == 0){ //i'm the child
        test_queue.orientation = 0;
        test_msg = receive_message(test_queue,10);
        std::cout << strcmp(test_msg.message_choice.m0.test_msg,test_string.c_str()) << std::endl;
        strcmp(test_msg.message_choice.m0.test_msg,test_string.c_str()) == 0 ?  exit(0) : exit(-1);
        std::cout << "CLIENT RECEIVED: " << test_msg.message_choice.m0.test_msg << std::endl;
    } else {
        //i'm the parent
        test_queue.orientation = 1;
        test_msg.message_type = 10;
        strcpy(test_msg.message_choice.m0.test_msg,test_string.c_str());
        send_message(test_queue,test_msg);
        std::cout << "MANDE EL MSG" << std::endl;
        int child_status;
        wait(&child_status);
        child_status =WEXITSTATUS(child_status);
        std::cout << "CLIENT FINISHED WITH STATUS " << child_status << std::endl;
        if(child_status == 0){
            delete_queue(test_queue);
            return 0;
        }else{
            delete_queue(test_queue);
            return -1;
        }
    }

}

int main(){
    std::cout << "***EXECUTING QUEUE TESTS***" << std::endl;
    if (test_msg_transmission() != 0){
        std::cout << "ERROR MSG TRANSMISSION TEST" << std::endl;
        exit(2);
    }

    return 0;

}
