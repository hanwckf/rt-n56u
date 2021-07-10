#ifndef CHINADNS_NG_DNLUTILS_H
#define CHINADNS_NG_DNLUTILS_H

#define _GNU_SOURCE
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#undef _GNU_SOURCE

/* dnl_ismatch() return value */
#define DNL_MRESULT_NOMATCH 0 // did not match
#define DNL_MRESULT_GFWLIST 1 // hit the gfwlist
#define DNL_MRESULT_CHNLIST 2 // hit the chnlist

/* initialize domain-name-list from file */
size_t dnl_init(const char *filename, bool is_gfwlist);

/* check if the given domain name matches */
uint8_t dnl_ismatch(char *domainname, bool is_gfwlist_first);

#endif
