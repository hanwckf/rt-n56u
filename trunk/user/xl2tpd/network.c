/*
 * Layer Two Tunnelling Protocol Daemon
 * Copyright (C) 1998 Adtran, Inc.
 * Copyright (C) 2002 Jeff McAdams
 *
 * Mark Spencer
 *
 * This software is distributed under the terms
 * of the GPL, which you should have received
 * along with this source.
 *
 * Network routines for UDP handling
 */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#ifndef LINUX
# include <sys/uio.h>
#endif
#include <sys/resource.h>
#include "l2tp.h"
#include "ipsecmast.h"
#include "misc.h"    /* for IPADDY macro */

char hostname[256];
int server_socket = -1;         /* Server socket */
#ifdef USE_KERNEL
int kernel_support;             /* Kernel Support there or not? */
#endif

int init_network (void)
{
    long arg;
    struct sockaddr_in server;
    unsigned int length = sizeof (server);

    gethostname (hostname, sizeof (hostname));

    /* create server socket only has lns */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = gconfig.listenaddr; 
    server.sin_port = htons (gconfig.port);

    if ((server_socket = socket (PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        l2tp_log (LOG_CRIT, "%s: Unable to allocate socket. Terminating.\n", __FUNCTION__);
        return -EINVAL;
    }

    arg=1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &arg, sizeof(arg));
#ifdef SO_NO_CHECK
    setsockopt(server_socket, SOL_SOCKET, SO_NO_CHECK, &arg, sizeof(arg));
#endif

    if (bind (server_socket, (struct sockaddr *) &server, sizeof (server)))
    {
        close (server_socket);
        l2tp_log (LOG_CRIT, "%s: Unable to bind socket: %s. Terminating.\n", __FUNCTION__, strerror(errno), errno);
        return -EINVAL;
    }

    if (getsockname (server_socket, (struct sockaddr *) &server, &length))
    {
        close (server_socket);
        l2tp_log (LOG_CRIT, "%s: Unable to read socket name.Terminating.\n", __FUNCTION__);
        return -EINVAL;
    }

#ifdef LINUX
    /*
     * For L2TP/IPsec with KLIPSng, set the socket to receive IPsec REFINFO
     * values.
     */
    arg=1;
    if(setsockopt(server_socket, IPPROTO_IP, gconfig.sarefnum, &arg, sizeof(arg)) != 0) {
#ifdef DEBUG_MORE
        l2tp_log(LOG_CRIT, "setsockopt recvref[%d]: %s\n", gconfig.sarefnum, strerror(errno));
#endif
        gconfig.ipsecsaref=0;
    }
    
    arg=1;
    if(setsockopt(server_socket, IPPROTO_IP, IP_PKTINFO, (char*)&arg, sizeof(arg)) != 0) {
	    l2tp_log(LOG_CRIT, "setsockopt IP_PKTINFO: %s\n", strerror(errno));
    }
#else
    l2tp_log(LOG_INFO, "No attempt being made to use IPsec SAref's since we're not on a Linux machine.\n");
#endif

#ifdef USE_KERNEL
    if (gconfig.forceuserspace)
    {
        l2tp_log (LOG_INFO, "Not looking for kernel support.\n");
        kernel_support = 0;
    }
    else
    {
        int kernel_fd = socket(AF_PPPOX, SOCK_DGRAM, PX_PROTO_OL2TP);
        if (kernel_fd < 0)
        {
            l2tp_log (LOG_INFO, "L2TP kernel support not detected.\n");
            kernel_support = 0;
        }
        else
        {
            close(kernel_fd);
            l2tp_log (LOG_INFO, "Using l2tp kernel support.\n");
            kernel_support = -1;
        }
    }
#else
    l2tp_log (LOG_INFO, "This binary does not support kernel L2TP.\n");
#endif

    arg = fcntl (server_socket, F_GETFL);
    arg |= O_NONBLOCK;
    fcntl (server_socket, F_SETFL, arg);
    gconfig.port = ntohs (server.sin_port);

    return 0;
}

static inline void extract (void *buf, int *tunnel, int *call)
{
    /*
     * Extract the tunnel and call #'s, and fix the order of the 
     * version
     */

    struct payload_hdr *p = (struct payload_hdr *) buf;
    if (PLBIT (p->ver))
    {
        *tunnel = p->tid;
        *call = p->cid;
    }
    else
    {
        *tunnel = p->length;
        *call = p->tid;
    }
}

static inline void fix_hdr (void *buf)
{
    /*
     * Fix the byte order of the header
     */

    struct payload_hdr *p = (struct payload_hdr *) buf;
    _u16 ver = ntohs (p->ver);
    if (CTBIT (p->ver))
    {
        /*
         * Control headers are always
         * exactly 12 bytes big.
         */
        swaps (buf, 12);
    }
    else
    {
        int len = 6;
        if (PSBIT (ver))
            len += 2;
        if (PLBIT (ver))
            len += 2;
        if (PFBIT (ver))
            len += 4;
        swaps (buf, len);
    }
}

void dethrottle (void *call)
{
/*	struct call *c = (struct call *)call; */
/*	if (c->throttle) {
#ifdef DEBUG_FLOW
		log(LOG_DEBUG, "%s: dethrottling call %d, and setting R-bit\n",__FUNCTION__,c->ourcid); 
#endif 		c->rbit = RBIT;
		c->throttle = 0;
	} else {
		log(LOG_DEBUG, "%s:  call %d already dethrottled?\n",__FUNCTION__,c->ourcid); 	
	} */
}

void foo (void *bar) {
}

void control_xmit (void *b)
{
    struct buffer *buf = (struct buffer *) b;
    struct tunnel *t;
    struct timeval tv;
    int ns;

    if (!buf)
    {
        l2tp_log (LOG_WARNING, "%s: called on NULL buffer!\n", __FUNCTION__);
        return;
    }

    ns = ntohs (((struct control_hdr *) (buf->start))->Ns);

    t = buf->tunnel;
    if (t)
    {
#ifdef DEBUG_CONTROL_XMIT
        l2tp_log (LOG_DEBUG,
                    "trying to send control packet %d to %d\n", ns, t->ourtid);
#endif
        if (ns < t->cLr)
        {
#ifdef DEBUG_CONTROL_XMIT
            l2tp_log (LOG_DEBUG, "%s: Tossing packet %d\n", __FUNCTION__, ns);
#endif
            /* Okay, it's been received.  Let's toss it now */
            toss (buf);
            return;
        }
    }

    buf->retries++;
    if (buf->retries > DEFAULT_MAX_RETRIES)
    {
        /*
           * Too many retries.  Either kill the tunnel, or
           * if there is no tunnel, just stop retransmitting.
         */
        if (t)
        {
            if (t->self->needclose)
            {
                l2tp_log (LOG_DEBUG,
                     "Unable to deliver closing message for tunnel %d. Destroying anyway.\n",
                     t->ourtid);
                t->self->needclose = 0;
                t->self->closing = -1;
    		tv.tv_sec = 1;
        	tv.tv_usec = 0;
        	schedule (tv, foo, t);
            }
            else
            {
                l2tp_log (LOG_NOTICE,
                     "Maximum retries exceeded for tunnel %d.  Closing.\n",
                     t->ourtid);
                strcpy (t->self->errormsg, "Timeout");
                t->self->needclose = -1;
    		tv.tv_sec = 1;
        	tv.tv_usec = 0;
        	schedule (tv, foo, t);
            }
	    call_close(t->self);
        }
	toss (buf);
    }
    else
    {
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        if (buf->retries > 1)
            tv.tv_sec = 1LL << (buf->retries-1);

        schedule (tv, control_xmit, buf);
#ifdef DEBUG_CONTROL_XMIT
        l2tp_log (LOG_DEBUG, "%s: Scheduling and transmitting packet %d, retries: %d\n",
             __FUNCTION__, ns, buf->retries);
#endif
        udp_xmit (buf, t);
    }
}

void udp_xmit (struct buffer *buf, struct tunnel *t)
{
    struct cmsghdr *cmsg = NULL;
    char cbuf[CMSG_SPACE(sizeof (unsigned int) + sizeof (struct in_pktinfo))];
    unsigned int *refp;
    struct msghdr msgh;
    int err;
    struct iovec iov;
    struct in_pktinfo *pktinfo;
    int finallen = 0;

    /*
     * OKAY, now send a packet with the right SAref values.
     */
    memset(&msgh, 0, sizeof(struct msghdr));
    msgh.msg_control = cbuf;
    msgh.msg_controllen = sizeof(cbuf);

    if (gconfig.ipsecsaref && t->refhim != IPSEC_SAREF_NULL) {
	cmsg = CMSG_FIRSTHDR(&msgh);
	cmsg->cmsg_level = IPPROTO_IP;
	cmsg->cmsg_type  = gconfig.sarefnum;
	cmsg->cmsg_len   = CMSG_LEN(sizeof(unsigned int));

	if(gconfig.debug_network) {
		l2tp_log(LOG_DEBUG,"sending with saref=%d using sarefnum=%d\n", t->refhim, gconfig.sarefnum);
	}
	refp = (unsigned int *)CMSG_DATA(cmsg);
	*refp = t->refhim;
	
	finallen = cmsg->cmsg_len;
    }

    if (t->my_addr.ipi_addr.s_addr){

	if ( ! cmsg) {
		cmsg = CMSG_FIRSTHDR(&msgh);
	}
	else {
		cmsg = CMSG_NXTHDR(&msgh, cmsg);
	}
	
	cmsg->cmsg_level = IPPROTO_IP;
	cmsg->cmsg_type = IP_PKTINFO;
	cmsg->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));

	pktinfo = (struct in_pktinfo*) CMSG_DATA(cmsg);
	*pktinfo = t->my_addr;
	
	finallen += cmsg->cmsg_len;
    }

    /*
     * Some OS don't like assigned buffer with zero length (e.g. OpenBSD),
     * some OS don't like empty buffer with non-zero length (e.g. Linux).
     * So make them all happy by assigning control buffer only if we really
     * have something there and zero both fields otherwise.
     */
    msgh.msg_controllen = finallen;
    if (!finallen)
        msgh.msg_control = NULL;

    iov.iov_base = buf->start;
    iov.iov_len  = buf->len;

    /* return packet from whence it came */
    msgh.msg_name    = &buf->peer;
    msgh.msg_namelen = sizeof(buf->peer);

    msgh.msg_iov  = &iov;
    msgh.msg_iovlen = 1;
    msgh.msg_flags = 0;

    /* Receive one packet. */
    if ((err = sendmsg(server_socket, &msgh, 0)) < 0) {
	l2tp_log(LOG_ERR, "udp_xmit failed to %s:%d with err=%d:%s\n",
		 IPADDY(t->peer.sin_addr), ntohs(t->peer.sin_port),
		 err,strerror(errno));
    }
}

int build_fdset (fd_set *readfds)
{
	struct tunnel *tun;
	struct call *call;
	int max = 0;

	tun = tunnels.head;
	FD_ZERO (readfds);

	while (tun)
	{
		if (tun->udp_fd > -1) {
			if (tun->udp_fd > max)
				max = tun->udp_fd;
			FD_SET (tun->udp_fd, readfds);
		}
		call = tun->call_head;
		while (call)
		{
			if (call->needclose ^ call->closing)
			{
				call_close (call);
				call = tun->call_head;
				if (!call)
					break;
				continue;
			}
			if (call->fd > -1)
			{
				if (!call->needclose && !call->closing)
				{
					if (call->fd > max)
						max = call->fd;
					FD_SET (call->fd, readfds);
				}
			}
			call = call->next;
		}
		/* Now that call fds have been collected, and checked for
		 * closing, check if the tunnel needs to be closed too
		 */
		if (tun->self->needclose ^ tun->self->closing)
		{
			if (gconfig.debug_tunnel)
				l2tp_log (LOG_DEBUG, "%s: closing down tunnel %d\n",
						__FUNCTION__, tun->ourtid);
			call_close (tun->self);
			/* Reset the while loop
			 * and check for NULL */
			tun = tunnels.head;
			if (!tun)
				break;
			continue;
		}
		tun = tun->next;
	}
	FD_SET (server_socket, readfds);
	if (server_socket > max)
		max = server_socket;
	FD_SET (control_fd, readfds);
	if (control_fd > max)
		max = control_fd;
	return max;
}

void network_thread ()
{
    /*
     * We loop forever waiting on either data from the ppp drivers or from
     * our network socket.  Control handling is no longer done here.
     */
    struct sockaddr_in from;
    struct in_pktinfo to;
    unsigned int fromlen;
    int tunnel, call;           /* Tunnel and call */
    int recvsize;               /* Length of data received */
    struct buffer *buf;         /* Payload buffer */
    struct call *c, *sc;        /* Call to send this off to */
    struct tunnel *st;          /* Tunnel */
    fd_set readfds;             /* Descriptors to watch for reading */
    int max;                    /* Highest fd */
    struct timeval tv, *ptv;    /* Timeout for select */
    struct msghdr msgh;
    struct iovec iov;
    char cbuf[256];
    unsigned int refme, refhim;
    int * currentfd;
    int server_socket_processed;

#ifdef HIGH_PRIO
    /* set high priority */
    if (setpriority(PRIO_PROCESS, 0, -20) < 0)
	l2tp_log (LOG_INFO, "xl2tpd: can't set priority to high: %m");
#endif

    /* This one buffer can be recycled for everything except control packets */
    buf = new_buf (MAX_RECV_SIZE);

    tunnel = 0;
    call = 0;

    for (;;)
    {
        int ret;
        process_signal();
        max = build_fdset (&readfds);
        ptv = process_schedule(&tv);
        ret = select (max + 1, &readfds, NULL, NULL, ptv);

        if (ret <= 0)
        {
#ifdef DEBUG_MORE
            if (ret == 0)
            {
                if (gconfig.debug_network)
                {
                    l2tp_log (LOG_DEBUG, "%s: select timeout\n", __FUNCTION__);
                }
            }
            else
            {
                if (gconfig.debug_network)
                {
                    l2tp_log (LOG_DEBUG,
                        "%s: select returned error %d (%s)\n",
                        __FUNCTION__, errno, strerror (errno));
                }
            }
#endif
            continue;
        }

        if (FD_ISSET (control_fd, &readfds))
        {
            do_control ();
        }
        server_socket_processed = 0;
        currentfd = NULL;
        st = tunnels.head;
        while (st || !server_socket_processed) {
            if (st && (st->udp_fd == -1)) {
                st=st->next;
                continue;
            }
            if (st) {
                currentfd = &st->udp_fd;
            } else {
                currentfd = &server_socket;
                server_socket_processed = 1;
            }
            if (FD_ISSET (*currentfd, &readfds))
        {
            /*
             * Okay, now we're ready for reading and processing new data.
             */
            recycle_buf (buf);

            /* Reserve space for expanding payload packet headers */
            buf->start += PAYLOAD_BUF;
            buf->len -= PAYLOAD_BUF;

	    memset(&from, 0, sizeof(from));
	    memset(&to,   0, sizeof(to));
	    
	    fromlen = sizeof(from);
	    
	    memset(&msgh, 0, sizeof(struct msghdr));
	    iov.iov_base = buf->start;
	    iov.iov_len  = buf->len;
	    msgh.msg_control = cbuf;
	    msgh.msg_controllen = sizeof(cbuf);
	    msgh.msg_name = &from;
	    msgh.msg_namelen = fromlen;
	    msgh.msg_iov  = &iov;
	    msgh.msg_iovlen = 1;
	    msgh.msg_flags = 0;
	    
	    /* Receive one packet. */
	    recvsize = recvmsg(*currentfd, &msgh, 0);

            if (recvsize < MIN_PAYLOAD_HDR_LEN)
            {
                if (recvsize < 0)
                {
                    if (errno == ECONNREFUSED) {
                        close(*currentfd);
                    }
                    if ((errno == ECONNREFUSED) ||
                        (errno == EBADF)) {
                        *currentfd = -1;
                    }
                    if (errno != EAGAIN)
                        l2tp_log (LOG_WARNING,
                             "%s: recvfrom returned error %d (%s)\n",
                             __FUNCTION__, errno, strerror (errno));
                }
                else
                {
                    l2tp_log (LOG_WARNING, "%s: received too small a packet\n",
                         __FUNCTION__);
                }
                if (st) st=st->next;
		continue;
            }


	    refme=refhim=0;


		struct cmsghdr *cmsg;
		/* Process auxiliary received data in msgh */
		for (cmsg = CMSG_FIRSTHDR(&msgh);
			cmsg != NULL;
			cmsg = CMSG_NXTHDR(&msgh,cmsg)) {
			/* extract destination(our) addr */
			if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_PKTINFO) {
				struct in_pktinfo* pktInfo = ((struct in_pktinfo*)CMSG_DATA(cmsg));
				to = *pktInfo;
			}
			/* extract IPsec info out */
			else if (gconfig.ipsecsaref && cmsg->cmsg_level == IPPROTO_IP
			&& cmsg->cmsg_type == gconfig.sarefnum) {
				unsigned int *refp;
				
				refp = (unsigned int *)CMSG_DATA(cmsg);
				refme =refp[0];
				refhim=refp[1];
			}
		}

	    /*
	     * some logic could be added here to verify that we only
	     * get L2TP packets inside of IPsec, or to provide different
	     * classes of service to packets not inside of IPsec.
	     */
	    buf->len = recvsize;
	    fix_hdr (buf->start);
	    extract (buf->start, &tunnel, &call);

	    if (gconfig.debug_network)
	    {
		l2tp_log(LOG_DEBUG, "%s: recv packet from %s, size = %d, "
			 "tunnel = %d, call = %d ref=%u refhim=%u\n",
			 __FUNCTION__, inet_ntoa (from.sin_addr),
			 recvsize, tunnel, call, refme, refhim);
	    }

	    if (gconfig.packet_dump)
	    {
		do_packet_dump (buf);
	    }
			if (!(c = get_call (tunnel, call, from.sin_addr,
			       from.sin_port, refme, refhim)))
	    {
				if ((c = get_tunnel (tunnel, from.sin_addr.s_addr, from.sin_port)))
		{
		    /*
		     * It is theoretically possible that we could be sent
		     * a control message (say a StopCCN) on a call that we
		     * have already closed or some such nonsense.  To
		     * prevent this from closing the tunnel, if we get a
		     * call on a valid tunnel, but not with a valid CID,
		     * we'll just send a ZLB to ACK receiving the packet.
		     */
		    if (gconfig.debug_tunnel)
			l2tp_log (LOG_DEBUG,
				  "%s: no such call %d on tunnel %d. Sending special ZLB\n",
				  __FUNCTION__, call, tunnel);
		    if (handle_special (buf, c, call) == 0)
			/* get a new buffer */
			buf = new_buf (MAX_RECV_SIZE);
		}
#ifdef DEBUG_MORE
		else{
		    l2tp_log (LOG_DEBUG,
			      "%s: unable to find call or tunnel to handle packet.  call = %d, tunnel = %d Dumping.\n",
			      __FUNCTION__, call, tunnel);
		    }
#endif
	    }
	    else
	    {
		if (c->container) {
			c->container->my_addr = to;
		}

		buf->peer = from;
		/* Handle the packet */
		c->container->chal_us.vector = NULL;
		if (handle_packet (buf, c->container, c))
		{
		    if (gconfig.debug_tunnel)
			l2tp_log (LOG_DEBUG, "%s: bad packet\n", __FUNCTION__);
		}
		if (c->cnu)
		{
		    /* Send Zero Byte Packet */
		    control_zlb (buf, c->container, c);
		    c->cnu = 0;
		}
		}
	}
	if (st) st=st->next;
	}

	/*
	 * finished obvious sources, look for data from PPP connections.
	 */
	st = tunnels.head;
        while (st)
        {
            sc = st->call_head;
            while (sc)
            {
                if ((sc->fd >= 0) && FD_ISSET (sc->fd, &readfds))
                {
                    /* Got some payload to send */
                    int result;

                    while ((result = read_packet (sc)) > 0)
                    {
                        add_payload_hdr (sc->container, sc, sc->ppp_buf);
                        if (gconfig.packet_dump)
                        {
                            do_packet_dump (sc->ppp_buf);
                        }


                        sc->prx = sc->data_rec_seq_num;
                        if (sc->zlb_xmit)
                        {
                            deschedule (sc->zlb_xmit);
                            sc->zlb_xmit = NULL;
                        }
                        sc->tx_bytes += sc->ppp_buf->len;
                        sc->tx_pkts++;
                        udp_xmit (sc->ppp_buf, st);
                        recycle_payload (sc->ppp_buf, sc->container->peer);
                    }
                    if (result != 0)
                    {
                        l2tp_log (LOG_WARNING,
                             "%s: tossing read packet, error = %s (%d).  Closing call.\n",
                             __FUNCTION__, strerror (-result), -result);
                        strcpy (sc->errormsg, strerror (-result));
                        sc->needclose = -1;
                    }
                }
                sc = sc->next;
            }
            st = st->next;
        }
    }

}

#ifdef USE_KERNEL
int connect_pppol2tp(struct tunnel *t) {
        if (kernel_support) {
            int ufd = -1, fd2 = -1;
            int flags;
            struct sockaddr_pppol2tp sax;

            struct sockaddr_in server;

            memset(&server, 0, sizeof(struct sockaddr_in));
            server.sin_family = AF_INET;
            server.sin_addr.s_addr = gconfig.listenaddr;
            server.sin_port = htons (gconfig.port);
            if ((ufd = socket (PF_INET, SOCK_DGRAM, 0)) < 0)
            {
                l2tp_log (LOG_CRIT, "%s: Unable to allocate UDP socket. Terminating.\n",
                    __FUNCTION__);
                return -EINVAL;
            }

            flags=1;
            setsockopt(ufd, SOL_SOCKET, SO_REUSEADDR, &flags, sizeof(flags));
#ifdef SO_NO_CHECK
            setsockopt(ufd, SOL_SOCKET, SO_NO_CHECK, &flags, sizeof(flags));
#endif

            if (bind (ufd, (struct sockaddr *) &server, sizeof (server)))
            {
                l2tp_log (LOG_CRIT, "%s: Unable to bind UDP socket: %s. Terminating.\n",
                     __FUNCTION__, strerror(errno), errno);
                close (ufd);
                return -EINVAL;
            }
            server = t->peer;
            flags = fcntl(ufd, F_GETFL);
            if (flags == -1 || fcntl(ufd, F_SETFL, flags | O_NONBLOCK) == -1) {
                l2tp_log (LOG_WARNING, "%s: Unable to set UDP socket nonblock.\n",
                     __FUNCTION__);
                close (ufd);
                return -EINVAL;
            }
            if (connect (ufd, (struct sockaddr *) &server, sizeof(server)) < 0) {
                l2tp_log (LOG_CRIT, "%s: Unable to connect UDP peer. Terminating.\n",
                 __FUNCTION__);
                close(ufd);
                return -EINVAL;
            }

            t->udp_fd=ufd;

            fd2 = socket(AF_PPPOX, SOCK_DGRAM, PX_PROTO_OL2TP);
            if (fd2 < 0) {
                l2tp_log (LOG_WARNING, "%s: Unable to allocate PPPoL2TP socket.\n",
                     __FUNCTION__);
                return -EINVAL;
            }
            flags = fcntl(fd2, F_GETFL);
            if (flags == -1 || fcntl(fd2, F_SETFL, flags | O_NONBLOCK) == -1) {
                l2tp_log (LOG_WARNING, "%s: Unable to set PPPoL2TP socket nonblock.\n",
                     __FUNCTION__);
                close(fd2);
                return -EINVAL;
            }
            memset(&sax, 0, sizeof(sax));
            sax.sa_family = AF_PPPOX;
            sax.sa_protocol = PX_PROTO_OL2TP;
            sax.pppol2tp.fd = t->udp_fd;
            sax.pppol2tp.addr.sin_addr.s_addr = t->peer.sin_addr.s_addr;
            sax.pppol2tp.addr.sin_port = t->peer.sin_port;
            sax.pppol2tp.addr.sin_family = AF_INET;
            sax.pppol2tp.s_tunnel  = t->ourtid;
            sax.pppol2tp.d_tunnel  = t->tid;
            if ((connect(fd2, (struct sockaddr *)&sax, sizeof(sax))) < 0) {
                l2tp_log (LOG_WARNING, "%s: Unable to connect PPPoL2TP socket. %d %s\n",
                     __FUNCTION__, errno, strerror(errno));
                close(fd2);
                return -EINVAL;
            }
            t->pppox_fd = fd2;
        }

    return 0;
}
#endif

