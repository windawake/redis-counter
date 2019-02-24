#include "redis_assert.h"
#include "common.h"

#include "object.h"

#define PROTO_REPLY_CHUNK_BYTES (16*1024) /* 16k output buffer */

/* With multiplexing we need to take per-client state.
 * Clients are taken in a linked list. */
typedef struct client {
    uint64_t id;            /* Client incremental unique ID. */
    int fd;                 /* Client socket. */
    redisDb *db;            /* Pointer to currently SELECTed DB. */
    robj *name;             /* As set by CLIENT SETNAME. */
    sds querybuf;           /* Buffer we use to accumulate client queries. */
    size_t qb_pos;          /* The position we have read in querybuf. */
    sds pending_querybuf;   /* If this client is flagged as master, this buffer
                               represents the yet not applied portion of the
                               replication stream that we are receiving from
                               the master. */
    size_t querybuf_peak;   /* Recent (100ms or more) peak of querybuf size. */
    int argc;               /* Num of arguments of current command. */
    robj **argv;            /* Arguments of current command. */
    struct redisCommand *cmd, *lastcmd;  /* Last command executed. */
    int reqtype;            /* Request protocol type: PROTO_REQ_* */
    int multibulklen;       /* Number of multi bulk arguments left to read. */
    long bulklen;           /* Length of bulk argument in multi bulk request. */
    list *reply;            /* List of reply objects to send to the client. */
    unsigned long long reply_bytes; /* Tot bytes of objects in reply list. */
    size_t sentlen;         /* Amount of bytes already sent in the current
                               buffer or object being sent. */
    time_t ctime;           /* Client creation time. */
    time_t lastinteraction; /* Time of the last interaction, used for timeout */
    time_t obuf_soft_limit_reached_time;
    int flags;              /* Client flags: CLIENT_* macros. */
    int authenticated;      /* When requirepass is non-NULL. */
    int replstate;          /* Replication state if this is a slave. */
    int repl_put_online_on_ack; /* Install slave write handler on ACK. */
    int repldbfd;           /* Replication DB file descriptor. */
    off_t repldboff;        /* Replication DB file offset. */
    off_t repldbsize;       /* Replication DB file size. */
    sds replpreamble;       /* Replication DB preamble. */
    long long read_reploff; /* Read replication offset if this is a master. */
    long long reploff;      /* Applied replication offset if this is a master. */
    long long repl_ack_off; /* Replication ack offset, if this is a slave. */
    long long repl_ack_time;/* Replication ack time, if this is a slave. */
    long long psync_initial_offset; /* FULLRESYNC reply offset other slaves
                                       copying this slave output buffer
                                       should use. */
    int btype;              /* Type of blocking op if CLIENT_BLOCKED. */
    long long woff;         /* Last write global replication offset. */
    list *watched_keys;     /* Keys WATCHED for MULTI/EXEC CAS */
    dict *pubsub_channels;  /* channels a client is interested in (SUBSCRIBE) */
    list *pubsub_patterns;  /* patterns a client is interested in (SUBSCRIBE) */
    sds peerid;             /* Cached peer ID. */
    listNode *client_list_node; /* list node in client list */

    /* Response buffer */
    int bufpos;
    char buf[PROTO_REPLY_CHUNK_BYTES];
} client;

void _serverAssert(const char *estr,const char *file, int line){
    printf("%s:%d:%s\n",file, line , estr);
}

void _serverPanic(const char *file, int line, const char *msg, ...){
    va_list ap;
    va_start(ap,msg);
    char fmtmsg[256];
    vsnprintf(fmtmsg,sizeof(fmtmsg),msg,ap);
    va_end(ap);

    printf("%s:%d:%s\n",file, line , fmtmsg);
}

// void _serverAssertPrintClientInfo(const client *c) {
//     int j;

//     bugReportStart();
//     serverLog(LL_WARNING,"=== ASSERTION FAILED CLIENT CONTEXT ===");
//     serverLog(LL_WARNING,"client->flags = %d", c->flags);
//     serverLog(LL_WARNING,"client->fd = %d", c->fd);
//     serverLog(LL_WARNING,"client->argc = %d", c->argc);
//     for (j=0; j < c->argc; j++) {
//         char buf[128];
//         char *arg;

//         if (c->argv[j]->type == OBJ_STRING && sdsEncodedObject(c->argv[j])) {
//             arg = (char*) c->argv[j]->ptr;
//         } else {
//             snprintf(buf,sizeof(buf),"Object type: %u, encoding: %u",
//                 c->argv[j]->type, c->argv[j]->encoding);
//             arg = buf;
//         }
//         serverLog(LL_WARNING,"client->argv[%d] = \"%s\" (refcount: %d)",
//             j, arg, c->argv[j]->refcount);
//     }
// }

// void serverLogObjectDebugInfo(const robj *o) {
//     serverLog(LL_WARNING,"Object type: %d", o->type);
//     serverLog(LL_WARNING,"Object encoding: %d", o->encoding);
//     serverLog(LL_WARNING,"Object refcount: %d", o->refcount);
//     if (o->type == OBJ_STRING && sdsEncodedObject(o)) {
//         serverLog(LL_WARNING,"Object raw string len: %zu", sdslen(o->ptr));
//         if (sdslen(o->ptr) < 4096) {
//             sds repr = sdscatrepr(sdsempty(),o->ptr,sdslen(o->ptr));
//             serverLog(LL_WARNING,"Object raw string content: %s", repr);
//             sdsfree(repr);
//         }
//     } else if (o->type == OBJ_LIST) {
//         serverLog(LL_WARNING,"List length: %d", (int) listTypeLength(o));
//     } else if (o->type == OBJ_SET) {
//         serverLog(LL_WARNING,"Set size: %d", (int) setTypeSize(o));
//     } else if (o->type == OBJ_HASH) {
//         serverLog(LL_WARNING,"Hash size: %d", (int) hashTypeLength(o));
//     } else if (o->type == OBJ_ZSET) {
//         serverLog(LL_WARNING,"Sorted set size: %d", (int) zsetLength(o));
//         if (o->encoding == OBJ_ENCODING_SKIPLIST)
//             serverLog(LL_WARNING,"Skiplist level: %d", (int) ((const zset*)o->ptr)->zsl->level);
//     }
// }

// void _serverAssertPrintObject(const robj *o) {
//     bugReportStart();
//     serverLog(LL_WARNING,"=== ASSERTION FAILED OBJECT CONTEXT ===");
//     serverLogObjectDebugInfo(o);
// }

void _serverAssertWithInfo(const client *c, const robj *o, const char *estr, const char *file, int line) {
    // if (c) _serverAssertPrintClientInfo(c);
    // if (o) _serverAssertPrintObject(o);
    _serverAssert(estr,file,line);
}

