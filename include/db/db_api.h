#ifndef DISTRIBUIDOS_DB_API_H
#define DISTRIBUIDOS_DB_API_H


#include "sqlite3.h"
#include <stdio.h>
#include <string>

typedef int (*callback_func) (void*,int,char**,char**);

void db_create(sqlite3 *&database,std::string filename);

void db_initialize(sqlite3 *&database);
void db_close(sqlite3 *&database);
void db_delete(sqlite3 *&database,std::string filename);

int db_insert_user(sqlite3 *&database);
void db_select_user(sqlite3 *database, int userid);
void db_select_rooms(sqlite3 *database);
void db_select_reservations(sqlite3 *database, int userid, int roomid);



#endif //DISTRIBUIDOS_DB_API_H
