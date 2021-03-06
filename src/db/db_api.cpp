#include <string>
#include <iostream>
#include <messages/message.h>
#include "../../include/db/db_api.h"

int execute_query(sqlite3 *database, std::string sql_query,callback_func callback, void* data, char* q_errmsg){
    int res = sqlite3_exec(database,sql_query.c_str(),callback,data,&q_errmsg);
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
            "paid_flag integer NOT NULL DEFAULT(0),"
            "FOREIGN KEY(user_id) REFERENCES Users(id),"\
            "FOREIGN KEY(room_id,seat_id) REFERENCES Seats(room_id,seat_id),"\
            "PRIMARY KEY(room_id,seat_id));"\
            "CREATE TABLE IF NOT EXISTS Clients_rooms("\
            "user_id integer NOT NULL,"\
            "room_id integer NOT NULL,"\
            "FOREIGN KEY(user_id) REFERENCES Users(id),"\
            "FOREIGN KEY(room_id) REFERENCES Rooms(id),"\
            "PRIMARY KEY(user_id,room_id)"\
            ");"\
            "CREATE TABLE IF NOT EXISTS Active_clients("\
            "user_id integer NOT NULL,"\
            "FOREIGN KEY(user_id) REFERENCES Users(id)"\
            "PRIMARY KEY(user_id)"\
            ");";
    int res = sqlite3_exec(database,sql_query.c_str(),0,NULL,&q_errmsg);
    if (res != SQLITE_OK) {
        fprintf(stderr,"Error : %s",q_errmsg);
        sqlite3_free(q_errmsg);
    } else {
        fprintf(stdout, "[CINEMA-DB] Database succesfully initalized \n");
    }
}

int db_remove_user_unpaid_reservations(sqlite3 *&database, int userid){
    char *q_errmsg = 0;
    std::string sql_query = "DELETE FROM Reservations WHERE user_id="+std::to_string(userid)+" AND paid_flag = 0;";
    int res=execute_query(database,sql_query.c_str(),NULL,NULL,q_errmsg);
    return (res == SQLITE_OK);
}

int db_remove_unpaid_reservations(sqlite3 *&database){
    char *q_errmsg = 0;
    std::string sql_query = "DELETE FROM Reservations WHERE paid_flag = 0;";
    int res=execute_query(database,sql_query.c_str(),NULL,NULL,q_errmsg);
    return (res == SQLITE_OK);
}

int db_update_paid_reservation(sqlite3 *&database,int user_id, Reservation user_reservation) {
    char *q_errmsg = 0;
    int room_id = user_reservation.room;
    int seat_id = user_reservation.seat_num;
    std::string sql_query = "UPDATE  Reservations SET paid_flag = 1 WHERE user_id=" + std::to_string(user_id) +" AND room_id= " +
            std::to_string(room_id)+" AND seat_id=" + std::to_string(seat_id) +";";
    int res = execute_query(database,sql_query.c_str(),NULL,NULL,q_errmsg);
    return (res == SQLITE_OK);
}

int db_logout_user(sqlite3 *&database, int user_id) {
    char *q_errmsg = 0;
    std::string sql_query = "DELETE FROM Active_clients WHERE user_id=" + std::to_string(user_id) +";";
    int res = execute_query(database,sql_query.c_str(),NULL,NULL,q_errmsg);
    return (res == SQLITE_OK);
}

int db_login_user(sqlite3 *&database, int user_id) {
    char *q_errmsg = 0;
    std::string sql_query ="INSERT INTO Active_clients VALUES(" + std::to_string(user_id) + ");";
    int res = execute_query(database,sql_query.c_str(),NULL,NULL,q_errmsg);
    return (res == SQLITE_OK);
}

int db_insert_user(sqlite3 *&database){
    char *q_errmsg = 0;
    std::string sql_query = "INSERT INTO Users VALUES(null);";
    int res = execute_query(database,sql_query.c_str(),NULL,NULL,q_errmsg);
    return (int)sqlite3_last_insert_rowid(database);
}

int db_insert_reservation(sqlite3 *&database, int userid, int roomid, int seatid) {
    int rc;
    if (seatid > 0) {
        char *q_errmsg= 0;
        const char *zSql;
        sqlite3_stmt *stmt;
        std::string sql_query = "INSERT INTO Reservations(user_id,room_id,seat_id) VALUES(?,?,?)";
        rc = sqlite3_prepare(database,sql_query.c_str(),strlen(sql_query.c_str()),&stmt,&zSql);

        if (rc == SQLITE_OK) {
            sqlite3_bind_int(stmt,1,userid);
            sqlite3_bind_int(stmt,2,roomid);
            sqlite3_bind_int(stmt,3,seatid);
        }

        sqlite3_step(stmt);
        rc =sqlite3_finalize(stmt);
    } else {
        rc = -1;
    }

    return (rc == SQLITE_OK);
}

int db_insert_room(sqlite3 *&database){
    char *q_errmsg = 0;
    std::string sql_query = "INSERT INTO Rooms VALUES(null);";
    return (execute_query(database,sql_query.c_str(),NULL,NULL,q_errmsg) == SQLITE_OK);
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
    rc =sqlite3_finalize(stmt);
    return (rc == SQLITE_OK);
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

int db_insert_user_in_room(sqlite3 *&database, int userid, int roomid) {
    char *q_errmsg = 0;
    std::string sql_query = "INSERT INTO Clients_rooms(user_id,room_id) VALUES(" + std::to_string(userid) + ","
                            + std::to_string(roomid) + ");";

    return (execute_query(database,sql_query,NULL,NULL,q_errmsg) == SQLITE_OK);
}

int db_remove_user_in_room(sqlite3 *&database, int userid){
    char *q_errmsg = 0;
    std::string userid_str = std::to_string(userid);
    std::string sql_query = "DELETE FROM Clients_rooms WHERE user_id="+userid_str+";";

    return (execute_query(database,sql_query,NULL,NULL,q_errmsg) == SQLITE_OK);
}

int db_remove_users_in_room(sqlite3 *&database){
    char *q_errmsg = 0;
    std::string sql_query = "DELETE FROM Clients_rooms;";
    return (execute_query(database,sql_query,NULL,NULL,q_errmsg) == SQLITE_OK);
}

int db_select_user_current_room(sqlite3 *&database, int userid){
    std::vector<int> res = {};
    char *q_errmsg = 0;
    std::string sql_query = "SELECT room_id FROM Clients_Rooms WHERE user_id=" + std::to_string(userid) + ";";
    execute_query(database,sql_query.c_str(),db_select_singlevalue_callback,&res,q_errmsg);
    return res.front();
}
std::vector<int> db_select_users_in_room(sqlite3 *&database, int roomid) {
    std::vector<int> res = {};
    char *q_errmsg = 0;
    std::string sql_query = "SELECT user_id FROM Clients_rooms WHERE room_id =" + std::to_string(roomid) + ";";
    execute_query(database,sql_query.c_str(),db_select_singlevalue_callback,&res,q_errmsg);

    return res;
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
    if ( remove(filename.c_str()) != 0 ) {
        perror("Error when deleting database file ");
        exit(1);
    }
    db_close(database);
}