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
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/select.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#ifdef HAVE_ERROR_H
#include <error.h>
#else
#include "portability/error.h"
#endif

#include "mtr.h"
#include "dns.h"
#include "net.h"
#include "asn.h"
#include "display.h"
#include "select.h"

void select_loop(
    struct mtr_ctl *ctl)
{
    fd_set readfd;
    fd_set writefd;
    int anyset = 0;
    int maxfd = 0;
    int dnsfd, netfd;
#ifdef ENABLE_IPV6
    int dnsfd6;
#endif
    int NumPing = 0;
    int paused = 0;
    struct timeval lasttime, thistime, selecttime;
    struct timeval startgrace;
    int dt;
    int rv;
    int graceperiod = 0;
    struct timeval intervaltime;
    static double dnsinterval = 0;

    memset(&startgrace, 0, sizeof(startgrace));

    gettimeofday(&lasttime, NULL);

    while (1) {
        dt = calc_deltatime(ctl->WaitTime);
        intervaltime.tv_sec = dt / 1000000;
        intervaltime.tv_usec = dt % 1000000;

        FD_ZERO(&readfd);
        FD_ZERO(&writefd);

        maxfd = 0;

        if (ctl->Interactive) {
            FD_SET(0, &readfd);
            maxfd = 1;
        }
#ifdef ENABLE_IPV6
        if (ctl->dns) {
            dnsfd6 = dns_waitfd6();
            if (dnsfd6 >= 0) {
                FD_SET(dnsfd6, &readfd);
                if (dnsfd6 >= maxfd)
                    maxfd = dnsfd6 + 1;
            } else {
                dnsfd6 = 0;
            }
        } else
            dnsfd6 = 0;
#endif
        if (ctl->dns) {
            dnsfd = dns_waitfd();
            FD_SET(dnsfd, &readfd);
            if (dnsfd >= maxfd)
                maxfd = dnsfd + 1;
        } else
            dnsfd = 0;

        netfd = net_waitfd();
        FD_SET(netfd, &readfd);
        if (netfd >= maxfd)
            maxfd = netfd + 1;

        do {
            if (anyset || paused) {
                /* Set timeout to 0.1s.
                 * While this is almost instantaneous for human operators,
                 * it's slow enough for computers to go do something else;
                 * this prevents mtr from hogging 100% CPU time on one core.
                 */
                selecttime.tv_sec = 0;
                selecttime.tv_usec = paused ? 100000 : 0;

                rv = select(maxfd, (void *) &readfd, &writefd, NULL,
                            &selecttime);

            } else {
                if (ctl->Interactive)
                    display_redraw(ctl);

                gettimeofday(&thistime, NULL);

                if (thistime.tv_sec > lasttime.tv_sec + intervaltime.tv_sec
                    || (thistime.tv_sec ==
                        lasttime.tv_sec + intervaltime.tv_sec
                        && thistime.tv_usec >=
                        lasttime.tv_usec + intervaltime.tv_usec)) {
                    lasttime = thistime;

                    if (!graceperiod) {
                        if (NumPing >= ctl->MaxPing
                            && (!ctl->Interactive || ctl->ForceMaxPing)) {
                            graceperiod = 1;
                            startgrace = thistime;
                        }

                        /* do not send out batch when we've already initiated grace period */
                        if (!graceperiod && net_send_batch(ctl))
                            NumPing++;
                    }
                }

                if (graceperiod) {
                    dt = (thistime.tv_usec - startgrace.tv_usec) +
                        1000000 * (thistime.tv_sec - startgrace.tv_sec);
                    if ((ctl->GraceTime * 1000 * 1000) < dt)
                        return;
                }

                selecttime.tv_usec = (thistime.tv_usec - lasttime.tv_usec);
                selecttime.tv_sec = (thistime.tv_sec - lasttime.tv_sec);
                if (selecttime.tv_usec < 0) {
                    --selecttime.tv_sec;
                    selecttime.tv_usec += 1000000;
                }
                selecttime.tv_usec =
                    intervaltime.tv_usec - selecttime.tv_usec;
                selecttime.tv_sec =
                    intervaltime.tv_sec - selecttime.tv_sec;
                if (selecttime.tv_usec < 0) {
                    --selecttime.tv_sec;
                    selecttime.tv_usec += 1000000;
                }

                if (ctl->dns) {
                    if ((selecttime.tv_sec > (time_t) dnsinterval) ||
                        ((selecttime.tv_sec == (time_t) dnsinterval) &&
                         (selecttime.tv_usec >
                          ((time_t) (dnsinterval * 1000000) % 1000000)))) {
                        selecttime.tv_sec = (time_t) dnsinterval;
                        selecttime.tv_usec =
                            (time_t) (dnsinterval * 1000000) % 1000000;
                    }
                }

                rv = select(maxfd, (void *) &readfd, NULL, NULL,
                            &selecttime);
            }
        } while ((rv < 0) && (errno == EINTR));

        if (rv < 0) {
            error(EXIT_FAILURE, errno, "Select failed");
        }
        anyset = 0;

        /*  Have we got new packets back?  */
        if (FD_ISSET(netfd, &readfd)) {
            net_process_return(ctl);
            anyset = 1;
        }

        if (ctl->dns) {
            /* Handle any pending resolver events */
            dnsinterval = ctl->WaitTime;
        }

        /*  Have we finished a nameservice lookup?  */
#ifdef ENABLE_IPV6
        if (ctl->dns && dnsfd6 && FD_ISSET(dnsfd6, &readfd)) {
            dns_ack6();
            anyset = 1;
        }
#endif
        if (ctl->dns && dnsfd && FD_ISSET(dnsfd, &readfd)) {
            dns_ack(ctl);
            anyset = 1;
        }

        /*  Has a key been pressed?  */
        if (FD_ISSET(0, &readfd)) {
            switch (display_keyaction(ctl)) {
            case ActionQuit:
                return;
                break;
            case ActionReset:
                net_reset(ctl);
                break;
            case ActionDisplay:
                ctl->display_mode =
                    (ctl->display_mode + 1) % DisplayModeMAX;
                break;
            case ActionClear:
                display_clear(ctl);
                break;
            case ActionPause:
                paused = 1;
                break;
            case ActionResume:
                paused = 0;
                break;
            case ActionMPLS:
                ctl->enablempls = !ctl->enablempls;
                display_clear(ctl);
                break;
            case ActionDNS:
                if (ctl->dns) {
                    ctl->use_dns = !ctl->use_dns;
                    display_clear(ctl);
                }
                break;
#ifdef HAVE_IPINFO
            case ActionII:
                ctl->ipinfo_no++;
                if (ctl->ipinfo_no > ctl->ipinfo_max)
                    ctl->ipinfo_no = 0;
                break;
            case ActionAS:
                ctl->ipinfo_no = ctl->ipinfo_no ? 0 : ctl->ipinfo_max;
                break;
#endif

            case ActionScrollDown:
                ctl->display_offset += 5;
                break;
            case ActionScrollUp:
                ctl->display_offset -= 5;
                if (ctl->display_offset < 0) {
                    ctl->display_offset = 0;
                }
                break;
            }
            anyset = 1;
        }
    }
    return;
}
