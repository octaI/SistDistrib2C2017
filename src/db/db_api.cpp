#include <string>
#include <iostream>
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
    char* q_errmsg = 0;
    std::string sql_query;
     //db already initialized
    sql_query ="PRAGMA foreign_key=ON;"\
            "CREATE TABLE IF NOT EXISTS Users ( "\
            "id integer PRIMARY KEY AUTOINCREMENT NOT NULL);"\
            "CREATE TABLE IF NOT EXISTS Rooms ( "\
            "id integer PRIMARY KEY AUTOINCREMENT NOT NULL );"\
            "CREATE TABLE IF NOT EXISTS Seats ("\
            "room_id  integer NOT NULL,"\
            "seat_id integer NOT NULL,"\
            "FOREIGN KEY(room_id) REFERENCES Rooms(id),"\
            "PRIMARY KEY (room_id,seat_id));"\
            "CREATE TABLE IF NOT EXISTS Reservations("\
            "user_id integer NOT NULL,"\
            "room_id integer NOT NULL,"\
            "seat_id integer NOT NULL,"\
            "reservation_date DATETIME DEFAULT(STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW')),"\
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

int db_insert_reservation(sqlite3 *&database, int userid, int roomid, int seatid) {
    char *q_errmsg= 0;
    const char *zSql;
    sqlite3_stmt *stmt;
    std::string sql_query = "INSERT INTO Reservations(user_id,room_id,seat_id) VALUES(?,?,?)";
    int rc = sqlite3_prepare(database,sql_query.c_str(),strlen(sql_query.c_str()),&stmt,&zSql);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt,1,userid);
        sqlite3_bind_int(stmt,2,roomid);
        sqlite3_bind_int(stmt,3,seatid);
    }

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc;
}

int db_insert_room(sqlite3 *&database){
    char *q_errmsg = 0;
    std::string sql_query = "INSERT INTO Rooms VALUES(null);";
    return execute_query(database,sql_query.c_str(),NULL,NULL,q_errmsg);
}

int db_insert_seats(sqlite3 *&database, int roomid, int seatid ){
    char *q_errmsg=0;
    const char *pzTest;
    sqlite3_stmt *stmt;
    std::string sql_query = "INSERT INTO Seats(room_id,seat_id) VALUES(?,?)";
    int rc = sqlite3_prepare(database,sql_query.c_str(),strlen(sql_query.c_str()),&stmt,&pzTest);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt,1,roomid);
        sqlite3_bind_int(stmt,2,seatid);
    }
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc;
}

static int db_select_singlevalue_callback(void *data,int argc,char** argv,char** col_name){
    std::vector<int> *data_as_array = reinterpret_cast<std::vector<int> *>(data);
    data_as_array->push_back(atoi(argv[0]));
    return 0;
}

std::vector<int> db_select_room(sqlite3 *&database){
    std::vector<int> res_vec = {};
    char *q_errmsg = 0;
    std::string sql_query = "SELECT id FROM Rooms;";
    callback_func my_func = db_select_singlevalue_callback;
    sqlite3_exec(database,sql_query.c_str(),my_func,&res_vec,&q_errmsg);

    return res_vec;
}

std::vector<int> db_select_room_seats(sqlite3 *database, int roomid){
    std::vector<int> res_vec = {};
    char *q_errmsg = 0;
    std::string sql_query = "SELECT seat_id FROM Seats WHERE room_id =" + std::to_string(roomid)+ ";";
    callback_func my_func = db_select_singlevalue_callback;
    sqlite3_exec(database,sql_query.c_str(),my_func,&res_vec,&q_errmsg);
    return res_vec;
}

std::vector<int> db_select_reservations(sqlite3* database, int roomid) {
    std::vector<int> res_vec = {};
    char *q_errmsg = 0;
    std::string sql_query = "SELECT seat_id FROM Reservations WHERE room_id =" + std::to_string(roomid) + ";";
    callback_func my_func = db_select_singlevalue_callback;
    sqlite3_exec(database,sql_query.c_str(),my_func,&res_vec,&q_errmsg);

    return res_vec;
}


void db_delete(sqlite3 *&database, std::string filename){
    if (database == NULL) return;
    if(remove(filename.c_str()) != 0) {
        perror("Error when deleting database file ");
        exit(1);
    }
    db_close(database);
}