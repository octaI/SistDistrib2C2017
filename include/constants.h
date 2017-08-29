#ifndef DISTRIBUIDOS_CONSTANTS_H
#define DISTRIBUIDOS_CONSTANTS_H

#define THROW_UTIL(message) std::cerr << std::string(__FILE__) << "::" << std::string(std::to_string(__LINE__)) << ": " << std::string(message) << std::endl; abort();


#endif //DISTRIBUIDOS_CONSTANTS_H
