/*
    mtr  --  a network diagnostic tool
    Copyright (C) 1997,1998  Matt Kimball

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "config.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#ifdef HAVE_ERROR_H
#include <error.h>
#else
#include "portability/error.h"
#endif
#include <errno.h>

#ifdef __APPLE__
#define BIND_8_COMPAT
#endif
#include <arpa/nameser.h>
#ifdef HAVE_ARPA_NAMESER_COMPAT_H
#include <arpa/nameser_compat.h>
#endif
#include <netdb.h>
#include <netinet/in.h>
#include <resolv.h>
#include <string.h>
#include <sys/socket.h>
#include <search.h>

#include "mtr.h"
#include "asn.h"
#include "utils.h"

/* #define IIDEBUG */
#ifdef IIDEBUG
#include <syslog.h>
#define DEB_syslog syslog
#else
#define DEB_syslog(...) do {} while (0)
#endif

#define IIHASH_HI	128
#define ITEMSMAX	15
#define ITEMSEP	'|'
#define NAMELEN	127
#define UNKN	"???"

static int iihash = 0;
static char fmtinfo[32];

/* items width: ASN, Route, Country, Registry, Allocated */
static const int iiwidth[] = { 7, 19, 4, 8, 11 };       /* item len + space */

typedef char *items_t[ITEMSMAX + 1];
static items_t items_a;         /* without hash: items */
static char txtrec[NAMELEN + 1];        /* without hash: txtrec */
static items_t *items = &items_a;


static char *ipinfo_lookup(
    const char *domain)
{
    unsigned char answer[PACKETSZ], *pt;
    char host[128];
    char *txt;
    int len, exp, size, txtlen, type;


    if (res_init() < 0) {
        error(0, 0, "@res_init failed");
        return NULL;
    }

    memset(answer, 0, PACKETSZ);
    if ((len = res_query(domain, C_IN, T_TXT, answer, PACKETSZ)) < 0) {
        if (iihash)
            DEB_syslog(LOG_INFO, "Malloc-txt: %s", UNKN);
        return xstrdup(UNKN);
    }

    pt = answer + sizeof(HEADER);

    if ((exp =
         dn_expand(answer, answer + len, pt, host, sizeof(host))) < 0) {
        printf("@dn_expand failed\n");
        return NULL;
    }

    pt += exp;

    GETSHORT(type, pt);
    if (type != T_TXT) {
        printf("@Broken DNS reply.\n");
        return NULL;
    }

    pt += INT16SZ;              /* class */

    if ((exp =
         dn_expand(answer, answer + len, pt, host, sizeof(host))) < 0) {
        printf("@second dn_expand failed\n");
        return NULL;
    }

    pt += exp;
    GETSHORT(type, pt);
    if (type != T_TXT) {
        printf("@Not a TXT record\n");
        return NULL;
    }

    pt += INT16SZ;              /* class */
    pt += INT32SZ;              /* ttl */
    GETSHORT(size, pt);
    txtlen = *pt;


    if (txtlen >= size || !txtlen) {
        printf("@Broken TXT record (txtlen = %d, size = %d)\n", txtlen,
               size);
        return NULL;
    }

    if (txtlen > NAMELEN)
        txtlen = NAMELEN;

    if (iihash) {
        txt = xmalloc(txtlen + 1);
    } else
        txt = (char *) txtrec;

    pt++;
    xstrncpy(txt, (char *) pt, txtlen + 1);

    if (iihash)
        DEB_syslog(LOG_INFO, "Malloc-txt(%p): %s", txt, txt);

    return txt;
}

/* originX.asn.cymru.com txtrec:    ASN | Route | Country | Registry | Allocated */
static char *split_txtrec(
    struct mtr_ctl *ctl,
    char *txt_rec)
{
    char *prev;
    char *next;
    int i = 0, j;

    if (!txt_rec)
        return NULL;
    if (iihash) {
        DEB_syslog(LOG_INFO, "Malloc-tbl: %s", txt_rec);
        if (!(items = malloc(sizeof(*items)))) {
            DEB_syslog(LOG_INFO, "Free-txt(%p)", txt_rec);
            free(txt_rec);
            return NULL;
        }
    }

    prev = txt_rec;

    while ((next = strchr(prev, ITEMSEP)) && (i < ITEMSMAX)) {
        *next = '\0';
        next++;
        (*items)[i] = trim(prev, ITEMSEP);
        prev = next;
        i++;
    }
    (*items)[i] = trim(prev, ITEMSEP);

    if (i < ITEMSMAX)
        i++;
    for (j = i; j <= ITEMSMAX; j++)
        (*items)[j] = NULL;

    if (i > ctl->ipinfo_max)
        ctl->ipinfo_max = i;
    if (ctl->ipinfo_no >= i) {
        if (ctl->ipinfo_no >= ctl->ipinfo_max)
            ctl->ipinfo_no = 0;
        return (*items)[0];
    } else
        return (*items)[ctl->ipinfo_no];
}

#ifdef ENABLE_IPV6
/* from dns.c:addr2ip6arpa() */
static void reverse_host6(
    struct in6_addr *addr,
    char *buff,
    int buff_length)
{
    int i;
    char *b = buff;
    for (i = (sizeof(*addr) / 2 - 1); i >= 0; i--, b += 4)      /* 64b portion */
        snprintf(b, buff_length,
                 "%x.%x.", addr->s6_addr[i] & 0xf, addr->s6_addr[i] >> 4);

    buff[strlen(buff) - 1] = '\0';
}
#endif

static char *get_ipinfo(
    struct mtr_ctl *ctl,
    ip_t * addr)
{
    char key[NAMELEN];
    char lookup_key[NAMELEN];
    char *val = NULL;
    ENTRY item;

    if (!addr)
        return NULL;

    if (ctl->af == AF_INET6) {
#ifdef ENABLE_IPV6
        reverse_host6(addr, key, NAMELEN);
        if (snprintf(lookup_key, NAMELEN, "%s.origin6.asn.cymru.com", key)
            >= NAMELEN)
            return NULL;
#else
        return NULL;
#endif
    } else {
        unsigned char buff[4];
        memcpy(buff, addr, 4);
        if (snprintf
            (key, NAMELEN, "%d.%d.%d.%d", buff[3], buff[2], buff[1],
             buff[0]) >= NAMELEN)
            return NULL;
        if (snprintf(lookup_key, NAMELEN, "%s.origin.asn.cymru.com", key)
            >= NAMELEN)
            return NULL;
    }

    if (iihash) {
        ENTRY *found_item;

        DEB_syslog(LOG_INFO, ">> Search: %s", key);
        item.key = key;;
        if ((found_item = hsearch(item, FIND))) {
            if (!(val = (*((items_t *) found_item->data))[ctl->ipinfo_no]))
                val = (*((items_t *) found_item->data))[0];
            DEB_syslog(LOG_INFO, "Found (hashed): %s", val);
        }
    }

    if (!val) {
        DEB_syslog(LOG_INFO, "Lookup: %s", key);
        if ((val = split_txtrec(ctl, ipinfo_lookup(lookup_key)))) {
            DEB_syslog(LOG_INFO, "Looked up: %s", key);
            if (iihash)
                if ((item.key = xstrdup(key))) {
                    item.data = (void *) items;
                    hsearch(item, ENTER);
                    DEB_syslog(LOG_INFO, "Insert into hash: %s", key);
                }
        }
    }

    return val;
}

ATTRIBUTE_CONST size_t get_iiwidth_len(
    void)
{
    return (sizeof(iiwidth) / sizeof((iiwidth)[0]));
}

ATTRIBUTE_CONST int get_iiwidth(
    int ipinfo_no)
{
    static const int len = (sizeof(iiwidth) / sizeof((iiwidth)[0]));

    if (ipinfo_no < len)
        return iiwidth[ipinfo_no];
    return iiwidth[ipinfo_no % len];
}

char *fmt_ipinfo(
    struct mtr_ctl *ctl,
    ip_t * addr)
{
    char *ipinfo = get_ipinfo(ctl, addr);
    char fmt[8];
    snprintf(fmt, sizeof(fmt), "%s%%-%ds", ctl->ipinfo_no ? "" : "AS",
             get_iiwidth(ctl->ipinfo_no));
    snprintf(fmtinfo, sizeof(fmtinfo), fmt, ipinfo ? ipinfo : UNKN);
    return fmtinfo;
}

int is_printii(
    struct mtr_ctl *ctl)
{
    return ((ctl->ipinfo_no >= 0) && (ctl->ipinfo_no != ctl->ipinfo_max));
}

void asn_open(
    struct mtr_ctl *ctl)
{
    if (ctl->ipinfo_no >= 0) {
        DEB_syslog(LOG_INFO, "hcreate(%d)", IIHASH_HI);
        if (!(iihash = hcreate(IIHASH_HI)))
            error(0, errno, "ipinfo hash");
    }
}

void asn_close(
    struct mtr_ctl *ctl)
{
    if ((ctl->ipinfo_no >= 0) && iihash) {
        DEB_syslog(LOG_INFO, "hdestroy()");
        hdestroy();
        iihash = 0;
    }
}
