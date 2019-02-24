#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "common.h"
#include "object.h"


static long long ustime(void) {
    struct timeval tv;
    long long ust;

    gettimeofday(&tv, NULL);
    ust = ((long long)tv.tv_sec)*1000000;
    ust += tv.tv_usec;
    return ust;
}

static long long mstime(void) {
    return ustime()/1000;
}

/* ----------------------------------------------------------------------------
 * Implementation of eviction, aging and LRU
 * --------------------------------------------------------------------------*/

/* Return the LRU clock, based on the clock resolution. This is a time
 * in a reduced-bits format that can be used to set and check the
 * object->lru field of redisObject structures. */
unsigned int getLRUClock(void) {
    return (mstime()/LRU_CLOCK_RESOLUTION) & LRU_CLOCK_MAX;
}

/* This function is used to obtain the current LRU clock.
 * If the current resolution is lower than the frequency we refresh the
 * LRU clock (as it should be in production servers) we return the
 * precomputed value, otherwise we need to resort to a system call. */
unsigned int LRU_CLOCK(void) {
    unsigned int lruclock;

    // if (1000/server.hz <= LRU_CLOCK_RESOLUTION) {
    //     atomicGet(server.lruclock,lruclock);
    // } else {
    //     lruclock = getLRUClock();
    // }
    return lruclock;
}

void incrRefCount(robj *o) {
    if (o->refcount != OBJ_SHARED_REFCOUNT) o->refcount++;
}

/* Low level key lookup API, not actually called directly from commands
 * implementations that should instead rely on lookupKeyRead(),
 * lookupKeyWrite() and lookupKeyReadWithFlags(). */
robj *lookupKey(redisDb *db, robj *key, int flags) {
    dictEntry *de = dictFind(db->dict,key->ptr);
    if (de) {
        robj *val = dictGetVal(de);

        return val;
    } else {
        return NULL;
    }
}

/* Lookup a key for write operations, and as a side effect, if needed, expires
 * the key if its TTL is reached.
 *
 * Returns the linked value object if the key exists or NULL if the key
 * does not exist in the specified DB. */
robj *lookupKeyWrite(redisDb *db, robj *key) {
    return lookupKey(db,key,LOOKUP_NONE);
}

/* Like lookupKeyReadWithFlags(), but does not use any flag, which is the
 * common case. */
// robj *lookupKeyRead(redisDb *db, robj *key) {
//     return lookupKeyReadWithFlags(db,key,LOOKUP_NONE);
// }


// robj *lookupKeyReadOrReply(client *c, robj *key, robj *reply) {
//     robj *o = lookupKeyRead(c->db, key);
//     if (!o) addReply(c,reply);
//     return o;
// }

// robj *lookupKeyWriteOrReply(client *c, robj *key, robj *reply) {
//     robj *o = lookupKeyWrite(c->db, key);
//     if (!o) addReply(c,reply);
//     return o;
// }

robj *createObject(int type, void *ptr) {
    robj *o = zmalloc(sizeof(*o));
    o->type = type;
    o->encoding = OBJ_ENCODING_RAW;
    o->ptr = ptr;
    o->refcount = 1;

    /* Set the LRU to the current lruclock (minutes resolution), or
     * alternatively the LFU counter. */
    o->lru = LRU_CLOCK();

    return o;
}




