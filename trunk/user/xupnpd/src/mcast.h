/* 
 * Copyright (C) 2011-2015 Anton Burdinuk
 * clark15b@gmail.com
 * https://tsdemuxer.googlecode.com/svn/trunk/xupnpd
 */

#ifndef __MCAST_H
#define __MCAST_H

#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>

namespace mcast
{
    enum
    {
        IF_UP           = 0x0001,
        IF_BROADCAST    = 0x0002,
        IF_LOOPBACK     = 0x0004,
        IF_POINTOPOINT  = 0x0008,
        IF_RUNNING      = 0x0010,
        IF_MULTICAST    = 0x0020
    };

    enum
    {
        IF_NAME_LEN     = 64,
        IF_ADDR_LEN     = 64
    };

    struct if_info
    {
        char if_name[IF_NAME_LEN];
        char if_addr[IF_ADDR_LEN];
        sockaddr_in if_sin;
        u_int32_t if_flags;
    };

    extern FILE* verb_fp;
    extern int debug;

    void uuid_init(void);
    void uuid_gen(char* dst);

    int isalpha(int ch);

    int get_if_info(const char* if_name,if_info* ifi);
    int get_if_list(if_info* ifi,int nifi);
    in_addr get_best_mcast_if_addr(void);
    int get_socket_port(int s);
    int create_tcp_listener(int port);

    class mcast_grp
    {
    protected:
        sockaddr_in mcast_sin;
        sockaddr_in mcast_if_sin;
        int mcast_ttl;
        int mcast_loop;
    public:
        mcast_grp(void);
        mcast_grp(const char* addr,const char* iface,int ttl,int loop);

        char interface[64];

        int init(const char* addr,const char* iface,int ttl,int loop);

        int join(void) const;
        int leave(int sock) const;

        int upstream(void) const;

        int send(int sock,const char* buf,int len,const char* to) const;
        int recv(int sock,char* buf,int len,char* from,int flags=0) const;

        static void close(int sock);
    };

}

#endif
