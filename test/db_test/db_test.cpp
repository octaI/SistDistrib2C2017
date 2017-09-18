#include <fstream>
#include <iostream>
#include "../../include/db/db_api.h"

#define TEST_DB_FILENAME "test_db.db"

int db_create_test(){
    sqlite3* handle ;
    db_create(handle,TEST_DB_FILENAME);
    std::ifstream f(TEST_DB_FILENAME);
    if (!f.good()) {
        std::cerr << "Error in db creation test" << std::endl;
        db_close(handle);
        return -1;
    }
    db_delete(handle,TEST_DB_FILENAME);
    return 0;
}

static int show_select_users_callback(void* data,int argc,char** argv,char** col_name){
    for (int i = 0; i<argc;i++){
        std::cout <<"User " << col_name[i] << ": " << argv[i] ;
    }
    std::cout << std::endl;
    return 0;
}

static int show_select_rooms_callback(void* data,int argc,char** argv,char** col_name){
    for (int i = 0; i<argc;i++){
        std::cout <<"Room " << col_name[i] << ": " << argv[i] ;
    }
    std::cout << std::endl;
    return 0;
}

static int show_select_seats_callback(void* data,int argc,char** argv,char** col_name) {
    for (int i = 0; i < argc; i++){
        std::cout << col_name[i] << " " << argv[i] << " " ;
    }
    std::cout << std::endl;
    return 0;
}

static int show_select_reservations_callback(void* data,int argc,char** argv,char** col_name) {
    for (int i = 0; i < argc; i++){
        std::cout  << col_name[i] << " " << argv[i] << " ";
    }
    std::cout << std::endl;
    return 0;
}

int db_insert_user_test() {
    std::string sql_testquery;
    char *q_errmsg = 0;
    const char* data = "Callback function called";
    sqlite3* handle;
    db_create(handle,TEST_DB_FILENAME);
    db_initialize(handle);
    for (int i = 0; i <5;i++){
        if(db_insert_user(handle) != SQLITE_OK) std::cout << sqlite3_errmsg(handle) << std::endl;

    }
    sql_testquery = "SELECT * FROM Users;";
    callback_func my_func = show_select_users_callback;
    sqlite3_exec(handle,sql_testquery.c_str(),my_func,(void*)data,&q_errmsg);
    db_delete(handle,TEST_DB_FILENAME);
}

int db_insert_room_test(){
    std::string sql_testquery;
    char *q_errmsg = 0;
    const char* data = "Callback function called";
    sqlite3* handle;
    db_create(handle,TEST_DB_FILENAME);
    db_initialize(handle);
    for (int i = 0; i <3;i++){
        if(db_insert_room(handle) != SQLITE_OK) std::cout << sqlite3_errmsg(handle) << std::endl;

    }
    sql_testquery = "SELECT * FROM Rooms;";
    callback_func my_func = show_select_rooms_callback;
    sqlite3_exec(handle,sql_testquery.c_str(),my_func,(void*)data,&q_errmsg);
    db_delete(handle,TEST_DB_FILENAME);
}

int db_insert_seats_test(){
    std::string sql_testquery;
    char *q_errmsg = 0;
    const char* data = "Callback function called";
    sqlite3* handle;
    db_create(handle,TEST_DB_FILENAME);
    db_initialize(handle);
    for (int i = 0; i <3;i++){
        if(db_insert_room(handle) != SQLITE_OK) std::cout << sqlite3_errcode(handle) << std::endl;

    }
    db_insert_seats(handle,1,4);
    if(db_insert_seats(handle,1,5) != SQLITE_OK) std::cout << sqlite3_errmsg(handle);

    sql_testquery = "SELECT * FROM Seats;";
    callback_func my_func = show_select_seats_callback;
    sqlite3_exec(handle,sql_testquery.c_str(),my_func,(void*)data,&q_errmsg);
    db_delete(handle,TEST_DB_FILENAME);
}

int db_insert_reservations_test(){
    std::string sql_testquery;
    char *q_errmsg = 0;
    const char* data = "Callback function called";
    sqlite3* handle;
    db_create(handle,TEST_DB_FILENAME);
    db_initialize(handle);
    for (int i = 0; i <3;i++){
        if(db_insert_room(handle) != SQLITE_OK || db_insert_user(handle)) std::cout << sqlite3_errmsg(handle) << std::endl;
    }
    for (int i = 0; i <3;i++){
        if (db_insert_seats(handle,1,i+1) != SQLITE_OK) std::cerr << sqlite3_errmsg(handle) << std::endl;
    }

    db_insert_reservation(handle,1,1,1);
    db_insert_reservation(handle,1,1,2);
    callback_func my_func = show_select_reservations_callback;
    sql_testquery = "SELECT * FROM Reservations";
    sqlite3_exec(handle,sql_testquery.c_str(),my_func,&data,&q_errmsg);
}
int main(){
    std::cout << "Initiating DB tests" << std::endl;
    db_create_test();
    db_insert_user_test();
    db_insert_room_test();
    db_insert_seats_test();
    db_insert_reservations_test();
    return 0;
}