#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

int callback(void* data, int ncols, char**values, char** headers){
    for(int i=0; i<ncols; i++){
        printf("%s %s\n", headers[i], values[i]);
    }
    return SQLITE_OK;
}

int main(){
    sqlite3* db;
    char* errMsg;
    int rc = sqlite3_open("/web/redis-counter/counter.db", &db);
    char* sql = "select * from counter";
    rc = sqlite3_exec(db, sql, callback, NULL, &errMsg);

    if( rc != SQLITE_OK){
        printf("error\n");
    }
}