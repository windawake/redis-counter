#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <time.h>

sqlite3* db;
int rc;

typedef struct counter_item{
    int key_id;
    char key_name[32];
    char key_value[32];
    char used_datetime[32];
} counter_item;

#define MAX_LINE 128

void init(){
    rc = sqlite3_open("/web/redis-counter/counter.db", &db);
}
int selectOneCallback(counter_item* item, int ncols, char**values, char** headers){
    for(int i=0; i<ncols; i++){
        // if(headers[i] == "key_id"){
        //     item->key_id =  values[i];
        // }

        // if(strncmp(headers[i],"key_name",8) == 0){
        //     printf("yes\n");
        //     strcpy(item->key_name, values[i]);
        // }
        if(strncmp(headers[i],"key_id",6) != 0){
            // strcpy(item->headers[i], values[i]);
        }
        

        // printf("%s %s\n", headers[i], values[i]);
    }

    return SQLITE_OK;
}

void selectOne(int keyId, counter_item* item){
    char* errMsg;
    /*声明sql语句存储变量*/
	char sql[128];
    memset(sql, 0 ,sizeof(sql));
    sprintf(sql , "select * from counter where key_id = '%d'", keyId);

    rc = sqlite3_exec(db, sql, selectOneCallback, item, &errMsg);

    if( rc != SQLITE_OK){
        printf("error\n");
    }
}

void selectMulti();
void saveOne(counter_item* item){
    sqlite3_stmt *stmt;
    const char *tail;
    char **dbRet;
    int ret, nRow , nCol;
    char* errMsg;
    char sql[MAX_LINE];

    memset(sql , 0 , sizeof(sql));
	sprintf(sql , "select * from counter where key_id = '%d'", item->key_id);
	ret = sqlite3_get_table(db , sql , &dbRet , &nRow , &nCol , &errMsg);
    
    if(nRow == 0){ /*执行insert操作*/
        memset(sql , 0 , sizeof(sql));
        sprintf(sql , "insert into counter (key_id, key_name, key_value, used_datetime) values (%d,'%s','%s','%s');",item->key_id , item->key_name , 
				item->key_value, item->used_datetime);
        
        ret = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
        if(ret != SQLITE_OK){
             printf("insert: %d\n", ret);
        }
    }else{ /*执行update操作*/
        memset(sql , 0 , sizeof(sql));
        sprintf(sql , "update counter set key_name ='%s', key_value = '%s', used_datetime = '%s' where key_id='%d'", item->key_name , 
				item->key_value, item->used_datetime, item->key_id);
        
        ret = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
        if(ret != SQLITE_OK){
             printf("update: %d\n", ret);
        }
    }

    printf("result: %d\n", ret);
}

void selectById(int keyId){
    char **dbRet;
    int ret, nRow , nCol , i , j , idx;
    char* errMsg;
    char sql[MAX_LINE], record[MAX_LINE];
    memset(sql , 0 , sizeof(sql));
	sprintf(sql , "select * from counter where key_id = '%d'", keyId);
	
	
	ret = sqlite3_get_table(db , sql , &dbRet , &nRow , &nCol , &errMsg);
    /*查询不成功*/
	if(ret != SQLITE_OK)
	{

    }

    idx = nCol;
    printf("%d %d %s\n", nRow, nCol, dbRet[5]);
	for(i=0; i<nRow; i++)
	{
		memset(record , 0 , MAX_LINE);
		sprintf(record , "%s %s %s %s", dbRet[idx] , dbRet[idx+1] , dbRet[idx+2], dbRet[idx+3]);
		printf("第%d条记录:%s\n",i,record);
		idx = idx + nCol;
	}//for
}

void datetimeOfNow(char* buf){
    time_t rawtime;
    struct tm *info;
    time(&rawtime);
    info = localtime(&rawtime);
    strftime(buf, 80, "%Y-%m-%d %H:%M:%S", info);
}

int main(){
    int keyId = 1;

    init();
    counter_item* item = malloc(sizeof(counter_item));
    // keyId = ITEM_PROP(item, "key_id");
    // printf("%d\n", item->key_id);
    // return 1;
    // selectOne(keyId, item);
    // printf("%s\n", item->key_name);

    // selectById(keyId);
    // selectById(keyId);

    char bufTime[80];
    datetimeOfNow(bufTime);
    item->key_id = 2;
    strcpy(item->key_name, "two");
    strcpy(item->key_value, "donkey");
    strcpy(item->used_datetime, bufTime);

    saveOne(item);
}