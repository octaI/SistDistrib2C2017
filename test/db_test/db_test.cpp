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
        std::cout <<"User " << col_name[i] << ": " << argv[i] << std::endl;
    }

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

int main(){
    std::cout << "Initiating DB tests" << std::endl;
    db_create_test();
    db_insert_user_test();
    return 0;
}