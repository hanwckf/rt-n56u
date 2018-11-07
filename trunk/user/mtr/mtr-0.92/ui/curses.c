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

#include "mtr.h"

#include <strings.h>
#include <unistd.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* MacOSX may need this before socket.h...*/
#if defined(HAVE_SYS_TYPES_H)
#include <sys/types.h>
#endif

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#if defined(HAVE_NCURSES_H)
#include <ncurses.h>
#elif defined(HAVE_NCURSES_CURSES_H)
#include <ncurses/curses.h>
#elif defined(HAVE_CURSES_H)
#include <curses.h>
#elif defined(HAVE_CURSESX_H)
#include <cursesX.h>
#else
#error No curses header file available
#endif

/* This go-around is needed only when compiling with antique version of curses.
   getmaxyx is part of Technical Standard X/Open Curses Issue 4, Version 2 (1996).
   http://pubs.opengroup.org/onlinepubs/9693989999/toc.pdf see page 106 */
#ifndef getmaxyx
#define getmaxyx(win,y,x)	((y) = (win)->_maxy + 1, (x) = (win)->_maxx + 1)
#endif

#include "mtr.h"
#include "mtr-curses.h"
#include "net.h"
#include "dns.h"
#include "asn.h"
#include "display.h"
#include "utils.h"


enum { NUM_FACTORS = 8 };
static double factors[NUM_FACTORS];
static int scale[NUM_FACTORS];
static char block_map[NUM_FACTORS];

enum { black = 1, red, green, yellow, blue, magenta, cyan, white };
static const int block_col[NUM_FACTORS + 1] = {
    COLOR_PAIR(red) | A_BOLD,
    A_NORMAL,
    COLOR_PAIR(green),
    COLOR_PAIR(green) | A_BOLD,
    COLOR_PAIR(yellow) | A_BOLD,
    COLOR_PAIR(magenta) | A_BOLD,
    COLOR_PAIR(magenta),
    COLOR_PAIR(red),
    COLOR_PAIR(red) | A_BOLD
};

static void pwcenter(
    char *str)
{
    int maxx;
    size_t cx;
    int __unused_int ATTRIBUTE_UNUSED;

    getmaxyx(stdscr, __unused_int, maxx);
    cx = (size_t) (maxx - strlen(str)) / 2;
    printw("%*s%s", (int) cx, "", str);
}


static char *format_number(
    int n,
    int w,
    char *buf)
{
    if (w != 5)
        /* XXX todo: implement w != 5.. */
        snprintf(buf, w + 1, "%s", "unimpl");
    else if (n < 100000)
        /* buf is good as-is */ ;
    else if (n < 1000000)
        snprintf(buf, w + 1, "%3dk%1d", n / 1000, (n % 1000) / 100);
    else if (n < 10000000)
        snprintf(buf, w + 1, "%1dM%03d", n / 1000000,
                 (n % 1000000) / 1000);
    else if (n < 100000000)
        snprintf(buf, w + 1, "%2dM%02d", n / 1000000,
                 (n % 1000000) / 10000);
    else if (n < 1000000000)
        snprintf(buf, w + 1, "%3dM%01d", n / 1000000,
                 (n % 1000000) / 100000);
    else                        /* if (n < 10000000000) */
        snprintf(buf, w + 1, "%1dG%03d", n / 1000000000,
                 (n % 1000000000) / 1000000);

    return buf;
}


int mtr_curses_keyaction(
    struct mtr_ctl *ctl)
{
    int c = getch();
    int i = 0;
    float f = 0.0;
    char buf[MAXFLD + 1];

    if (c == 'Q') {             /* must be checked before c = tolower(c) */
        mvprintw(2, 0, "Type of Service(tos): %d\n", ctl->tos);
        mvprintw(3, 0,
                 "default 0x00, min cost 0x02, rel 0x04,, thr 0x08, low del 0x10...\n");
        move(2, 22);
        refresh();
        while ((c = getch()) != '\n' && i < MAXFLD) {
            attron(A_BOLD);
            printw("%c", c);
            attroff(A_BOLD);
            refresh();
            buf[i++] = c;       /* need more checking on 'c' */
        }
        buf[i] = '\0';
        ctl->tos = atoi(buf);
        if (ctl->tos > 255 || ctl->tos < 0)
            ctl->tos = 0;
        return ActionNone;
    }

    c = tolower(c);

    switch (c) {
    case 'q':
    case 3:
        return ActionQuit;
    case 12:
        return ActionClear;
    case 19:
    case 'p':
        return ActionPause;
    case 17:
    case ' ':
        return ActionResume;
    case 'r':
        return ActionReset;
    case 'd':
        return ActionDisplay;
    case 'e':
        return ActionMPLS;
    case 'n':
        return ActionDNS;
#ifdef HAVE_IPINFO
    case 'y':
        return ActionII;
    case 'z':
        return ActionAS;
#endif
    case '+':
        return ActionScrollDown;
    case '-':
        return ActionScrollUp;
    case 's':
        mvprintw(2, 0, "Change Packet Size: %d\n", ctl->cpacketsize);
        mvprintw(3, 0, "Size Range: %d-%d, < 0:random.\n", MINPACKET,
                 MAXPACKET);
        move(2, 20);
        refresh();
        while ((c = getch()) != '\n' && i < MAXFLD) {
            attron(A_BOLD);
            printw("%c", c);
            attroff(A_BOLD);
            refresh();
            buf[i++] = c;       /* need more checking on 'c' */
        }
        buf[i] = '\0';
        ctl->cpacketsize = atoi(buf);
        return ActionNone;
    case 'b':
        mvprintw(2, 0, "Ping Bit Pattern: %d\n", ctl->bitpattern);
        mvprintw(3, 0, "Pattern Range: 0(0x00)-255(0xff), <0 random.\n");
        move(2, 18);
        refresh();
        while ((c = getch()) != '\n' && i < MAXFLD) {
            attron(A_BOLD);
            printw("%c", c);
            attroff(A_BOLD);
            refresh();
            buf[i++] = c;       /* need more checking on 'c' */
        }
        buf[i] = '\0';
        ctl->bitpattern = atoi(buf);
        if (ctl->bitpattern > 255)
            ctl->bitpattern = -1;
        return ActionNone;
    case 'i':
        mvprintw(2, 0, "Interval : %0.0f\n\n", ctl->WaitTime);
        move(2, 11);
        refresh();
        while ((c = getch()) != '\n' && i < MAXFLD) {
            attron(A_BOLD);
            printw("%c", c);
            attroff(A_BOLD);
            refresh();
            buf[i++] = c;       /* need more checking on 'c' */
        }
        buf[i] = '\0';

        f = atof(buf);

        if (f <= 0.0)
            return ActionNone;
        if (getuid() != 0 && f < 1.0)
            return ActionNone;
        ctl->WaitTime = f;

        return ActionNone;
    case 'f':
        mvprintw(2, 0, "First TTL: %d\n\n", ctl->fstTTL);
        move(2, 11);
        refresh();
        while ((c = getch()) != '\n' && i < MAXFLD) {
            attron(A_BOLD);
            printw("%c", c);
            attroff(A_BOLD);
            refresh();
            buf[i++] = c;       /* need more checking on 'c' */
        }
        buf[i] = '\0';
        i = atoi(buf);

        if (i < 1 || i > ctl->maxTTL)
            return ActionNone;
        ctl->fstTTL = i;

        return ActionNone;
    case 'm':
        mvprintw(2, 0, "Max TTL: %d\n\n", ctl->maxTTL);
        move(2, 9);
        refresh();
        while ((c = getch()) != '\n' && i < MAXFLD) {
            attron(A_BOLD);
            printw("%c", c);
            attroff(A_BOLD);
            refresh();
            buf[i++] = c;       /* need more checking on 'c' */
        }
        buf[i] = '\0';
        i = atoi(buf);

        if (i < ctl->fstTTL || i > (MaxHost - 1))
            return ActionNone;
        ctl->maxTTL = i;

        return ActionNone;
        /* fields to display & their ordering */
    case 'o':
        mvprintw(2, 0, "Fields: %s\n\n", ctl->fld_active);

        for (i = 0; i < MAXFLD; i++) {
            if (data_fields[i].descr != NULL)
                printw("  %s\n", data_fields[i].descr);
        }
        printw("\n");
        move(2, 8);             /* length of "Fields: " */
        refresh();

        i = 0;
        while ((c = getch()) != '\n' && i < MAXFLD) {
            if (strchr(ctl->available_options, c)) {
                attron(A_BOLD);
                printw("%c", c);
                attroff(A_BOLD);
                refresh();
                buf[i++] = c;   /* Only permit values in "available_options" be entered */
            } else {
                printf("\a");   /* Illegal character. Beep, ring the bell. */
            }
        }
        buf[i] = '\0';
        if (strlen(buf) > 0)
            xstrncpy(ctl->fld_active, buf, 2 * MAXFLD);

        return ActionNone;
    case 'j':
        if (strchr(ctl->fld_active, 'N'))
            /* GeoMean and jitter */
            xstrncpy(ctl->fld_active, "DR AGJMXI", 2 * MAXFLD);
        else
            /* default */
            xstrncpy(ctl->fld_active, "LS NABWV", 2 * MAXFLD);
        return ActionNone;
    case 'u':
        switch (ctl->mtrtype) {
        case IPPROTO_ICMP:
        case IPPROTO_TCP:
            ctl->mtrtype = IPPROTO_UDP;
            break;
        case IPPROTO_UDP:
            ctl->mtrtype = IPPROTO_ICMP;
            break;
        }
        return ActionNone;
    case 't':
        switch (ctl->mtrtype) {
        case IPPROTO_ICMP:
        case IPPROTO_UDP:
            ctl->mtrtype = IPPROTO_TCP;
            break;
        case IPPROTO_TCP:
            ctl->mtrtype = IPPROTO_ICMP;
            break;
        }
        return ActionNone;
        /* reserve to display help message -Min */
    case '?':
    case 'h':
        mvprintw(2, 0, "Command:\n");
        printw("  ?|h     help\n");
        printw("  p       pause (SPACE to resume)\n");
        printw("  d       switching display mode\n");
        printw("  e       toggle MPLS information on/off\n");
        printw("  n       toggle DNS on/off\n");
        printw("  r       reset all counters\n");
        printw
            ("  o str   set the columns to display, default str='LRS N BAWV'\n");
        printw
            ("  j       toggle latency(LS NABWV)/jitter(DR AGJMXI) stats\n");
        printw("  c <n>   report cycle n, default n=infinite\n");
        printw
            ("  i <n>   set the ping interval to n seconds, default n=1\n");
        printw
            ("  f <n>   set the initial time-to-live(ttl), default n=1\n");
        printw
            ("  m <n>   set the max time-to-live, default n= # of hops\n");
        printw("  s <n>   set the packet size to n or random(n<0)\n");
        printw
            ("  b <c>   set ping bit pattern to c(0..255) or random(c<0)\n");
        printw("  Q <t>   set ping packet's TOS to t\n");
        printw("  u       switch between ICMP ECHO and UDP datagrams\n");
#ifdef HAVE_IPINFO
        printw("  y       switching IP info\n");
        printw("  z       toggle ASN info on/off\n");
#endif
        printw("\n");
        printw(" press any key to go back...");
        getch();                /* read and ignore 'any key' */
        return ActionNone;
    default:                   /* ignore unknown input */
        return ActionNone;
    }
}


static void format_field(
    char *dst,
    int dst_length,
    const char *format,
    int n)
{
    if (index(format, 'N')) {
        *dst++ = ' ';
        format_number(n, 5, dst);
    } else if (strchr(format, 'f')) {
        /* this is for fields where we measure integer microseconds but
           display floating point miliseconds. Convert to float here. */
        snprintf(dst, dst_length, format, n / 1000.0);
        /* this was marked as a temporary hack over 10 years ago. -- REW */
    } else {
        snprintf(dst, dst_length, format, n);
    }
}

static void mtr_curses_hosts(
    struct mtr_ctl *ctl,
    int startstat)
{
    int max;
    int at;
    struct mplslen *mpls, *mplss;
    ip_t *addr, *addrs;
    int y;
    char *name;

    int i, j, k;
    int hd_len;
    char buf[1024];
    int __unused_int ATTRIBUTE_UNUSED;

    max = net_max(ctl);

    for (at = net_min(ctl) + ctl->display_offset; at < max; at++) {
        printw("%2d. ", at + 1);
        addr = net_addr(at);
        mpls = net_mpls(at);

        if (addrcmp((void *) addr, (void *) &ctl->unspec_addr, ctl->af) !=
            0) {
            name = dns_lookup(ctl, addr);
            if (!net_up(at))
                attron(A_BOLD);
#ifdef HAVE_IPINFO
            if (is_printii(ctl))
                printw(fmt_ipinfo(ctl, addr));
#endif
            if (name != NULL) {
                if (ctl->show_ips)
                    printw("%s (%s)", name, strlongip(ctl, addr));
                else
                    printw("%s", name);
            } else {
                printw("%s", strlongip(ctl, addr));
            }
            attroff(A_BOLD);

            getyx(stdscr, y, __unused_int);
            move(y, startstat);

            /* net_xxx returns times in usecs. Just display millisecs */
            hd_len = 0;
            for (i = 0; i < MAXFLD; i++) {
                /* Ignore options that don't exist */
                /* On the other hand, we now check the input side. Shouldn't happen, 
                   can't be careful enough. */
                j = ctl->fld_index[ctl->fld_active[i]];
                if (j == -1)
                    continue;
                format_field(buf + hd_len, sizeof(buf) - hd_len,
                             data_fields[j].format,
                             data_fields[j].net_xxx(at));
                hd_len += data_fields[j].length;
            }
            buf[hd_len] = 0;
            printw("%s", buf);

            for (k = 0; k < mpls->labels && ctl->enablempls; k++) {
                printw("\n    [MPLS: Lbl %lu Exp %u S %u TTL %u]",
                       mpls->label[k], mpls->exp[k], mpls->s[k],
                       mpls->ttl[k]);
            }

            /* Multi path */
            for (i = 0; i < MAXPATH; i++) {
                addrs = net_addrs(at, i);
                mplss = net_mplss(at, i);
                if (addrcmp((void *) addrs, (void *) addr, ctl->af) == 0)
                    continue;
                if (addrcmp
                    ((void *) addrs, (void *) &ctl->unspec_addr,
                     ctl->af) == 0)
                    break;

                name = dns_lookup(ctl, addrs);
                if (!net_up(at))
                    attron(A_BOLD);
                printw("\n    ");
#ifdef HAVE_IPINFO
                if (is_printii(ctl))
                    printw(fmt_ipinfo(ctl, addrs));
#endif
                if (name != NULL) {
                    if (ctl->show_ips)
                        printw("%s (%s)", name, strlongip(ctl, addrs));
                    else
                        printw("%s", name);
                } else {
                    printw("%s", strlongip(ctl, addrs));
                }
                for (k = 0; k < mplss->labels && ctl->enablempls; k++) {
                    printw("\n    [MPLS: Lbl %lu Exp %u S %u TTL %u]",
                           mplss->label[k], mplss->exp[k], mplss->s[k],
                           mplss->ttl[k]);
                }
                attroff(A_BOLD);
            }

        } else {
            printw("???");
        }

        printw("\n");
    }
    move(2, 0);
}

static void mtr_gen_scale(
    struct mtr_ctl *ctl)
{
    int *saved, i, max, at;
    int range;
    static int low_ms, high_ms;

    low_ms = 1000000;
    high_ms = -1;

    for (i = 0; i < NUM_FACTORS; i++) {
        scale[i] = 0;
    }
    max = net_max(ctl);
    for (at = ctl->display_offset; at < max; at++) {
        saved = net_saved_pings(at);
        for (i = 0; i < SAVED_PINGS; i++) {
            if (saved[i] < 0)
                continue;
            if (saved[i] < low_ms) {
                low_ms = saved[i];
            }
            if (saved[i] > high_ms) {
                high_ms = saved[i];
            }
        }
    }
    range = high_ms - low_ms;
    for (i = 0; i < NUM_FACTORS; i++) {
        scale[i] = low_ms + ((double) range * factors[i]);
    }
}

static void mtr_curses_init(
    void)
{
    int i;
    int block_split;

    /* Initialize factors to a log scale. */
    for (i = 0; i < NUM_FACTORS; i++) {
        factors[i] = ((double) 1 / NUM_FACTORS) * (i + 1);
        factors[i] *= factors[i];       /* Squared. */
    }

    /* Initialize block_map.  The block_split is always smaller than 9 */
    block_split = (NUM_FACTORS - 2) / 2;
    for (i = 1; i <= block_split; i++) {
        block_map[i] = '0' + i;
    }
    for (i = block_split + 1; i < NUM_FACTORS - 1; i++) {
        block_map[i] = 'a' + i - block_split - 1;
    }
    block_map[0] = '.';
    block_map[NUM_FACTORS - 1] = '>';
}

static void mtr_print_scaled(
    int ms)
{
    int i;

    for (i = 0; i < NUM_FACTORS; i++) {
        if (ms <= scale[i]) {
            attrset(block_col[i + 1]);
            printw("%c", block_map[i]);
            attrset(A_NORMAL);
            return;
        }
    }
    printw(">");
}


static void mtr_fill_graph(
    struct mtr_ctl *ctl,
    int at,
    int cols)
{
    int *saved;
    int i;

    saved = net_saved_pings(at);
    for (i = SAVED_PINGS - cols; i < SAVED_PINGS; i++) {
        if (saved[i] == -2) {
            printw(" ");
        } else if (saved[i] == -1) {
            attrset(block_col[0]);
            printw("%c", '?');
            attrset(A_NORMAL);
        } else {
            if (ctl->display_mode == DisplayModeBlockmap) {
                if (saved[i] > scale[6]) {
                    printw("%c", block_map[NUM_FACTORS - 1]);
                } else {
                    printw(".");
                }
            } else {
                mtr_print_scaled(saved[i]);
            }
        }
    }
}


static void mtr_curses_graph(
    struct mtr_ctl *ctl,
    int startstat,
    int cols)
{
    int max, at, y;
    ip_t *addr;
    char *name;
    int __unused_int ATTRIBUTE_UNUSED;

    max = net_max(ctl);

    for (at = ctl->display_offset; at < max; at++) {
        printw("%2d. ", at + 1);

        addr = net_addr(at);
        if (!addr) {
            printw("???\n");
            continue;
        }

        if (!net_up(at))
            attron(A_BOLD);
        if (addrcmp((void *) addr, (void *) &ctl->unspec_addr, ctl->af)) {
#ifdef HAVE_IPINFO
            if (is_printii(ctl))
                printw(fmt_ipinfo(ctl, addr));
#endif
            name = dns_lookup(ctl, addr);
            printw("%s", name ? name : strlongip(ctl, addr));
        } else
            printw("???");
        attroff(A_BOLD);

        getyx(stdscr, y, __unused_int);
        move(y, startstat);

        printw(" ");
        mtr_fill_graph(ctl, at, cols);
        printw("\n");
    }
}


void mtr_curses_redraw(
    struct mtr_ctl *ctl)
{
    int maxx;
    int startstat;
    int rowstat;
    time_t t;
    int __unused_int ATTRIBUTE_UNUSED;

    int i, j;
    int hd_len = 0;
    char buf[1024];
    char fmt[16];


    erase();
    getmaxyx(stdscr, __unused_int, maxx);

    rowstat = 5;

    move(0, 0);
    attron(A_BOLD);
    snprintf(buf, sizeof(buf), "%s%s%s", "My traceroute  [v",
             PACKAGE_VERSION, "]");
    pwcenter(buf);
    attroff(A_BOLD);

    mvprintw(1, 0, "%s (%s)", ctl->LocalHostname, net_localaddr());
    t = time(NULL);
    mvprintw(1, maxx - 25, iso_time(&t));
    printw("\n");

    printw("Keys:  ");
    attron(A_BOLD);
    printw("H");
    attroff(A_BOLD);
    printw("elp   ");
    attron(A_BOLD);
    printw("D");
    attroff(A_BOLD);
    printw("isplay mode   ");
    attron(A_BOLD);
    printw("R");
    attroff(A_BOLD);
    printw("estart statistics   ");
    attron(A_BOLD);
    printw("O");
    attroff(A_BOLD);
    printw("rder of fields   ");
    attron(A_BOLD);
    printw("q");
    attroff(A_BOLD);
    printw("uit\n");

    if (ctl->display_mode == DisplayModeDefault) {
        for (i = 0; i < MAXFLD; i++) {
            j = ctl->fld_index[ctl->fld_active[i]];
            if (j < 0)
                continue;

            snprintf(fmt, sizeof(fmt), "%%%ds", data_fields[j].length);
            snprintf(buf + hd_len, sizeof(buf) - hd_len, fmt,
                     data_fields[j].title);
            hd_len += data_fields[j].length;
        }
        attron(A_BOLD);
        mvprintw(rowstat - 1, 0, " Host");
        mvprintw(rowstat - 1, maxx - hd_len - 1, "%s", buf);
        mvprintw(rowstat - 2, maxx - hd_len - 1,
                 "   Packets               Pings");
        attroff(A_BOLD);

        move(rowstat, 0);
        mtr_curses_hosts(ctl, maxx - hd_len - 1);

    } else {
        char msg[80];
        int padding = 30;
        int max_cols;

#ifdef HAVE_IPINFO
        if (is_printii(ctl))
            padding += get_iiwidth(ctl->ipinfo_no);
#endif
        max_cols =
            maxx <= SAVED_PINGS + padding ? maxx - padding : SAVED_PINGS;
        startstat = padding - 2;

        snprintf(msg, sizeof(msg), " Last %3d pings", max_cols);
        mvprintw(rowstat - 1, startstat, msg);

        attroff(A_BOLD);
        move(rowstat, 0);

        mtr_gen_scale(ctl);
        mtr_curses_graph(ctl, startstat, max_cols);

        printw("\n");
        attron(A_BOLD);
        printw("Scale:");
        attroff(A_BOLD);

        for (i = 0; i < NUM_FACTORS - 1; i++) {
            printw("  ");
            attrset(block_col[i + 1]);
            printw("%c", block_map[i]);
            attrset(A_NORMAL);
            printw(":%d ms", scale[i] / 1000);
        }
        printw("  ");
        attrset(block_col[NUM_FACTORS]);
        printw("%c", block_map[NUM_FACTORS - 1]);
        attrset(A_NORMAL);
    }

    refresh();
}


void mtr_curses_open(
    struct mtr_ctl *ctl)
{
    int bg_col = 0;
    int i;

    initscr();
    raw();
    noecho();
    start_color();
    if (use_default_colors() == OK)
        bg_col = -1;
    for (i = 0; i < NUM_FACTORS; i++)
        init_pair(i + 1, i, bg_col);

    mtr_curses_init();
    mtr_curses_redraw(ctl);
}


void mtr_curses_close(
    void)
{
    printw("\n");
    endwin();
}


void mtr_curses_clear(
    struct mtr_ctl *ctl)
{
    mtr_curses_close();
    mtr_curses_open(ctl);
}
