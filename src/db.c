#include "common.h"
#include "object.h"
#include "db.h"

/* Db->dict, keys are sds strings, vals are Redis objects. */
dictType dbDictType = {
    dictSdsHash,                /* hash function */
    NULL,                       /* key dup */
    NULL,                       /* val dup */
    dictSdsKeyCompare,          /* key compare */
    dictSdsDestructor,          /* key destructor */
    NULL, // dictObjectDestructor   /* val destructor */
};

uint64_t dictSdsHash(const void *key) {
    return dictGenHashFunction((unsigned char*)key, sdslen((char*)key));
}


int dictSdsKeyCompare(void *privdata, const void *key1,
        const void *key2)
{
    int l1,l2;
    DICT_NOTUSED(privdata);

    l1 = sdslen((sds)key1);
    l2 = sdslen((sds)key2);
    if (l1 != l2) return 0;
    return memcmp(key1, key2, l1) == 0;
}

void dictSdsDestructor(void *privdata, void *val)
{
    DICT_NOTUSED(privdata);

    sdsfree(val);
}


// void dictObjectDestructor(void *privdata, void *val)
// {
//     DICT_NOTUSED(privdata);

//     if (val == NULL) return; /* Lazy freeing will set value to NULL. */
//     decrRefCount(val);
// }



/* Add the key to the DB. It's up to the caller to increment the reference
 * counter of the value if needed.
 *
 * The program is aborted if the key already exists. */
void dbAdd(redisDb *db, robj *key, robj *val) {
    sds copy = sdsdup(key->ptr);
    int retval = dictAdd(db->dict, copy, val);
    serverAssertWithInfo(NULL,key,retval == DICT_OK);
}

/* Overwrite an existing key with a new value. Incrementing the reference
 * count of the new value is up to the caller.
 * This function does not modify the expire time of the existing key.
 *
 * The program is aborted if the key was not already present. */
void dbOverwrite(redisDb *db, robj *key, robj *val) {
    dictEntry *de = dictFind(db->dict,key->ptr);

    serverAssertWithInfo(NULL,key,de != NULL);
    dictEntry auxentry = *de;
    // robj *old = dictGetVal(de);

    dictSetVal(db->dict, de, val);

    dictFreeVal(db->dict, &auxentry);
}

/* High level Set operation. This function can be used in order to set
 * a key, whatever it was existing or not, to a new object.
 *
 * 1) The ref count of the value object is incremented.
 * 2) clients WATCHing for the destination key notified.
 * 3) The expire time of the key is reset (the key is made persistent).
 *
 * All the new keys in the database should be created via this interface. */
void setKey(redisDb *db, robj *key, robj *val) {
    if (lookupKeyWrite(db,key) == NULL) {
        dbAdd(db,key,val);
    } else {
        dbOverwrite(db,key,val);
    }
    incrRefCount(val);
    // removeExpire(db,key);
}

//递增
void incrKey(redisDb *db, robj *key) {
    robj* ret = lookupKeyWrite(db,key);
    robj* val = zmalloc(sizeof(robj));
    long value;
    sds ptr = sdsnew("");
    if (ret == NULL) {
        value = 1;
        sdsll2str(ptr, value);
        val->ptr = ptr;
        dbAdd(db,key,val);
    } else {
        value = atol(ret->ptr) + 1;
        sdsll2str(ptr, value);
        val->ptr = ptr;
        dbOverwrite(db,key,val);
    }
    // incrRefCount(val);
    // removeExpire(db,key);
}

//递减
void decrKey(redisDb *db, robj *key) {
    robj* ret = lookupKeyWrite(db,key);
    robj* val = zmalloc(sizeof(robj));
    long value;
    if (ret == NULL) {
        value = 0;
        val->ptr = (void*)((long)value);
        dbAdd(db,key,val);
    } else {
        value = (long)ret->ptr - 1;
        if(value <= 0){
            value = 0;
        }
        val->ptr = (void*)((long)(value));
        dbOverwrite(db,key,val);
    }
    // incrRefCount(val);
    // removeExpire(db,key);
}


/**
 * 客户端输入的字符串转为二进制安全字符串
 */
void clientBuf(sds *target, int argc, char **argv){
    sds cmd;
    int len, i;

    /* Construct command */
    cmd = sdsempty();
    cmd = sdscatfmt(cmd, "*%i\r\n", argc);
    for (i=0; i < argc; i++) {
        len = strlen(argv[i]);
        cmd = sdscatfmt(cmd, "$%u\r\n", len);
        cmd = sdscatlen(cmd, argv[i], len);
        cmd = sdscatlen(cmd, "\r\n", sizeof("\r\n")-1);
    }
    *target = cmd;
}


#ifdef DB_TEST_MAIN

int main(){

    //  // demo04
    // sds cmd;
    // // char* argv[] = {"get", "foo"};
    // char* argv[] = {"set", "foo", "111"};

    // // demo04
    // clientBuf(&cmd, 3, argv);

    // printf("%s\n", cmd);

    // // demo05
    // // sds recv = sdsnew("*2\r\n$3\r\nGET\r\n$3\r\nfoo\r\n");
    // int argc2;
    // sds *argv2;
    // argv2 = sdssplitargs(cmd,&argc2);

    // for (int i = 0; i < argc2; i++) {
    //     printf("%s\n", argv2[i]);
    // }
    // sdsfreesplitres(argv2,argc2);


   

    //demo03
    // sds out = sdsempty();
    // char *input = "*2\r\n$3\r\nGET\r\n$3\r\nfoo\r\n";
    // out = sdscatrepr(out, input, strlen(input));
    // out = sdscat(out,"\n");
    // printf("%s\n", out);

    // robj* one = createObject(OBJ_STRING,sdsnew("\r\n"));
    // robj* two = createObject(OBJ_STRING,sdsnew("+OK\r\n"));
    // robj* three = createObject(OBJ_STRING,sdsnew("-ERR\r\n"));

    // // demo02
    // redisDb* db = zmalloc(sizeof(redisDb));
    // db->dict = dictCreate(&dbDictType,NULL);

    // robj* key1 = zmalloc(sizeof(robj));
    // key1->ptr = sdsnew("one");

    // robj* key2 = zmalloc(sizeof(robj));
    // key2->ptr = sdsnew("two");

    // robj* val1 = zmalloc(sizeof(robj));
    // int* productId = 1234;
    // val1->ptr = productId;

    // robj* val2 = zmalloc(sizeof(robj));
    // sds name = sdsnew("tom");
    // val2->ptr = name;
    // setKey(db, key1, val1);
    // setKey(db, key1, val1);
    // setKey(db, key2, val2);

    // robj* ret = dictFetchValue(db->dict, sdsnew("one"));

    // printf("%d\n", (int*)(ret->ptr));

    //demo01
    // sds name2 = sdsnew("jack",10);

    // //没看出效果，好尴尬
    // sds name3 = sdsRemoveFreeSpace(name2);

}
#endif