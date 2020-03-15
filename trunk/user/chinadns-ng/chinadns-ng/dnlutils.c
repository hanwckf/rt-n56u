#define _GNU_SOURCE
#include "dnlutils.h"
#include "dnsutils.h"
#include "logutils.h"
#include "uthash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#undef _GNU_SOURCE

/* a very simple memory pool (alloc only) */
static void* mempool_alloc(size_t length) {
    static void  *mempool_buffer = NULL;
    static size_t mempool_length = 0;
    if (mempool_length < length) {
        /* arg.length must be <= 4096 */
        mempool_length = 4096; /* block size */
        mempool_buffer = malloc(mempool_length);
    }
    mempool_buffer += length;
    mempool_length -= length;
    return mempool_buffer - length;
}

/* hash entry typedef */
typedef struct {
    UT_hash_handle hh;
    char dname[];
} dnlentry_t;

/* hash table (head entry) */
static dnlentry_t *g_gfwlist_headentry = NULL;
static dnlentry_t *g_chnlist_headentry = NULL;

/* handling dname matching pattern */
static inline char* dnl_pattern_strip(char *pattern) {
    if (pattern[0] == '.' || pattern[strlen(pattern) - 1] == '.') return NULL;
    for (int i = 0; i < 4; ++i) {
        char *sepptr = strrchr(pattern, '.');
        if (sepptr) {
            *sepptr = '/';
            if (i == 3) pattern = sepptr + 1;
        } else if (i == 0) {
            return NULL;
        } else {
            break;
        }
    }
    return pattern;
}

/* split dname pattern, array length is 2 */
static inline void dnl_pattern_split(const char *pattern, const char* subpattern_array[]) {
    int slashchar_count = 0;
    for (int i = 0; pattern[i]; ++i) {
        if (pattern[i] == '/') ++slashchar_count;
    }
    if (slashchar_count == 2) {
        subpattern_array[0] = strchr(pattern, '/') + 1;
    } else if (slashchar_count == 3) {
        subpattern_array[1] = strchr(pattern, '/') + 1;
        subpattern_array[0] = strchr(subpattern_array[1], '/') + 1;
    }
}

/* convert domain name, array length is 3 */
static inline void dnl_input_convert(char *fulldomain, const char* subdomain_array[]) {
    if (fulldomain[0] == '.') return;
    int replace_count = 0;
    while (replace_count < 4) {
        char *sepptr = strrchr(fulldomain, '.');
        if (!sepptr) break;
        *sepptr = '/';
        ++replace_count;
    }
    switch (replace_count) {
        case 1:
            subdomain_array[0] = fulldomain;
            break;
        case 2:
            subdomain_array[1] = fulldomain;
            subdomain_array[0] = strchr(fulldomain, '/') + 1;
            break;
        case 3:
            subdomain_array[2] = fulldomain;
            subdomain_array[1] = strchr(fulldomain, '/') + 1;
            subdomain_array[0] = strchr(subdomain_array[1], '/') + 1;
            break;
        case 4:
            subdomain_array[2] = strchr(fulldomain, '/') + 1;
            subdomain_array[1] = strchr(subdomain_array[2], '/') + 1;
            subdomain_array[0] = strchr(subdomain_array[1], '/') + 1;
            break;
    }
}

/* initialize domain-name-list from file */
size_t dnl_init(const char *filename, bool is_gfwlist) {
    FILE *fp = NULL;
    if (strcmp(filename, "-") == 0) {
        fp = stdin;
    } else {
        fp = fopen(filename, "r");
        if (!fp) {
            LOGERR("[dnl_init] failed to open '%s': (%d) %s", filename, errno, strerror(errno));
            exit(errno);
        }
    }

    dnlentry_t **headentry = is_gfwlist ? &g_gfwlist_headentry : &g_chnlist_headentry;
    char strbuf[DNS_DOMAIN_NAME_MAXLEN];
    while (fscanf(fp, "%253s", strbuf) > 0) {
        char *dname = dnl_pattern_strip(strbuf);
        if (!dname) continue;

        dnlentry_t *entry = NULL;
        MYHASH_GET(*headentry, entry, dname, strlen(dname));
        if (entry) continue;

        entry = mempool_alloc(sizeof(dnlentry_t) + strlen(dname) + 1);
        strcpy(entry->dname, dname);
        MYHASH_ADD(*headentry, entry, entry->dname, strlen(entry->dname));
    }
    if (fp != stdin) fclose(fp);

    dnlentry_t *curentry = NULL, *tmpentry = NULL;
    MYHASH_FOR(*headentry, curentry, tmpentry) {
        const char* subpattern_array[2] = {0};
        dnl_pattern_split(curentry->dname, subpattern_array);
        for (int i = 0; i < 2 && subpattern_array[i]; ++i) {
            dnlentry_t *findentry = NULL;
            MYHASH_GET(*headentry, findentry, subpattern_array[i], strlen(subpattern_array[i]));
            if (findentry) {
                MYHASH_DEL(*headentry, curentry);
                break;
            }
        }
    }
    return MYHASH_LEN(*headentry);
}

/* check if the given domain name matches */
uint8_t dnl_ismatch(char *domainname, bool is_gfwlist_first) {
    const char* subdomain_array[3] = {0};
    dnl_input_convert(domainname, subdomain_array);

    dnlentry_t *headentry = is_gfwlist_first ? g_gfwlist_headentry : g_chnlist_headentry;
    if (headentry) {
        for (int i = 0; i < 3 && subdomain_array[i]; ++i) {
            dnlentry_t *findentry = NULL;
            MYHASH_GET(headentry, findentry, subdomain_array[i], strlen(subdomain_array[i]));
            if (findentry) return is_gfwlist_first ? DNL_MRESULT_GFWLIST : DNL_MRESULT_CHNLIST;
        }
    }

    headentry = is_gfwlist_first ? g_chnlist_headentry : g_gfwlist_headentry;
    if (headentry) {
        for (int i = 0; i < 3 && subdomain_array[i]; ++i) {
            dnlentry_t *findentry = NULL;
            MYHASH_GET(headentry, findentry, subdomain_array[i], strlen(subdomain_array[i]));
            if (findentry) return is_gfwlist_first ? DNL_MRESULT_CHNLIST : DNL_MRESULT_GFWLIST;
        }
    }

    return DNL_MRESULT_NOMATCH;
}
