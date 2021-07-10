#ifndef CHINADNS_NG_CHINADNS_H
#define CHINADNS_NG_CHINADNS_H

#define _GNU_SOURCE
#include <stdbool.h>
#include "netutils.h"
#undef _GNU_SOURCE

/* global variable declaration */
extern bool g_noip_as_chnip; /* used by dnsutils.h */
extern char g_ipset_setname4[IPSET_MAXNAMELEN]; /* used by netutils.h */
extern char g_ipset_setname6[IPSET_MAXNAMELEN]; /* used by netutils.h */

#endif
