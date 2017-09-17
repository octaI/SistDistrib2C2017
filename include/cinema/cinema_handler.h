#ifndef DISTRIBUIDOS_CINEMA_HANDLER_H
#define DISTRIBUIDOS_CINEMA_HANDLER_H

#include <messages/message.h>

#define CINEMA_LISTENER_EXIT 1;

q_message cinema_handle(q_message message, int *exit);

#endif //DISTRIBUIDOS_CINEMA_HANDLER_H
