#include "../../include/db/db_api.h"
#include "../../include/constants.h"

int main(){
    sqlite3 *handler;
    std::string filename = "../admin/" + std::string(DATABASE_FILENAME);
    sqlite3_open(filename.c_str(),&handler);
    db_insert_user(handler);
    db_insert_user(handler);
    db_insert_room(handler);
    db_insert_room(handler);
    db_insert_room(handler);
}

