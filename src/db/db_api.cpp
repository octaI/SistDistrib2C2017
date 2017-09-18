#include <constants.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include "../../include/db/db_api.h"

int execute_query(sqlite3 *database, std::string sql_query,callback_func callback, void* data, char* q_errmsg){
    int res = sqlite3_exec(database,sql_query.c_str(),callback,NULL,&q_errmsg);
    if (res != SQLITE_OK) {
        fprintf(stderr,"Error : %s",q_errmsg);
        sqlite3_free(q_errmsg);
    }

    return res;
}

void db_create(sqlite3*& database,std::string filename){
    int res = 0;
    res = sqlite3_open(filename.c_str(),&database);
    if (SQLITE_OK != res) { //error
        std::cerr << "Can't open database " << sqlite3_errmsg(database) << std::endl;
        sqlite3_close(database);
        exit(1);
    }
}

void db_close(sqlite3 *&database) {
    if(sqlite3_close(database) != 0) {
        perror("Error when closing the database connection \n");
        exit(1);
    }
}

void db_initialize(sqlite3 *&database) {
    std::string sql_query;
    sql_query = "SELECT * FROM Users;";
    char *q_errmsg = 0;
    if (sqlite3_exec(database,sql_query.c_str(),NULL,NULL,&q_errmsg) == SQLITE_OK) {
        fprintf(stdout,"Database has already been initialized");
        return;
    } //db already initialized
    sql_query = "CREATE TABLE Users ( "\
            "id integer PRIMARY KEY AUTOINCREMENT NOT NULL);"\
            "CREATE TABLE Rooms ( "\
            "id integer PRIMARY KEY AUTOINCREMENT NOT NULL );"\
            "CREATE TABLE Seats ("\
            "room_id  INT NOT NULL,"
            "seat_id integer  NOT NULL,"\
            "FOREIGN KEY(room_id) REFERENCES Rooms(id),"\
            "PRIMARY KEY (seat_id,room_id));"\
            "CREATE TABLE Reservations("\
            "user_id INT NOT NULL,"\
            "room_id INT NOT NULL,"\
            "seat_id INT NOT NULL,"\
            "reservation_date DATETIME DEFAULT CURRENT_TIMESTAMP,"\
            "FOREIGN KEY(user_id) REFERENCES Users(id),"\
            "FOREIGN KEY(room_id,seat_id) REFERENCES Seats(room_id,seat_id),"\
            "PRIMARY KEY(user_id,room_id,seat_id,reservation_date));";
    int res = sqlite3_exec(database,sql_query.c_str(),0,NULL,&q_errmsg);
    if (res != SQLITE_OK) {
        fprintf(stderr,"Error : %s",q_errmsg);
        sqlite3_free(q_errmsg);
    } else {
        fprintf(stdout, "Database succesfully initalized \n");
    }
}

int db_insert_user(sqlite3 *&database){
    char *q_errmsg = 0;
    std::string sql_query = "INSERT INTO Users VALUES(null);";
    return execute_query(database,sql_query.c_str(),NULL,NULL,q_errmsg);

}
void db_delete(sqlite3 *&database, std::string filename){
    if (database == NULL) return;
    if(remove(filename.c_str()) != 0) {
        perror("Error when deleting database file ");
        exit(1);
    }
    db_close(database);
}