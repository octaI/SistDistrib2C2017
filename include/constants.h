#ifndef DISTRIBUIDOS_CONSTANTS_H
#define DISTRIBUIDOS_CONSTANTS_H


/* -------------------------------------------------------*/

#define THROW_UTIL(message) \
    std::cerr << std::string(__FILE__) << "::" << std::string(std::to_string(__LINE__)) << ": " << std::string(message) << std::endl; \
    exit(1);

#define QUEUE_COMMUNICATION_FILE "/bin/bash"
#define QUEUE_COMMUNICATION_CHAR 'a'

#define QUEUE_CINEMA_ADMIN_FILE "/bin/bash"
#define QUEUE_CINEMA_ADMIN_CHAR 'e'

#define QUEUE_ACTIVITY_FILE "/bin/bash"
#define QUEUE_ACTIVITY_CHAR 'h'

#define QUEUE_CLIENT_FILE "/bin/bash"
#define QUEUE_CLIENT_CHAR 'k'


/* ---Definition of channel constants between known communications in the system */

#define CINEMA_TO_CLIENT 1
#define CLIENT_TO_CINEMA 0

#define ADMIN_TO_CINEMA 1

/* ---Definition of message types constants---*/

#define ADMIN_REQUEST 1l

#define DATABASE_FILENAME "main_db.db"

#endif //DISTRIBUIDOS_CONSTANTS_H
