#ifndef DISTRIBUIDOS_DB_API_H
#define DISTRIBUIDOS_DB_API_H


#include "sqlite3.h"
#include <stdio.h>
#include <string>
#include <cstring>
#include <vector>
typedef int (*callback_func) (void*,int,char**,char**);

void db_create(sqlite3 *&database,std::string filename);

void db_initialize(sqlite3 *&database);
void db_close(sqlite3 *&database);
void db_delete(sqlite3 *&database,std::string filename);

int db_insert_user(sqlite3 *&database);
int db_insert_room(sqlite3 *&database);
int db_insert_seats(sqlite3 *&database, int roomid, int seatid);
int db_insert_reservation(sqlite3 *&database, int userid, int roomid, int seatid);
int db_insert_user_in_room(sqlite3 *&database, int userid, int roomid);

int db_remove_user_in_room(sqlite3 *&database, int userid);

void db_select_user(sqlite3 *database, int userid);
std::vector<int> db_select_users_in_room(sqlite3 *&database, int roomid);
std::vector<int> db_select_room(sqlite3 *&database);
std::vector<int> db_select_room_seats(sqlite3 *database,int roomid);
std::vector<int> db_select_reservations(sqlite3 *database,int roomid);



#endif //DISTRIBUIDOS_DB_API_H
