#ifndef IPT2SOCKS_LRUCACHE_H
#define IPT2SOCKS_LRUCACHE_H

#define _GNU_SOURCE
#include "uthash.h"
#include "netutils.h"
#undef _GNU_SOURCE

/* default max number of entries */
#define LRUCACHE_MAXSIZE_DEFAULT 256

/* udp client lruentry structure typedef */
typedef struct {
    ip_port_t       clt_ipport;
    uv_tcp_t       *tcp_handle;
    uv_udp_t       *udp_handle;
    uv_timer_t     *free_timer;
    UT_hash_handle  hh;
} cltentry_t, cltcache_t;

/* udp server lruentry structure typedef */
typedef struct {
    ip_port_t       svr_ipport;
    uv_timer_t     *free_timer;
    int             svr_sockfd;
    UT_hash_handle  hh;
} svrentry_t, svrcache_t;

/* get/set the maxsize of lrucache (globalvar) */
uint16_t lrucache_get_maxsize(void);
void lrucache_set_maxsize(uint16_t maxsize);

/* put the given entry into the lrucache, return another removed entry */
cltentry_t* cltcache_put(cltcache_t **cache, cltentry_t *entry);
svrentry_t* svrcache_put(svrcache_t **cache, svrentry_t *entry);

/* get the entry associated with the given key, return NULL if not exists */
cltentry_t* cltcache_get(cltcache_t **cache, ip_port_t *keyptr);
svrentry_t* svrcache_get(svrcache_t **cache, ip_port_t *keyptr);

/* move the given entry to the end of the lrucache (indicates that it is used) */
void cltcache_use(cltcache_t **cache, cltentry_t *entry);
void svrcache_use(svrcache_t **cache, svrentry_t *entry);

/* delete the given entry from the lrucache (remove only, do not release memory) */
void cltcache_del(cltcache_t **cache, cltentry_t *entry);
void svrcache_del(svrcache_t **cache, svrentry_t *entry);

#endif
