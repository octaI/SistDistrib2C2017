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

#define QUEUE_ADMIN_CLIENT_FILE "/bin/bash"
#define QUEUE_ADMIN_CLIENT_CHAR 'u'

#define SHM_CLIENT_FILE "/bin/bash"
#define SHM_CLIENT_CHAR 'p'
#define SHM_CLIENT_SIZE sizeof(int)


#define SHM_CINEMA_TIMER_FILE "/bin/bash"
#define SHM_CINEMA_TIMER_CHAR 't'
#define SHM_CINEMA_TIMER_SIZE sizeof(int)

/* ---Definition of channel constants between known communications in the system */

#define CINEMA_TO_CLIENT 1
#define CLIENT_TO_CINEMA 0

#define ADMIN_TO_CINEMA 1

/* ---Definition of message types constants---*/

#define ADMIN_REQUEST 1

#define DATABASE_FILENAME "main_db.db"

#endif //DISTRIBUIDOS_CONSTANTS_H
