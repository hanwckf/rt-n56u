#define _GNU_SOURCE
#include "maputils.h"
#include <stdlib.h>
#include <string.h>
#undef _GNU_SOURCE

/* put key and value to hashmap */
hashentry_t* hashmap_put(hashmap_t **hashmap, uint16_t unique_msgid, uint16_t origin_msgid, int query_timerfd, uint8_t dnlmatch_ret, const inet6_skaddr_t *source_addr) {
    hashentry_t *hashentry = malloc(sizeof(hashentry_t));
    hashentry->unique_msgid = unique_msgid;
    hashentry->origin_msgid = origin_msgid;
    hashentry->query_timerfd = query_timerfd;
    hashentry->trustdns_buf = NULL;
    hashentry->chinadns_got = false;
    hashentry->dnlmatch_ret = dnlmatch_ret;
    memcpy(&hashentry->source_addr, source_addr, sizeof(inet6_skaddr_t));
    MYHASH_ADD(*hashmap, hashentry, &hashentry->unique_msgid, sizeof(hashentry->unique_msgid));
    return hashentry;
}

/* get entry_ptr by unique_msgid */
hashentry_t* hashmap_get(hashmap_t *hashmap, uint16_t unique_msgid) {
    hashentry_t *hashentry = NULL;
    MYHASH_GET(hashmap, hashentry, &unique_msgid, sizeof(unique_msgid));
    return hashentry;
}

/* delete and free the entry from hashmap */
void hashmap_del(hashmap_t **hashmap, hashentry_t *hashentry) {
    MYHASH_DEL(*hashmap, hashentry);
    free(hashentry);
}
