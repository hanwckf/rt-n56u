#ifndef __DBAPI_H_
#define __DBAPI_H_
#include "httpd.h"
#define offsetof2(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (const typeof( ((type *)0)->member )*)(ptr);    \
        (type *)( (char *)__mptr - offsetof2(type,member) );})

#define MAGIC "magicv1 "
#define MAGIC_LEN 8
#define DBHEADER_LEN 8
#define HEADER_PREFIX (MAGIC_LEN + DBHEADER_LEN)
#define SK_PATH_MAX 128
#define BUF_MAX 2048
#define READ_MAX 65536

#define DELAY_PREFIX "__delay__"
#define DELAY_PREFIX_LEN 9
#define DELAY_KEY_LEN 128

#define SKIPD_DEBUG 3

#define _min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _b : _a; })

#define _max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

typedef struct _dbclient {
    int remote_fd;

    char* buf;
    int buf_max;
    int buf_len;
    int buf_pos;
} dbclient;

typedef int (*fn_db_parse)(dbclient* client, webs_t wp, char* prefix, char* key, char* value);

int dbclient_start(dbclient* client);
int dbclient_bulk(dbclient* client, const char* command, const char* key, int nk, const char* value, int nv);
int dbclient_end(dbclient* client);
int dbclient_list(dbclient* client, char* prefix, webs_t wp, fn_db_parse fn);
#endif

