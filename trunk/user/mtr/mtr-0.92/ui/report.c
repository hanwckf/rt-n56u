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

#include <sys/types.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include "mtr.h"
#include "report.h"
#include "net.h"
#include "dns.h"
#include "asn.h"
#include "utils.h"

#define MAXLOADBAL 5
#define MAX_FORMAT_STR 81


void report_open(
    void)
{
    const time_t now = time(NULL);
    const char *t = iso_time(&now);

    printf("Start: %s\n", t);
}

static size_t snprint_addr(
    struct mtr_ctl *ctl,
    char *dst,
    size_t dst_len,
    ip_t * addr)
{
    if (addrcmp((void *) addr, (void *) &ctl->unspec_addr, ctl->af)) {
        struct hostent *host =
            ctl->dns ? addr2host((void *) addr, ctl->af) : NULL;
        if (!host)
            return snprintf(dst, dst_len, "%s", strlongip(ctl, addr));
        else if (ctl->dns && ctl->show_ips)
            return snprintf(dst, dst_len, "%s (%s)", host->h_name,
                            strlongip(ctl, addr));
        else
            return snprintf(dst, dst_len, "%s", host->h_name);
    } else
        return snprintf(dst, dst_len, "%s", "???");
}


#ifdef HAVE_IPINFO
static void print_mpls(
    struct mplslen *mpls)
{
    int k;
    for (k = 0; k < mpls->labels; k++)
        printf("       [MPLS: Lbl %lu Exp %u S %cu TTL %u]\n",
               mpls->label[k], mpls->exp[k], mpls->s[k], mpls->ttl[k]);
}
#endif

void report_close(
    struct mtr_ctl *ctl)
{
    int i, j, at, max, z, w;
    struct mplslen *mpls, *mplss;
    ip_t *addr;
    ip_t *addr2 = NULL;
    char name[MAX_FORMAT_STR];
    char buf[1024];
    char fmt[16];
    size_t len = 0;
    size_t len_hosts = 33;
#ifdef HAVE_IPINFO
    int len_tmp;
    const size_t iiwidth_len = get_iiwidth_len();
#endif

    if (ctl->reportwide) {
        /* get the longest hostname */
        len_hosts = strlen(ctl->LocalHostname);
        max = net_max(ctl);
        at = net_min(ctl);
        for (; at < max; at++) {
            size_t nlen;
            addr = net_addr(at);
            if ((nlen = snprint_addr(ctl, name, sizeof(name), addr)))
                if (len_hosts < nlen)
                    len_hosts = nlen;
        }
    }
#ifdef HAVE_IPINFO
    len_tmp = len_hosts;
    if (ctl->ipinfo_no >= 0 && iiwidth_len) {
        ctl->ipinfo_no %= iiwidth_len;
        if (ctl->reportwide) {
            len_hosts++;        /* space */
            len_tmp += get_iiwidth(ctl->ipinfo_no);
            if (!ctl->ipinfo_no)
                len_tmp += 2;   /* align header: AS */
        }
    }
    snprintf(fmt, sizeof(fmt), "HOST: %%-%ds", len_tmp);
#else
    snprintf(fmt, sizeof(fmt), "HOST: %%-%zus", len_hosts);
#endif
    snprintf(buf, sizeof(buf), fmt, ctl->LocalHostname);
    len = ctl->reportwide ? strlen(buf) : len_hosts;
    for (i = 0; i < MAXFLD; i++) {
        j = ctl->fld_index[ctl->fld_active[i]];
        if (j < 0)
            continue;

        snprintf(fmt, sizeof(fmt), "%%%ds", data_fields[j].length);
        snprintf(buf + len, sizeof(buf), fmt, data_fields[j].title);
        len += data_fields[j].length;
    }
    printf("%s\n", buf);

    max = net_max(ctl);
    at = net_min(ctl);
    for (; at < max; at++) {
        addr = net_addr(at);
        mpls = net_mpls(at);
        snprint_addr(ctl, name, sizeof(name), addr);

#ifdef HAVE_IPINFO
        if (is_printii(ctl)) {
            snprintf(fmt, sizeof(fmt), " %%2d. %%s%%-%zus", len_hosts);
            snprintf(buf, sizeof(buf), fmt, at + 1, fmt_ipinfo(ctl, addr),
                     name);
        } else {
#endif
            snprintf(fmt, sizeof(fmt), " %%2d.|-- %%-%zus", len_hosts);
            snprintf(buf, sizeof(buf), fmt, at + 1, name);
#ifdef HAVE_IPINFO
        }
#endif
        len = ctl->reportwide ? strlen(buf) : len_hosts;
        for (i = 0; i < MAXFLD; i++) {
            j = ctl->fld_index[ctl->fld_active[i]];
            if (j < 0)
                continue;

            /* 1000.0 is a temporay hack for stats usec to ms, impacted net_loss. */
            if (strchr(data_fields[j].format, 'f')) {
                snprintf(buf + len, sizeof(buf), data_fields[j].format,
                         data_fields[j].net_xxx(at) / 1000.0);
            } else {
                snprintf(buf + len, sizeof(buf), data_fields[j].format,
                         data_fields[j].net_xxx(at));
            }
            len += data_fields[j].length;
        }
        printf("%s\n", buf);

        /* This feature shows 'loadbalances' on routes */

        /* z is starting at 1 because addrs[0] is the same that addr */
        for (z = 1; z < MAXPATH; z++) {
            int found = 0;
            addr2 = net_addrs(at, z);
            mplss = net_mplss(at, z);
            if ((addrcmp
                 ((void *) &ctl->unspec_addr, (void *) addr2,
                  ctl->af)) == 0)
                break;
            for (w = 0; w < z; w++)
                /* Ok... checking if there are ips repeated on same hop */
                if ((addrcmp
                     ((void *) addr2, (void *) net_addrs(at, w),
                      ctl->af)) == 0) {
                    found = 1;
                    break;
                }

            if (!found) {

#ifdef HAVE_IPINFO
                if (is_printii(ctl)) {
                    if (mpls->labels && z == 1 && ctl->enablempls)
                        print_mpls(mpls);
                    snprint_addr(ctl, name, sizeof(name), addr2);
                    printf("     %s%s\n", fmt_ipinfo(ctl, addr2), name);
                    if (ctl->enablempls)
                        print_mpls(mplss);
                }
#else
                int k;
                if (mpls->labels && z == 1 && ctl->enablempls) {
                    for (k = 0; k < mpls->labels; k++) {
                        printf
                            ("    |  |+-- [MPLS: Lbl %lu Exp %u S %u TTL %u]\n",
                             mpls->label[k], mpls->exp[k], mpls->s[k],
                             mpls->ttl[k]);
                    }
                }

                if (z == 1) {
                    printf("    |  `|-- %s\n", strlongip(ctl, addr2));
                    for (k = 0; k < mplss->labels && ctl->enablempls; k++) {
                        printf
                            ("    |   +-- [MPLS: Lbl %lu Exp %u S %u TTL %u]\n",
                             mplss->label[k], mplss->exp[k], mplss->s[k],
                             mplss->ttl[k]);
                    }
                } else {
                    printf("    |   |-- %s\n", strlongip(ctl, addr2));
                    for (k = 0; k < mplss->labels && ctl->enablempls; k++) {
                        printf
                            ("    |   +-- [MPLS: Lbl %lu Exp %u S %u TTL %u]\n",
                             mplss->label[k], mplss->exp[k], mplss->s[k],
                             mplss->ttl[k]);
                    }
                }
#endif
            }
        }

        /* No multipath */
#ifdef HAVE_IPINFO
        if (is_printii(ctl)) {
            if (mpls->labels && z == 1 && ctl->enablempls)
                print_mpls(mpls);
        }
#else
        if (mpls->labels && z == 1 && ctl->enablempls) {
            int k;
            for (k = 0; k < mpls->labels; k++) {
                printf("    |   +-- [MPLS: Lbl %lu Exp %u S %u TTL %u]\n",
                       mpls->label[k], mpls->exp[k], mpls->s[k],
                       mpls->ttl[k]);
            }
        }
#endif
    }
}


void txt_open(
    void)
{
}


void txt_close(
    struct mtr_ctl *ctl)
{
    report_close(ctl);
}


void json_open(
    void)
{
}


void json_close(
    struct mtr_ctl *ctl)
{
    int i, j, at, first, max;
    ip_t *addr;
    char name[MAX_FORMAT_STR];

    printf("{\n");
    printf("  \"report\": {\n");
    printf("    \"mtr\": {\n");
    printf("      \"src\": \"%s\",\n", ctl->LocalHostname);
    printf("      \"dst\": \"%s\",\n", ctl->Hostname);
    printf("      \"tos\": \"0x%X\",\n", ctl->tos);
    if (ctl->cpacketsize >= 0) {
        printf("      \"psize\": \"%d\",\n", ctl->cpacketsize);
    } else {
        printf("      \"psize\": \"rand(%d-%d)\",\n", MINPACKET,
               -ctl->cpacketsize);
    }
    if (ctl->bitpattern >= 0) {
        printf("      \"bitpattern\": \"0x%02X\",\n",
               (unsigned char) (ctl->bitpattern));
    } else {
        printf("      \"bitpattern\": \"rand(0x00-FF)\",\n");
    }
    printf("      \"tests\": \"%d\"\n", ctl->MaxPing);
    printf("    },\n");

    printf("    \"hubs\": [");

    max = net_max(ctl);
    at = first = net_min(ctl);
    for (; at < max; at++) {
        addr = net_addr(at);
        snprint_addr(ctl, name, sizeof(name), addr);

        if (at == first) {
            printf("{\n");
        } else {
            printf("    {\n");
        }
        printf("      \"count\": \"%d\",\n", at + 1);
        printf("      \"host\": \"%s\",\n", name);
#ifdef HAVE_IPINFO
        if(!ctl->ipinfo_no) {
          char* fmtinfo = fmt_ipinfo(ctl, addr);
          if (fmtinfo != NULL) fmtinfo = trim(fmtinfo, '\0');
          printf("      \"ASN\": \"%s\",\n", fmtinfo);
        }
#endif
        for (i = 0; i < MAXFLD; i++) {
            const char *format;

            j = ctl->fld_index[ctl->fld_active[i]];

            /* Commas */
            if (i + 1 == MAXFLD) {
                printf("\n");
            } else if (j > 0 && i != 0) {
                printf(",\n");
            }

            if (j <= 0)
                continue;       /* Field nr 0, " " shouldn't be printed in this method. */

            /* Format value */
            format = data_fields[j].format;
            if (strchr(format, 'f')) {
                format = "%.2f";
            } else {
                format = "%d";
            }

            /* Format json line */
            snprintf(name, sizeof(name), "%s%s", "      \"%s\": ", format);

            /* Output json line */
            if (strchr(data_fields[j].format, 'f')) {
                /* 1000.0 is a temporay hack for stats usec to ms, impacted net_loss. */
                printf(name,
                       data_fields[j].title,
                       data_fields[j].net_xxx(at) / 1000.0);
            } else {
                printf(name,
                       data_fields[j].title, data_fields[j].net_xxx(at));
            }
        }
        if (at + 1 == max) {
            printf("    }]\n");
        } else {
            printf("    },\n");
        }
    }
    printf("  }\n");
    printf("}\n");
}



void xml_open(
    void)
{
}


void xml_close(
    struct mtr_ctl *ctl)
{
    int i, j, at, max;
    ip_t *addr;
    char name[MAX_FORMAT_STR];

    printf("<?xml version=\"1.0\"?>\n");
    printf("<MTR SRC=\"%s\" DST=\"%s\"", ctl->LocalHostname,
           ctl->Hostname);
    printf(" TOS=\"0x%X\"", ctl->tos);
    if (ctl->cpacketsize >= 0) {
        printf(" PSIZE=\"%d\"", ctl->cpacketsize);
    } else {
        printf(" PSIZE=\"rand(%d-%d)\"", MINPACKET, -ctl->cpacketsize);
    }
    if (ctl->bitpattern >= 0) {
        printf(" BITPATTERN=\"0x%02X\"",
               (unsigned char) (ctl->bitpattern));
    } else {
        printf(" BITPATTERN=\"rand(0x00-FF)\"");
    }
    printf(" TESTS=\"%d\">\n", ctl->MaxPing);

    max = net_max(ctl);
    at = net_min(ctl);
    for (; at < max; at++) {
        addr = net_addr(at);
        snprint_addr(ctl, name, sizeof(name), addr);

        printf("    <HUB COUNT=\"%d\" HOST=\"%s\">\n", at + 1, name);
        for (i = 0; i < MAXFLD; i++) {
            const char *title;

            j = ctl->fld_index[ctl->fld_active[i]];
            if (j <= 0)
                continue;       /* Field nr 0, " " shouldn't be printed in this method. */

            snprintf(name, sizeof(name), "%s%s%s", "        <%s>",
                     data_fields[j].format, "</%s>\n");

            /* XML doesn't allow "%" in tag names, rename Loss% to just Loss */
            title = data_fields[j].title;
            if (strcmp(data_fields[j].title, "Loss%") == 0) {
                title = "Loss";
            }

            /* 1000.0 is a temporay hack for stats usec to ms, impacted net_loss. */
            if (strchr(data_fields[j].format, 'f')) {
                printf(name,
                       title, data_fields[j].net_xxx(at) / 1000.0, title);
            } else {
                printf(name, title, data_fields[j].net_xxx(at), title);
            }
        }
        printf("    </HUB>\n");
    }
    printf("</MTR>\n");
}


void csv_open(
    void)
{
}

void csv_close(
    struct mtr_ctl *ctl,
    time_t now)
{
    int i, j, at, max;
    ip_t *addr;
    char name[MAX_FORMAT_STR];

    for (i = 0; i < MAXFLD; i++) {
        j = ctl->fld_index[ctl->fld_active[i]];
        if (j < 0)
            continue;
    }

    max = net_max(ctl);
    at = net_min(ctl);
    for (; at < max; at++) {
        addr = net_addr(at);
        snprint_addr(ctl, name, sizeof(name), addr);

        if (at == net_min(ctl)) {
            printf("Mtr_Version,Start_Time,Status,Host,Hop,Ip,");
#ifdef HAVE_IPINFO
            if (!ctl->ipinfo_no) {
                printf("Asn,");
            }
#endif
            for (i = 0; i < MAXFLD; i++) {
                j = ctl->fld_index[ctl->fld_active[i]];
                if (j < 0)
                    continue;
                printf("%s,", data_fields[j].title);
            }
            printf("\n");
        }
#ifdef HAVE_IPINFO
        if (!ctl->ipinfo_no) {
            char *fmtinfo = fmt_ipinfo(ctl, addr);
            fmtinfo = trim(fmtinfo, '\0');
            printf("MTR.%s,%lld,%s,%s,%d,%s,%s", PACKAGE_VERSION,
                   (long long) now, "OK", ctl->Hostname, at + 1, name,
                   fmtinfo);
        } else
#endif
            printf("MTR.%s,%lld,%s,%s,%d,%s", PACKAGE_VERSION,
                   (long long) now, "OK", ctl->Hostname, at + 1, name);

        for (i = 0; i < MAXFLD; i++) {
            j = ctl->fld_index[ctl->fld_active[i]];
            if (j < 0)
                continue;

            /* 1000.0 is a temporay hack for stats usec to ms, impacted net_loss. */
            if (strchr(data_fields[j].format, 'f')) {
                printf(",%.2f",
                       (double) (data_fields[j].net_xxx(at) / 1000.0));
            } else {
                printf(",%d", data_fields[j].net_xxx(at));
            }
        }
        printf("\n");
    }
}
