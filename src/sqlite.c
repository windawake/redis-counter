#include <sqlite3.h>
#include "config.h"
#include "error-user.h"


sqlite3* db;
int rc;

typedef struct counter_item{
    int key_id;
    char key_name[32];
    char key_value[32];
    char used_datetime[32];
} counter_item;

#define MAX_SQL_LINE 256


void init(){
    rc = sqlite3_open("/web/redis-counter/counter.db", &db);
}

void selectOne(int keyId, counter_item* item){
    char **dbRet;
    int ret, nRow, nCol, idx;
    char* errMsg;
    char sql[MAX_SQL_LINE];
    memset(sql , 0 , sizeof(sql));
	sprintf(sql , "select * from counter where key_id = '%d'", keyId);
	
	
	ret = sqlite3_get_table(db , sql , &dbRet , &nRow , &nCol , &errMsg);
    /*查询不成功*/
	if(ret != SQLITE_OK)
	{
        errExit("select failed");
    }
    if(nRow == 0){
        errExit("select record not find");
    }
    if(nRow > 1){
        errExit("nRow more than 1");
    }

    idx = nCol;
    item->key_id = atoi(dbRet[idx]);
    strcpy(item->key_name, dbRet[idx+1]);
    strcpy(item->key_value, dbRet[idx+2]);
    strcpy(item->used_datetime, dbRet[idx+3]);
}

void selectMulti();
void saveOne(counter_item* item){
    char **dbRet;
    int ret, nRow , nCol;
    char* errMsg;
    char sql[MAX_SQL_LINE];

    memset(sql , 0 , sizeof(sql));
	sprintf(sql , "select * from counter where key_id = '%d'", item->key_id);
	ret = sqlite3_get_table(db , sql , &dbRet , &nRow , &nCol , &errMsg);
    if(ret != SQLITE_OK)
	{
        errExit("select failed:");
    }
    
    if(nRow == 0){ /*执行insert操作*/
        memset(sql , 0 , sizeof(sql));
        sprintf(sql , "insert into counter (key_id, key_name, key_value, used_datetime) values (%d,'%s','%s','%s');",item->key_id , item->key_name , 
				item->key_value, item->used_datetime);
        
        ret = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
        if(ret != SQLITE_OK){
             errExit("insert error");
        }
    }else{ /*执行update操作*/
        memset(sql , 0 , sizeof(sql));
        sprintf(sql , "update counter set key_name ='%s', key_value = '%s', used_datetime = '%s' where key_id='%d'", item->key_name , 
				item->key_value, item->used_datetime, item->key_id);
        
        ret = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
        if(ret != SQLITE_OK){
             errExit("update error");
        }
    }

}


void datetimeOfNow(char* buf){
    time_t rawtime;
    struct tm *info;
    time(&rawtime);
    info = localtime(&rawtime);
    strftime(buf, 80, "%Y-%m-%d %H:%M:%S", info);
}

int main(){
    int keyId = 2;

    init();
    counter_item* item = malloc(sizeof(counter_item));
    
    selectOne(keyId, item);
    printf("%s\n", item->key_name);

    // selectById(keyId);
    // selectById(keyId);

    // insert or update
    // char bufTime[80];
    // datetimeOfNow(bufTime);
    // item->key_id = 2;
    // strcpy(item->key_name, "two");
    // strcpy(item->key_value, "donkey");
    // strcpy(item->used_datetime, bufTime);

    // saveOne(item);
}