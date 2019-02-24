#include "common.h"
#include "object.h"
#include "db.h"
#include "command.h"

extern dictType dbDictType;


void commandServer(redisDb* db, char* inbuf, sds outbuf){
    int argc;
    sds *argv;
    argv = sdssplitargs(inbuf,&argc);
    sds action;

    robj* cKey = zmalloc(sizeof(robj));
    robj* cVal = zmalloc(sizeof(robj));

    // 解析输入命令
    for (int index = 2; index < argc; index+=2) {
        if(index == 2){
            action = sdsnew(argv[index]);
        }else if(index == 4){
            
            cKey->ptr = argv[index];
        }else if(index == 6){
            cVal->ptr = argv[index];
        }
    }

    // 命令处理方法
    if(sdscmp(action, sdsnew("get")) == 0){
        robj* ret = dictFetchValue(db->dict, sdsnew(cKey->ptr));
        //如果key不存在
        if(ret == NULL){
            sdscpy(outbuf, "missing key");
            sdscatlen(outbuf,"\n",1);
            return;
        }else{
            sdscpy(outbuf, ret->ptr);
            sdscatlen(outbuf,"\n",1);
        }
        
    }else if(sdscmp(action, sdsnew("set")) == 0){
        setKey(db, cKey, cVal);
        sdscpy(outbuf, "ok");
        sdscatlen(outbuf,"\n",1);
    }else if(sdscmp(action, sdsnew("incr")) == 0){
        incrKey(db, cKey);
        sdscpy(outbuf, "ok");
        sdscatlen(outbuf,"\n",1);
    }else if(sdscmp(action, sdsnew("decr")) == 0){
        decrKey(db, cKey);
        sdscpy(outbuf, "ok");
        sdscatlen(outbuf,"\n",1);
    }
}

#ifdef COMMAND_TEST_MAIN

int main(){
    // 实例化redisDb
	redisDb* db = zmalloc(sizeof(redisDb));
    db->dict = dictCreate(&dbDictType,NULL);

    // robj* ret = dictFetchValue(db->dict, sdsnew("sleeping"));

    // robj* key2 = zmalloc(sizeof(robj));
    // key2->ptr = sdsnew("two");
    // robj* val2 = zmalloc(sizeof(robj));
    // sds name = sdsnew("tom");
    // sdsll2str(name, 111);
    // val2->ptr = name;

    // robj* key3 = zmalloc(sizeof(robj));
    // key3->ptr = sdsnew("three");
    // robj* val3 = zmalloc(sizeof(robj));
    // val3->ptr = sdsnew("hello");

    
    // setKey(db, key2, val2);
    // setKey(db, key3, val3);
    

    char inbuf[32] = "*2\r\n$3\r\nget\r\n$3\r\ntwo\r\n";
    sds outbuf = sdsnewlen("", 100);
    commandServer(db, inbuf, outbuf);
    printf("%s", outbuf);

}

#endif