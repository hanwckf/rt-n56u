/* 
 * Copyright (C) 2011-2015 Anton Burdinuk
 * clark15b@gmail.com
 * https://tsdemuxer.googlecode.com/svn/trunk/xupnpd
 */

#include "mcast.h"
#include <sys/socket.h>
#include <arpa/inet.h>

#ifdef WITH_LIBUUID
#ifdef __FreeBSD__
#include <uuid.h>
#else
#include <uuid/uuid.h>
#endif /* __FreeBSD__ */
#endif /* WITH_LIBUUID */

#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include "mem.h"
#include "compat.h"

namespace mcast
{
    FILE* verb_fp=0;
    int debug=0;
}

void mcast::uuid_init(void)
{
    srand(getpid());
}

void mcast::uuid_gen(char* dst)
{
#ifdef WITH_LIBUUID
#ifdef __FreeBSD__
    uuid_t uuid;
    char *p;

    uuid_create(&uuid, NULL);
    uuid_to_string(&uuid, &p, NULL);
    snprintf(dst, 48, "%s", p);
    free(p);
#else
    uuid_t uuid;
    uuid_generate(uuid);

    uuid_unparse_lower(uuid,dst);
#endif /* __FreeBSD__ */

#else /* WITH_LIBUUID */
    u_int16_t t[8];

    int n=0;

#ifdef WITH_URANDOM
    FILE* fp=fopen("/dev/urandom","rb");
    if(fp)
    {
        n=fread(t,sizeof(t),1,fp);
        fclose(fp);
    }
#endif /*WITH_URANDOM*/

    if(!n)
    {
        for(int i=0;i<sizeof(t)/sizeof(*t);i++)
            t[i]=rand()&0xffff;
    }

    sprintf(dst,"%.4x%.4x-%.4x-%.4x-%.4x-%.4x%.4x%.4x",t[0],t[1],t[2],t[3],t[4],t[5],t[6],t[7]);
#endif /*WITH_LIBUUID*/
}

int mcast::get_if_info(const char* if_name,if_info* ifi)
{
    *ifi->if_name=0;
    ifi->if_flags=0;
    *ifi->if_addr=0;
    memset((char*)&ifi->if_sin,0,sizeof(sockaddr_in));

    int s=socket(PF_INET,SOCK_DGRAM,0);

    if(s!=-1)
    {
        ifreq ifr;

#if defined(__FreeBSD__) || defined(__APPLE__)
        snprintf(ifr.ifr_name,IFNAMSIZ,"%s",if_name);
#else
        snprintf(ifr.ifr_ifrn.ifrn_name,IFNAMSIZ,"%s",if_name);
#endif /* __FreeBSD__ */

        snprintf(ifi->if_name,IF_NAME_LEN,"%s",if_name);

        if(ioctl(s,SIOCGIFADDR,&ifr)!=-1)
        {
            if(ifr.ifr_addr.sa_family==AF_INET)
            {
                ifi->if_sin.sin_family=ifr.ifr_addr.sa_family;
                ifi->if_sin.sin_addr=((sockaddr_in*)&ifr.ifr_addr)->sin_addr;
                ifi->if_sin.sin_port=0;
                snprintf(ifi->if_addr,IF_ADDR_LEN,"%s",inet_ntoa(ifi->if_sin.sin_addr));
            }
        }

        if(ioctl(s,SIOCGIFFLAGS,&ifr)!=-1)
        {
            if(ifr.ifr_flags&IFF_UP)            ifi->if_flags|=IF_UP;
            if(ifr.ifr_flags&IFF_BROADCAST)     ifi->if_flags|=IF_BROADCAST;
            if(ifr.ifr_flags&IFF_LOOPBACK)      ifi->if_flags|=IF_LOOPBACK;
            if(ifr.ifr_flags&IFF_POINTOPOINT)   ifi->if_flags|=IF_POINTOPOINT;
            if(ifr.ifr_flags&IFF_RUNNING)       ifi->if_flags|=IF_RUNNING;
            if(ifr.ifr_flags&IFF_MULTICAST)     ifi->if_flags|=IF_MULTICAST;
        }

        close(s);
    }

    return *ifi->if_addr?0:-1;
}

int mcast::get_if_list(if_info* ifi,int nifi)
{
    int s=socket(PF_INET,SOCK_DGRAM,0);

    int n=0;

    if(s!=-1)
    {
        int l=sizeof(ifreq)*nifi;

        ifreq* ifr=(ifreq*)MALLOC(l);

        if(ifi)
        {
            ifconf ifc;

            memset((char*)&ifc,0,sizeof(ifc));

            ifc.ifc_buf=(char*)ifr;
            ifc.ifc_len=l;

            if(ioctl(s,SIOCGIFCONF,&ifc)!=-1)
            {
                n=ifc.ifc_len/sizeof(ifreq);
                if(n>nifi)
                    n=nifi;

                for(int i=0;i<n;i++)
                {
#if defined(__FreeBSD__) || defined(__APPLE__)
                    get_if_info(ifr[i].ifr_name,ifi+i);
#else
                    get_if_info(ifr[i].ifr_ifrn.ifrn_name,ifi+i);
#endif /* __FreeBSD__ */
                }
            }

            FREE(ifr);
        }

        close(s);
    }

    return n;
}

in_addr mcast::get_best_mcast_if_addr(void)
{
    mcast::if_info ifi[16];

    for(int i=0;i<mcast::get_if_list(ifi,sizeof(ifi)/sizeof(*ifi));i++)
    {
        if(!(ifi[i].if_flags&IF_LOOPBACK) && ifi[i].if_flags&IF_UP && ifi[i].if_flags&IF_MULTICAST)
            return ifi[i].if_sin.sin_addr;
    }

    in_addr tt;
    tt.s_addr=INADDR_ANY;
    return tt;
}

int mcast::get_socket_port(int s)
{
    sockaddr_in sin;
    socklen_t sin_len=sizeof(sin);

    if(getsockname(s,(sockaddr*)&sin,&sin_len))
        return 0;

    return ntohs(sin.sin_port);
}


mcast::mcast_grp::mcast_grp(void)
{
    memset((char*)&mcast_sin,0,sizeof(mcast_sin));
    memset((char*)&mcast_if_sin,0,sizeof(mcast_if_sin));
    mcast_ttl=0;
    mcast_loop=0;
    *interface=0;
}

mcast::mcast_grp::mcast_grp(const char* addr,const char* iface,int ttl,int loop)
{
    init(addr,iface,ttl,loop);
}

int mcast::isalpha(int ch)
{
    if((ch>64 && ch<91) || (ch>96 && ch<123))
        return 1;
    return 0;
}

int mcast::mcast_grp::init(const char* addr,const char* iface,int ttl,int loop)
{
    if(!iface)
        iface="";

    mcast_ttl=ttl;
    mcast_loop=loop;

    mcast_sin.sin_family=AF_INET;
    mcast_sin.sin_port=0;
    mcast_sin.sin_addr.s_addr=INADDR_ANY;
#if defined(__FreeBSD__) || defined(__APPLE__)
    mcast_sin.sin_len=sizeof(mcast_sin);
#endif /* __FreeBSD__ */

    mcast_if_sin.sin_family=AF_INET;
    mcast_if_sin.sin_port=0;
    mcast_if_sin.sin_addr.s_addr=INADDR_ANY;
#if defined(__FreeBSD__) || defined(__APPLE__)
    mcast_if_sin.sin_len=sizeof(mcast_if_sin);
#endif /* __FreeBSD__ */

    *interface=0;

    char tmp[256];
    strcpy(tmp,addr);

    if(mcast_ttl<1)
        mcast_ttl=1;

    char* port=strchr(tmp,':');
    if(port)
    {
        *port=0;
        mcast_sin.sin_port=htons(atoi(port+1));
    }

    mcast_sin.sin_addr.s_addr=inet_addr(tmp);

    int if_by_name=0;
    for(const char* p=iface;*p;p++)
        if(isalpha(*p))
            { if_by_name=1; break; }

    if(*iface)
    {
        if(if_by_name)
        {
            if(verb_fp)
                fprintf(verb_fp,"find multicast interface address by name '%s'\n",iface);

            if_info ifi;
            get_if_info(iface,&ifi);
            memcpy((char*)&mcast_if_sin,&ifi.if_sin,sizeof(sockaddr_in));
        }else
            mcast_if_sin.sin_addr.s_addr=inet_addr(iface);
    }

    if(mcast_if_sin.sin_addr.s_addr==INADDR_ANY)
    {
        if(verb_fp)
            fprintf(verb_fp,"find multicast default interface address\n");
        mcast_if_sin.sin_addr=get_best_mcast_if_addr();
    }

    if(verb_fp)
    {
        fprintf(verb_fp,"multicast interface address: '%s'\n",mcast_if_sin.sin_addr.s_addr==INADDR_ANY?"any":inet_ntoa(mcast_if_sin.sin_addr));
        fprintf(verb_fp,"multicast group address: '%s:%i'\n",inet_ntoa(mcast_sin.sin_addr),ntohs(mcast_sin.sin_port));
    }

    snprintf(interface,sizeof(interface),"%s",inet_ntoa(mcast_if_sin.sin_addr));

    return 0;
}

int mcast::mcast_grp::join(void) const
{
    int sock=socket(PF_INET,SOCK_DGRAM,0);

    if(sock!=-1)
    {
        sockaddr_in sin;

        sin.sin_family=AF_INET;
        sin.sin_port=mcast_sin.sin_port;
        sin.sin_addr.s_addr=INADDR_ANY;
#if defined(__FreeBSD__) || defined(__APPLE__)
        sin.sin_len=sizeof(sin);
#endif /* __FreeBSD__ */

        int reuse=1;
        setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));

        if(!bind(sock,(sockaddr*)&sin,sizeof(sin)))
        {
            ip_mreq mcast_group;
            memset((char*)&mcast_group,0,sizeof(mcast_group));
            mcast_group.imr_multiaddr.s_addr=mcast_sin.sin_addr.s_addr;
            mcast_group.imr_interface.s_addr=mcast_if_sin.sin_addr.s_addr;

            if(!setsockopt(sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mcast_group,sizeof(mcast_group)))
            {
                if(verb_fp)
                {
                    fprintf(verb_fp,"join to multicast group '%s:%i' on ",inet_ntoa(mcast_group.imr_multiaddr),ntohs(sin.sin_port));
                    fprintf(verb_fp,"interface '%s'\n",mcast_group.imr_interface.s_addr==INADDR_ANY?"any":inet_ntoa(mcast_group.imr_interface));
                }

                return sock;
            }

            close(sock);
        }
        close(sock);
    }

    if(verb_fp)
    {
        fprintf(verb_fp,"can`t join to multicast group '%s:%i' on ",inet_ntoa(mcast_sin.sin_addr),ntohs(mcast_sin.sin_port));
        fprintf(verb_fp,"interface '%s'\n",mcast_if_sin.sin_addr.s_addr==INADDR_ANY?"any":inet_ntoa(mcast_if_sin.sin_addr));
    }

    return -1;
}

int mcast::mcast_grp::leave(int sock) const
{
    ip_mreq mcast_group;
    memset((char*)&mcast_group,0,sizeof(mcast_group));
    mcast_group.imr_multiaddr.s_addr=mcast_sin.sin_addr.s_addr;
    mcast_group.imr_interface.s_addr=mcast_if_sin.sin_addr.s_addr;

    if(!setsockopt(sock,IPPROTO_IP,IP_DROP_MEMBERSHIP,&mcast_group,sizeof(mcast_group)))
    {
        if(verb_fp)
        {
            fprintf(verb_fp,"leave multicast group '%s' on ",inet_ntoa(mcast_group.imr_multiaddr));
            fprintf(verb_fp,"interface '%s'\n",mcast_group.imr_interface.s_addr==INADDR_ANY?"any":inet_ntoa(mcast_group.imr_interface));
        }
    }else
    {
        if(verb_fp)
        {
            fprintf(verb_fp,"can`t leave multicast group '%s' on ",inet_ntoa(mcast_group.imr_multiaddr));
            fprintf(verb_fp,"interface '%s'\n",mcast_group.imr_interface.s_addr==INADDR_ANY?"any":inet_ntoa(mcast_group.imr_interface));
        }
    }

    close(sock);

    return 0;
}

int mcast::mcast_grp::upstream(void) const
{
    int sock=socket(PF_INET,SOCK_DGRAM,0);

    if(sock!=-1)
    {
        setsockopt(sock,IPPROTO_IP,IP_MULTICAST_TTL,&mcast_ttl,sizeof(mcast_ttl));
        setsockopt(sock,IPPROTO_IP,IP_MULTICAST_LOOP,&mcast_loop,sizeof(mcast_loop));
        setsockopt(sock,IPPROTO_IP,IP_MULTICAST_IF,&mcast_if_sin.sin_addr,sizeof(in_addr));

        sockaddr_in sin;
        sin.sin_family=AF_INET;
        sin.sin_addr.s_addr=mcast_if_sin.sin_addr.s_addr;
        sin.sin_port=0;
#if defined(__FreeBSD__) || defined(__APPLE__)
        sin.sin_len=sizeof(sin);
#endif /* __FreeBSD__ */
        bind(sock,(sockaddr*)&sin,sizeof(sin));

        if(verb_fp)
        {
            fprintf(verb_fp,"multicast upstream address: '%s:%i'\n",inet_ntoa(sin.sin_addr),get_socket_port(sock));
            fprintf(verb_fp,"multicast upstream ttl: %i\n",mcast_ttl);
        }

        return sock;
    }

    if(verb_fp)
        fprintf(verb_fp,"can`t create multicast upstream channel\n");

    return -1;
}

void mcast::mcast_grp::close(int sock)
{
    ::close(sock);
}

int mcast::mcast_grp::send(int sock,const char* buf,int len,const char* to) const
{
    if(to && !*to)
        to=0;

    sockaddr_in sin;

    if(to)
    {
        char tmp[64];
        strcpy(tmp,to);
        char* p=strchr(tmp,':');
        if(!p)
            return 0;
        *p=0;
        p++;

        sin.sin_family=AF_INET;
        sin.sin_addr.s_addr=inet_addr(tmp);
        sin.sin_port=htons(atoi(p));
#if defined(__FreeBSD__) || defined(__APPLE__)
        sin.sin_len=sizeof(sin);
#endif /* __FreeBSD__ */
    }

    int n=sendto(sock,buf,len,0,(sockaddr*)(to?&sin:&mcast_sin),sizeof(sockaddr_in));

    if(n>0 && verb_fp)
    {
        if(to)
            fprintf(verb_fp,"send %i bytes to '%s'\n",n,to);
        else
        {
            fprintf(verb_fp,"send %i bytes to multicast group '%s:%i' via ",n,inet_ntoa(mcast_sin.sin_addr),ntohs(mcast_sin.sin_port));
            fprintf(verb_fp,"interface '%s'\n",mcast_if_sin.sin_addr.s_addr==INADDR_ANY?"any":inet_ntoa(mcast_if_sin.sin_addr));
        }

        if(debug)
        {
            fwrite(buf,len,1,verb_fp);
            fprintf(verb_fp,"\n");
        }
    }

    return n;
}

int mcast::mcast_grp::recv(int sock,char* buf,int len,char* from,int flags) const
{
    sockaddr_in sin;
    socklen_t sin_len=sizeof(sin);

    int n=recvfrom(sock,buf,len,flags,(sockaddr*)&sin,&sin_len);

    if(n>0)
    {
        sprintf(from,"%s:%i",inet_ntoa(sin.sin_addr),ntohs(sin.sin_port));

        if(verb_fp)
        {
            fprintf(verb_fp,"recv %i bytes from '%s'\n",n,from);

            if(debug)
            {
                fwrite(buf,n,1,verb_fp);
                fprintf(verb_fp,"\n");
            }
        }

    }else
        *from=0;

    return n;
}

int mcast::create_tcp_listener(int port)
{
    int s=socket(PF_INET,SOCK_STREAM,0);

    if(s!=-1)
    {
        int reuse=1;
        setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));

        sockaddr_in sin;
        sin.sin_family=AF_INET;
        sin.sin_addr.s_addr=INADDR_ANY;
        sin.sin_port=htons(port);
#if defined(__FreeBSD__) || defined(__APPLE__)
        sin.sin_len=sizeof(sin);
#endif /* __FreeBSD__ */

        if(!bind(s,(sockaddr*)&sin,sizeof(sin)))
        {
            if(!listen(s,5))
                return s;
        }

        close(s);
    }

    return -1;
}
