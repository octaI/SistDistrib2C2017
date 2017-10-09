#include "../../include/db/db_api.h"
#include "../../include/constants.h"

int main(){
    sqlite3 *handler;
    std::string filename = std::string(DATABASE_FILENAME);
    //db_delete(handler, filename);

    sqlite3_open(filename.c_str(),&handler);

    db_initialize(handler);

    //db_insert_user(handler);
    db_insert_user(handler);
    db_insert_user(handler);

    db_insert_room(handler);
    db_insert_room(handler);
    db_insert_room(handler);
    for (int i = 0; i < 20; i++){
        db_insert_seats(handler,1,i+1);
    }
    for (int i = 0; i < 10; i++){
        db_insert_seats(handler,2,i+1);
    }
    for (int i =0; i < 15;i++){
        db_insert_seats(handler,3,i+1);
    }
    sqlite3_close(handler);
    return 0;
}

