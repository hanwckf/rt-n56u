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

#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#ifdef HAVE_ERROR_H
#include <error.h>
#else
#include "portability/error.h"
#endif

#include "mtr.h"
#include "cmdpipe.h"
#include "net.h"
#include "display.h"
#include "dns.h"
#include "utils.h"

#define MinSequence 33000
#define MaxSequence 65536

static int packetsize;          /* packet size used by ping */

static void sockaddrtop(
    struct sockaddr *saddr,
    char *strptr,
    size_t len);

struct nethost {
    ip_t addr;
    ip_t addrs[MAXPATH];        /* for multi paths byMin */
    int xmit;
    int returned;
    int sent;
    int up;
    long long ssd;              /* sum of squares of differences from the current average */
    int last;
    int best;
    int worst;
    int avg;                    /* average:  addByMin */
    int gmean;                  /* geometric mean: addByMin */
    int jitter;                 /* current jitter, defined as t1-t0 addByMin */
    int javg;                   /* avg jitter */
    int jworst;                 /* max jitter */
    int jinta;                  /* estimated variance,? rfc1889's "Interarrival Jitter" */
    int transit;
    int saved[SAVED_PINGS];
    int saved_seq_offset;
    struct mplslen mpls;
    struct mplslen mplss[MAXPATH];
};


struct sequence {
    int index;
    int transit;
    int saved_seq;
    struct timeval time;
};


static struct nethost host[MaxHost];
static struct sequence sequence[MaxSequence];
static struct packet_command_pipe_t packet_command_pipe;

#ifdef ENABLE_IPV6
static struct sockaddr_storage sourcesockaddr_struct;
static struct sockaddr_storage remotesockaddr_struct;
static struct sockaddr_in6 *ssa6 =
    (struct sockaddr_in6 *) &sourcesockaddr_struct;
static struct sockaddr_in6 *rsa6 =
    (struct sockaddr_in6 *) &remotesockaddr_struct;
#else
static struct sockaddr_in sourcesockaddr_struct;
static struct sockaddr_in remotesockaddr_struct;
#endif

static struct sockaddr *sourcesockaddr =
    (struct sockaddr *) &sourcesockaddr_struct;
static struct sockaddr *remotesockaddr =
    (struct sockaddr *) &remotesockaddr_struct;
static struct sockaddr_in *ssa4 =
    (struct sockaddr_in *) &sourcesockaddr_struct;
static struct sockaddr_in *rsa4 =
    (struct sockaddr_in *) &remotesockaddr_struct;

static ip_t *sourceaddress;
static ip_t *remoteaddress;

#ifdef ENABLE_IPV6
static char localaddr[INET6_ADDRSTRLEN];
#else
#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 16
#endif
static char localaddr[INET_ADDRSTRLEN];
#endif

static int batch_at = 0;
static int numhosts = 10;

/* return the number of microseconds to wait before sending the next
   ping */
int calc_deltatime(
    float waittime)
{
    waittime /= numhosts;
    return 1000000 * waittime;
}


static void save_sequence(
    struct mtr_ctl *ctl,
    int index,
    int seq)
{
    display_rawxmit(ctl, index, seq);

    sequence[seq].index = index;
    sequence[seq].transit = 1;
    sequence[seq].saved_seq = ++host[index].xmit;
    memset(&sequence[seq].time, 0, sizeof(sequence[seq].time));

    host[index].transit = 1;

    if (host[index].sent) {
        host[index].up = 0;
    }

    host[index].sent = 1;
    net_save_xmit(index);
}

static int new_sequence(
    struct mtr_ctl *ctl,
    int index)
{
    static int next_sequence = MinSequence;
    int seq;

    seq = next_sequence++;
    if (next_sequence >= MaxSequence) {
        next_sequence = MinSequence;
    }

    save_sequence(ctl, index, seq);

    return seq;
}


/*  Attempt to find the host at a particular number of hops away  */
static void net_send_query(
    struct mtr_ctl *ctl,
    int index,
    int packet_size)
{
    int seq = new_sequence(ctl, index);
    int time_to_live = index + 1;

    send_probe_command(ctl, &packet_command_pipe, remoteaddress,
                       sourceaddress, packetsize, seq, time_to_live);
}


/* We got a return on something we sent out.  Record the address and
   time.  */
static void net_process_ping(
    struct mtr_ctl *ctl,
    int seq,
    struct mplslen *mpls,
    ip_t * addr,
    int totusec)
{
    int index;
    int oldavg;                 /* usedByMin */
    int oldjavg;                /* usedByMin */
    int i;                      /* usedByMin */
#ifdef ENABLE_IPV6
    char addrcopy[sizeof(struct in6_addr)];
#else
    char addrcopy[sizeof(struct in_addr)];
#endif

    addrcpy((void *) &addrcopy, (char *) addr, ctl->af);

    if ((seq < 0) || (seq >= MaxSequence)) {
        return;
    }

    if (!sequence[seq].transit) {
        return;
    }
    sequence[seq].transit = 0;

    index = sequence[seq].index;

    if (addrcmp((void *) &(host[index].addr),
                (void *) &ctl->unspec_addr, ctl->af) == 0) {
        /* should be out of if as addr can change */
        addrcpy((void *) &(host[index].addr), addrcopy, ctl->af);
        host[index].mpls = *mpls;
        display_rawhost(ctl, index, (void *) &(host[index].addr));

        /* multi paths */
        addrcpy((void *) &(host[index].addrs[0]), addrcopy, ctl->af);
        host[index].mplss[0] = *mpls;
    } else {
        for (i = 0; i < MAXPATH;) {
            if (addrcmp
                ((void *) &(host[index].addrs[i]), (void *) &addrcopy,
                 ctl->af) == 0
                || addrcmp((void *) &(host[index].addrs[i]),
                           (void *) &ctl->unspec_addr, ctl->af) == 0) {
                break;
            }
            i++;
        }

        if (addrcmp((void *) &(host[index].addrs[i]), addrcopy, ctl->af) !=
            0 && i < MAXPATH) {
            addrcpy((void *) &(host[index].addrs[i]), addrcopy, ctl->af);
            host[index].mplss[i] = *mpls;
            display_rawhost(ctl, index, (void *) &(host[index].addrs[i]));
        }
    }

    host[index].jitter = totusec - host[index].last;
    if (host[index].jitter < 0) {
        host[index].jitter = -host[index].jitter;
    }

    host[index].last = totusec;

    if (host[index].returned < 1) {
        host[index].best = host[index].worst = host[index].gmean = totusec;
        host[index].avg = host[index].ssd = 0;

        host[index].jitter = host[index].jworst = host[index].jinta = 0;
    }

    if (totusec < host[index].best) {
        host[index].best = totusec;
    }
    if (totusec > host[index].worst) {
        host[index].worst = totusec;
    }

    if (host[index].jitter > host[index].jworst) {
        host[index].jworst = host[index].jitter;
    }

    host[index].returned++;
    oldavg = host[index].avg;
    host[index].avg += (totusec - oldavg + .0) / host[index].returned;
    host[index].ssd +=
        (totusec - oldavg + .0) * (totusec - host[index].avg);

    oldjavg = host[index].javg;
    host[index].javg +=
        (host[index].jitter - oldjavg) / host[index].returned;
    /* below algorithm is from rfc1889, A.8 */
    host[index].jinta +=
        host[index].jitter - ((host[index].jinta + 8) >> 4);

    if (host[index].returned > 1) {
        host[index].gmean =
            pow((double) host[index].gmean,
                (host[index].returned - 1.0) / host[index].returned)
            * pow((double) totusec, 1.0 / host[index].returned);
    }

    host[index].sent = 0;
    host[index].up = 1;
    host[index].transit = 0;

    net_save_return(index, sequence[seq].saved_seq, totusec);
    display_rawping(ctl, index, totusec, seq);
}

/*
    Invoked when the read pipe from the mtr-packet subprocess is readable.
    If we have received a complete reply, process it.
*/
void net_process_return(
    struct mtr_ctl *ctl)
{
    handle_command_replies(ctl, &packet_command_pipe, net_process_ping);
}


ip_t *net_addr(
    int at)
{
    return (ip_t *) & (host[at].addr);
}


ip_t *net_addrs(
    int at,
    int i)
{
    return (ip_t *) & (host[at].addrs[i]);
}

void *net_mpls(
    int at)
{
    return (struct mplslen *) &(host[at].mplss);
}

void *net_mplss(
    int at,
    int i)
{
    return (struct mplslen *) &(host[at].mplss[i]);
}

int net_loss(
    int at)
{
    if ((host[at].xmit - host[at].transit) == 0) {
        return 0;
    }

    /* times extra 1000 */
    return 1000 * (100 -
                   (100.0 * host[at].returned /
                    (host[at].xmit - host[at].transit)));
}


int net_drop(
    int at)
{
    return (host[at].xmit - host[at].transit) - host[at].returned;
}


int net_last(
    int at)
{
    return (host[at].last);
}


int net_best(
    int at)
{
    return (host[at].best);
}


int net_worst(
    int at)
{
    return (host[at].worst);
}


int net_avg(
    int at)
{
    return (host[at].avg);
}


int net_gmean(
    int at)
{
    return (host[at].gmean);
}


int net_stdev(
    int at)
{
    if (host[at].returned > 1) {
        return (sqrt(host[at].ssd / (host[at].returned - 1.0)));
    } else {
        return (0);
    }
}


int net_jitter(
    int at)
{
    return (host[at].jitter);
}


int net_jworst(
    int at)
{
    return (host[at].jworst);
}


int net_javg(
    int at)
{
    return (host[at].javg);
}


int net_jinta(
    int at)
{
    return (host[at].jinta);
}


int net_max(
    struct mtr_ctl *ctl)
{
    int at;
    int max;

    max = 0;
    for (at = 0; at < ctl->maxTTL - 1; at++) {
        if (addrcmp((void *) &(host[at].addr),
                    (void *) remoteaddress, ctl->af) == 0) {
            return at + 1;
        } else if (addrcmp((void *) &(host[at].addr),
                           (void *) &ctl->unspec_addr, ctl->af) != 0) {
            max = at + 2;
        }
    }

    return max;
}


int net_min(
    struct mtr_ctl *ctl)
{
    return (ctl->fstTTL - 1);
}


int net_returned(
    int at)
{
    return host[at].returned;
}


int net_xmit(
    int at)
{
    return host[at].xmit;
}


int net_up(
    int at)
{
    return host[at].up;
}


char *net_localaddr(
    void)
{
    return localaddr;
}


void net_end_transit(
    void)
{
    int at;

    for (at = 0; at < MaxHost; at++) {
        host[at].transit = 0;
    }
}

int net_send_batch(
    struct mtr_ctl *ctl)
{
    int n_unknown = 0, i;

    /* randomized packet size and/or bit pattern if packetsize<0 and/or 
       bitpattern<0.  abs(packetsize) and/or abs(bitpattern) will be used 
     */
    if (batch_at < ctl->fstTTL) {
        if (ctl->cpacketsize < 0) {
            /* Someone used a formula here that tried to correct for the 
               "end-error" in "rand()". By "end-error" I mean that if you 
               have a range for "rand()" that runs to 32768, and the 
               destination range is 10000, you end up with 4 out of 32768 
               0-2768's and only 3 out of 32768 for results 2769 .. 9999. 
               As our detination range (in the example 10000) is much 
               smaller (reasonable packet sizes), and our rand() range much 
               larger, this effect is insignificant. Oh! That other formula
               didn't work. */
            packetsize =
                MINPACKET + rand() % (-ctl->cpacketsize - MINPACKET);
        } else {
            packetsize = ctl->cpacketsize;
        }
        if (ctl->bitpattern < 0) {
            ctl->bitpattern =
                -(int) (256 + 255 * (rand() / (RAND_MAX + 0.1)));
        }
    }

    net_send_query(ctl, batch_at, abs(packetsize));

    for (i = ctl->fstTTL - 1; i < batch_at; i++) {
        if (addrcmp
            ((void *) &(host[i].addr), (void *) &ctl->unspec_addr,
             ctl->af) == 0)
            n_unknown++;

        /* The second condition in the next "if" statement was added in mtr-0.56, 
           but I don't remember why. It makes mtr stop skipping sections of unknown
           hosts. Removed in 0.65. 
           If the line proves necessary, it should at least NOT trigger that line
           when host[i].addr == 0 */
        if ((addrcmp((void *) &(host[i].addr),
                     (void *) remoteaddress, ctl->af) == 0))
            n_unknown = MaxHost;        /* Make sure we drop into "we should restart" */
    }

    if (                        /* success in reaching target */
           (addrcmp((void *) &(host[batch_at].addr),
                    (void *) remoteaddress, ctl->af) == 0) ||
           /* fail in consecutive maxUnknown (firewall?) */
           (n_unknown > ctl->maxUnknown) ||
           /* or reach limit  */
           (batch_at >= ctl->maxTTL - 1)) {
        numhosts = batch_at + 1;
        batch_at = ctl->fstTTL - 1;
        return 1;
    }

    batch_at++;
    return 0;
}


/*  Ensure the interface address a valid address for our use  */
static void net_validate_interface_address(
    int address_family,
    char *interface_address)
{
    if (inet_pton(address_family, interface_address, sourceaddress) != 1) {
        error(EXIT_FAILURE, errno, "invalid local address");
    }

    if (inet_ntop
        (address_family, sourceaddress, localaddr,
         sizeof(localaddr)) == NULL) {
        error(EXIT_FAILURE, errno, "invalid local address");
    }
}


/*
  Find the local address we will use to sent to the remote
  host by connecting a UDP socket and checking the address
  the socket is bound to.
*/
static void net_find_local_address(
    void)
{
    int udp_socket;
    int addr_length;
    struct sockaddr_storage remote_sockaddr;
    struct sockaddr_in *remote4;
    struct sockaddr_in6 *remote6;

    udp_socket =
        socket(remotesockaddr->sa_family, SOCK_DGRAM, IPPROTO_UDP);
    if (udp_socket == -1) {
        error(EXIT_FAILURE, errno, "udp socket creation failed");
    }

    /*
       We need to set the port to a non-zero value for the connect
       to succeed.
     */
    if (remotesockaddr->sa_family == AF_INET6) {
#ifdef ENABLE_IPV6
        addr_length = sizeof(struct sockaddr_in6);

        memcpy(&remote_sockaddr, rsa6, addr_length);
        remote6 = (struct sockaddr_in6 *) &remote_sockaddr;
        remote6->sin6_port = htons(1);
#endif
    } else {
        addr_length = sizeof(struct sockaddr_in);

        memcpy(&remote_sockaddr, rsa4, addr_length);
        remote4 = (struct sockaddr_in *) &remote_sockaddr;
        remote4->sin_port = htons(1);
    }

    if (connect
        (udp_socket, (struct sockaddr *) &remote_sockaddr, addr_length)) {
        error(EXIT_FAILURE, errno, "udp socket connect failed");
    }

    if (getsockname(udp_socket, sourcesockaddr, &addr_length)) {

        error(EXIT_FAILURE, errno, "local address determination failed");
    }

    sockaddrtop(sourcesockaddr, localaddr, sizeof(localaddr));

    close(udp_socket);
}


int net_open(
    struct mtr_ctl *ctl,
    struct hostent *hostent)
{
    int err;

    /*  Spawn the mtr-packet child process  */
    err = open_command_pipe(ctl, &packet_command_pipe);
    if (err) {
        return err;
    }

    net_reset(ctl);

    remotesockaddr->sa_family = hostent->h_addrtype;

    switch (hostent->h_addrtype) {
    case AF_INET:
        addrcpy((void *) &(rsa4->sin_addr), hostent->h_addr, AF_INET);
        sourceaddress = (ip_t *) & (ssa4->sin_addr);
        remoteaddress = (ip_t *) & (rsa4->sin_addr);
        break;
#ifdef ENABLE_IPV6
    case AF_INET6:
        addrcpy((void *) &(rsa6->sin6_addr), hostent->h_addr, AF_INET6);
        sourceaddress = (ip_t *) & (ssa6->sin6_addr);
        remoteaddress = (ip_t *) & (rsa6->sin6_addr);
        break;
#endif
    default:
        error(EXIT_FAILURE, 0, "net_open bad address type");
    }

    if (ctl->InterfaceAddress) {
        net_validate_interface_address(ctl->af, ctl->InterfaceAddress);
    } else {
        net_find_local_address();
    }

    return 0;
}


void net_reopen(
    struct mtr_ctl *ctl,
    struct hostent *addr)
{
    int at;

    for (at = 0; at < MaxHost; at++) {
        memset(&host[at], 0, sizeof(host[at]));
    }

    remotesockaddr->sa_family = addr->h_addrtype;
    addrcpy((void *) remoteaddress, addr->h_addr, addr->h_addrtype);

    switch (addr->h_addrtype) {
    case AF_INET:
        addrcpy((void *) &(rsa4->sin_addr), addr->h_addr, AF_INET);
        break;
#ifdef ENABLE_IPV6
    case AF_INET6:
        addrcpy((void *) &(rsa6->sin6_addr), addr->h_addr, AF_INET6);
        break;
#endif
    default:
        error(EXIT_FAILURE, 0, "net_reopen bad address type");
    }

    net_reset(ctl);
    net_send_batch(ctl);
}


void net_reset(
    struct mtr_ctl *ctl)
{
    static struct nethost template = {
        .saved_seq_offset = 2 - SAVED_PINGS
    };

    int at, i;

    batch_at = ctl->fstTTL - 1; /* above replacedByMin */
    numhosts = 10;

    for (i = 0; i < SAVED_PINGS; i++)
        template.saved[i] = -2;

    for (at = 0; at < MaxHost; at++) {
        memcpy(&(host[at]), &template, sizeof(template));
    }

    for (at = 0; at < MaxSequence; at++) {
        sequence[at].transit = 0;
    }

}


/*  Close the pipe to the packet generator process, and kill the process  */
void net_close(
    void)
{
    close_command_pipe(&packet_command_pipe);
}


int net_waitfd(
    void)
{
    return packet_command_pipe.read_fd;
}


int *net_saved_pings(
    int at)
{
    return host[at].saved;
}


static void net_save_increment(
    void)
{
    int at;
    for (at = 0; at < MaxHost; at++) {
        memmove(host[at].saved, host[at].saved + 1,
                (SAVED_PINGS - 1) * sizeof(int));
        host[at].saved[SAVED_PINGS - 1] = -2;
        host[at].saved_seq_offset += 1;
    }
}


void net_save_xmit(
    int at)
{
    if (host[at].saved[SAVED_PINGS - 1] != -2)
        net_save_increment();
    host[at].saved[SAVED_PINGS - 1] = -1;
}


void net_save_return(
    int at,
    int seq,
    int ms)
{
    int idx;
    idx = seq - host[at].saved_seq_offset;
    if ((idx < 0) || (idx >= SAVED_PINGS)) {
        return;
    }
    host[at].saved[idx] = ms;
}

/* Similar to inet_ntop but uses a sockaddr as it's argument. */
static void sockaddrtop(
    struct sockaddr *saddr,
    char *strptr,
    size_t len)
{
    struct sockaddr_in *sa4;
#ifdef ENABLE_IPV6
    struct sockaddr_in6 *sa6;
#endif

    switch (saddr->sa_family) {
    case AF_INET:
        sa4 = (struct sockaddr_in *) saddr;
        xstrncpy(strptr, inet_ntoa(sa4->sin_addr), len - 1);
        strptr[len - 1] = '\0';
        return;
#ifdef ENABLE_IPV6
    case AF_INET6:
        sa6 = (struct sockaddr_in6 *) saddr;
        inet_ntop(sa6->sin6_family, &(sa6->sin6_addr), strptr, len);
        return;
#endif
    default:
        error(0, 0, "sockaddrtop unknown address type");
        strptr[0] = '\0';
        return;
    }
}


/* Address comparison. */
int addrcmp(
    char *a,
    char *b,
    int family)
{
    int rc = -1;

    switch (family) {
    case AF_INET:
        rc = memcmp(a, b, sizeof(struct in_addr));
        break;
#ifdef ENABLE_IPV6
    case AF_INET6:
        rc = memcmp(a, b, sizeof(struct in6_addr));
        break;
#endif
    }

    return rc;
}

/* Address copy. */
void addrcpy(
    char *a,
    char *b,
    int family)
{

    switch (family) {
    case AF_INET:
        memcpy(a, b, sizeof(struct in_addr));
        break;
#ifdef ENABLE_IPV6
    case AF_INET6:
        memcpy(a, b, sizeof(struct in6_addr));
        break;
#endif
    }
}

/* for GTK frontend */
void net_harvest_fds(
    struct mtr_ctl *ctl)
{
    fd_set writefd;
    int maxfd = 0;
    struct timeval tv;

    FD_ZERO(&writefd);
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    select(maxfd, NULL, &writefd, NULL, &tv);
}
