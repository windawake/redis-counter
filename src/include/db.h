#ifndef __DB_H
#define __DB_H

#include "common.h"

/* Error codes */
#define C_OK                    0
#define C_ERR                   -1

/* Static server configuration */
#define CONFIG_DEFAULT_DYNAMIC_HZ 1             /* Adapt hz to # of clients.*/
#define CONFIG_DEFAULT_HZ        10             /* Time interrupt calls/sec. */
#define CONFIG_MIN_HZ            1
#define CONFIG_MAX_HZ            500
#define MAX_CLIENTS_PER_CLOCK_TICK 200          /* HZ is adapted based on that. */
#define CONFIG_DEFAULT_SERVER_PORT        6379  /* TCP port. */
#define CONFIG_DEFAULT_TCP_BACKLOG       511    /* TCP listen backlog. */
#define CONFIG_DEFAULT_CLIENT_TIMEOUT       0   /* Default client timeout: infinite */
#define CONFIG_DEFAULT_DBNUM     16
#define CONFIG_MAX_LINE    1024
#define CRON_DBS_PER_CALL 16
#define NET_MAX_WRITES_PER_EVENT (1024*64)
#define PROTO_SHARED_SELECT_CMDS 10
#define OBJ_SHARED_INTEGERS 10000
#define OBJ_SHARED_BULKHDR_LEN 32
#define LOG_MAX_LEN    1024 /* Default maximum length of syslog messages.*/

struct sharedObjectsStruct {
    robj *crlf, *ok, *err, *emptybulk, *czero, *cone, *cnegone, *pong, *space,
    *colon, *nullbulk, *nullmultibulk, *queued,
    *emptymultibulk, *wrongtypeerr, *nokeyerr, *syntaxerr, *sameobjecterr,
    *outofrangeerr, *noscripterr, *loadingerr, *slowscripterr, *bgsaveerr,
    *masterdownerr, *roslaveerr, *execaborterr, *noautherr, *noreplicaserr,
    *busykeyerr, *oomerr, *plus, *messagebulk, *pmessagebulk, *subscribebulk,
    *unsubscribebulk, *psubscribebulk, *punsubscribebulk, *del, *unlink,
    *rpop, *lpop, *lpush, *rpoplpush, *zpopmin, *zpopmax, *emptyscan,
    *select[PROTO_SHARED_SELECT_CMDS],
    *integers[OBJ_SHARED_INTEGERS],
    *mbulkhdr[OBJ_SHARED_BULKHDR_LEN], /* "*<value>\r\n" */
    *bulkhdr[OBJ_SHARED_BULKHDR_LEN];  /* "$<value>\r\n" */
    sds minstring, maxstring;
};


uint64_t dictSdsHash(const void *key);


int dictSdsKeyCompare(void *privdata, const void *key1,
        const void *key2);

void dictSdsDestructor(void *privdata, void *val);


void dictObjectDestructor(void *privdata, void *val);



void setKey(redisDb *db, robj *key, robj *val);
void clientBuf(sds *target, int argc, char **argv);
void incrKey(redisDb *db, robj *key);
void decrKey(redisDb *db, robj *key);

#endif