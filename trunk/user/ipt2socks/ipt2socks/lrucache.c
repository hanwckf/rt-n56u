#define _GNU_SOURCE
#include "lrucache.h"
#include <stdlib.h>
#include <string.h>
#undef _GNU_SOURCE

/* lrucache maxsize (private variable) */
static uint16_t lrucache_maxsize = LRUCACHE_MAXSIZE_DEFAULT;

/* get/set the maxsize of lrucache (globalvar) */
uint16_t lrucache_get_maxsize(void) {
    return lrucache_maxsize;
}
void lrucache_set_maxsize(uint16_t maxsize) {
    lrucache_maxsize = maxsize;
}

/* put the given entry into the lrucache, return another removed entry */
cltentry_t* cltcache_put(cltcache_t **cache, cltentry_t *entry) {
    HASH_ADD(hh, *cache, clt_ipport, sizeof(ip_port_t), entry);
    if (HASH_COUNT(*cache) > lrucache_maxsize) {
        cltentry_t *curentry = NULL, *tmpentry = NULL;
        HASH_ITER(hh, *cache, curentry, tmpentry) {
            HASH_DEL(*cache, curentry);
            return curentry;
        }
    }
    return NULL;
}
svrentry_t* svrcache_put(svrcache_t **cache, svrentry_t *entry) {
    HASH_ADD(hh, *cache, svr_ipport, sizeof(ip_port_t), entry);
    if (HASH_COUNT(*cache) > lrucache_maxsize) {
        svrentry_t *curentry = NULL, *tmpentry = NULL;
        HASH_ITER(hh, *cache, curentry, tmpentry) {
            HASH_DEL(*cache, curentry);
            return curentry;
        }
    }
    return NULL;
}

/* get the entry associated with the given key, return NULL if not exists */
cltentry_t* cltcache_get(cltcache_t **cache, ip_port_t *keyptr) {
    cltentry_t *entry = NULL;
    HASH_FIND(hh, *cache, keyptr, sizeof(ip_port_t), entry);
    if (!entry) return NULL;
    HASH_DEL(*cache, entry);
    HASH_ADD(hh, *cache, clt_ipport, sizeof(ip_port_t), entry);
    return entry;
}
svrentry_t* svrcache_get(svrcache_t **cache, ip_port_t *keyptr) {
    svrentry_t *entry = NULL;
    HASH_FIND(hh, *cache, keyptr, sizeof(ip_port_t), entry);
    if (!entry) return NULL;
    HASH_DEL(*cache, entry);
    HASH_ADD(hh, *cache, svr_ipport, sizeof(ip_port_t), entry);
    return entry;
}

/* move the given entry to the end of the lrucache (indicates that it is used) */
void cltcache_use(cltcache_t **cache, cltentry_t *entry) {
    HASH_DEL(*cache, entry);
    HASH_ADD(hh, *cache, clt_ipport, sizeof(ip_port_t), entry);
}
void svrcache_use(svrcache_t **cache, svrentry_t *entry) {
    HASH_DEL(*cache, entry);
    HASH_ADD(hh, *cache, svr_ipport, sizeof(ip_port_t), entry);
}

/* delete the given entry from the lrucache (remove only, do not release memory) */
void cltcache_del(cltcache_t **cache, cltentry_t *entry) {
    HASH_DEL(*cache, entry);
}
void svrcache_del(svrcache_t **cache, svrentry_t *entry) {
    HASH_DEL(*cache, entry);
}
