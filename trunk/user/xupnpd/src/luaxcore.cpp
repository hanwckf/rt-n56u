/*
 * Copyright (C) 2011-2015 Anton Burdinuk
 * clark15b@gmail.com
 * https://tsdemuxer.googlecode.com/svn/trunk/xupnpd
 */

#include "luaxcore.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include "luacompat.h"
#include "mem.h"
#include <time.h>
#include "mcast.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <ctype.h>
#include "compat.h"

#ifndef WITHOUT_OPENSSL
#include "openssl/ssl.h"
#endif /* WITHOUT_OPENSSL */

// HTML5
// ACE Stream (http://torrent-tv.ru/)
// HLS - http://en.wikipedia.org/wiki/HTTP_Live_Streaming
// TODO: https://github.com/blahlt/nStreamLmod/blob/master/start.xml (nStream)
// TODO: http://www.twitch.tv
// TODO: uuidgen to UI
// TODO: profile by IP
// TODO: RTSP, RTMP
// TODO: sendurl - reopen after pause?
// TODO: SQLite support
// TODO: TS padding for WDTV?
// TODO: m3u tree by group-title (grp/subgrp1/subgrp2 => reload_playlists)
// http://rg3.github.io/youtube-dl/
// Remux ts - http://superuser.com/questions/343716/ffmpeg-how-to-demux-live-multi-program-transport-stream, https://trac.ffmpeg.org/ticket/995

namespace core
{
    int http_timeout=15;
    int http_sendurl_buf_size=16384;
    int http_sendurl_wait_all=0;

    time_t start_time=0;

    char user_agent[256]="xupnpd";

#ifndef WITHOUT_OPENSSL
    SSL_CTX *ssl_ctx=NULL;
#endif /* WITHOUT_OPENSSL */

    struct url_data
    {
        char host[128];
        char vhost[128];
        int port;
        char urn[1024];
        char auth[256];
        int ssl;
    };

    struct timer_event
    {
        time_t tv;
        const char* name;
        int sec;
        timer_event* next;
    };


    timer_event* timers=0;

    struct listener
    {
        int port;
        int fd;
        const char* name;
        listener* next;
    };

    listener* listeners=0,*listeners_end=0;

    int detached=0;             // daemon
    FILE* http_client_fp=0;     // for HTTP workers only

    int connect(const char* s,int port);
    FILE* sock2file(int socket);

#ifndef WITHOUT_OPENSSL
    BIO* sock2bio(int socket, SSL* ssl);
#endif /* WITHOUT_OPENSSL */

    mcast::mcast_grp ssdp_mcast_grp;
    int ssdp_upstream=-1;
    int ssdp_downstream=-1;

    int listener_add(const char* host,int port,const char* name,int backlog)
    {
        if(!name || !*name || port<1)
            return -1;

        if(!host || !*host)
        {
            if(*ssdp_mcast_grp.interface)
                host=ssdp_mcast_grp.interface;
            else
                host="*";
        }

        if(backlog<1)
            backlog=5;

        sockaddr_in sin;
        sin.sin_family=AF_INET;
        sin.sin_addr.s_addr=INADDR_ANY;
        sin.sin_port=htons(port);

        if(strcmp(host,"*") && strcmp(host,"any"))
            sin.sin_addr.s_addr=inet_addr(host);

        int fd=socket(sin.sin_family,SOCK_STREAM,0);
        if(fd==-1)
            return -2;

        int reuse=1;
        setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));

        if(bind(fd,(sockaddr*)&sin,sizeof(sin)) || listen(fd,backlog))
        {
            close(fd);
            return -3;
        }

        fcntl(fd,F_SETFL,fcntl(fd,F_GETFL,0)|O_NONBLOCK);

        int n=strlen(name);

        listener* l=(listener*)MALLOC(sizeof(listener)+n+1);

        l->port=port;
        l->fd=fd;
        l->name=(char*)(l+1);
        l->next=0;
        strcpy((char*)l->name,name);

        if(!listeners)
            listeners=listeners_end=l;
        else
        {
            listeners_end->next=l;
            listeners_end=l;
        }

        return 0;
    }

    void listener_clear(void)
    {
        while(listeners)
        {
            listener* tmp=listeners;
            listeners=listeners->next;

            if(tmp->fd!=-1)
                close(tmp->fd);
            FREE(tmp);
        }

        listeners=listeners_end=0;
    }

    volatile int __sig_quit=0;
    volatile int __sig_alarm=0;
    volatile int __sig_child=0;
    volatile int __sig_usr1=0;
    volatile int __sig_usr2=0;

    int __sig_pipe[2]={-1,-1};
    int __event_pipe[2]={-1,-1};

    char* parse_command_line(const char* cmd,char** dst,int n);

    void sig_handler(int n)
    {
        int e=errno;

        switch(n)
        {
        case SIGINT:
        case SIGQUIT:
        case SIGTERM:
            __sig_quit=1;
            break;
        case SIGALRM:
            __sig_alarm=1;
            break;
        case SIGCHLD:
            __sig_child=1;
            break;
        case SIGUSR1:
            __sig_usr1=1;
            break;
        case SIGUSR2:
            __sig_usr2=1;
            break;
        }

        send(__sig_pipe[1],"*",1,MSG_DONTWAIT);

        errno=e;
    }

    void timer_add(int sec,const char* name)
    {
        if(!name || !*name)
            return;

        timer_event* e=(timer_event*)MALLOC(sizeof(timer_event)+strlen(name)+1);
        e->next=0;
        e->tv=time(0)+sec;
        e->name=(char*)(e+1);
        strcpy((char*)e->name,name);
        e->sec=sec;
        if(!timers || e->tv<=timers->tv)
        {
            e->next=timers;
            timers=e;
        }else
        {
            for(timer_event* tmp=timers;tmp;tmp=tmp->next)
            {
                if(!tmp->next)
                {
                    tmp->next=e;
                    break;
                }else
                {
                    if(tmp->next->tv>=e->tv)
                    {
                        e->next=tmp->next;
                        tmp->next=e;
                        break;
                    }
                }
            }
        }
    }

    void timer_reset(void)
    {
        int sec=0;

        if(timers)
        {
            sec=timers->tv-time(0);
            if(sec<1)
                sec=1;
        }

        alarm(sec);
    }

    void timer_clear(void)
    {
        alarm(0);

        while(timers)
        {
            timer_event* tmp=timers;
            timers=timers->next;
            FREE(tmp);
        }
        timers=0;
    }

    void ssdp_done(void)
    {
        if(ssdp_upstream!=-1)
        {
            ssdp_mcast_grp.close(ssdp_upstream);
            ssdp_upstream=-1;
        }

        if(core::ssdp_downstream!=-1)
        {
            ssdp_mcast_grp.leave(ssdp_downstream);
            ssdp_downstream=-1;
        }
    }

    void ssdp_clear(void)
    {
        if(ssdp_upstream!=-1)
        {
            ssdp_mcast_grp.close(ssdp_upstream);
            ssdp_upstream=-1;
        }

        if(core::ssdp_downstream!=-1)
        {
            ssdp_mcast_grp.close(ssdp_downstream);
            ssdp_downstream=-1;
        }
    }

    pid_t fork_process(int detach)
    {
        pid_t pid=fork();

        if(!pid)
        {
            alarm(0);

            for(int i=0;i<sizeof(core::__sig_pipe)/sizeof(*core::__sig_pipe);i++)
                close(core::__sig_pipe[i]);

            close(core::__event_pipe[0]);

            core::ssdp_clear();
            core::listener_clear();

            if(detach)
            {
                int fd=open("/dev/null",O_RDWR);
                if(fd>=0)
                {
                    for(int i=0;i<3;i++)
                        dup2(fd,i);
                    close(fd);
                }else
                    for(int i=0;i<3;i++)
                        close(i);
            }
        }

        return pid;
    }

    void process_event(lua_State* L,const char* name,int arg1)
    {
        lua_getglobal(L,"events");

        lua_getfield(L,-1,name);

        if(lua_type(L,-1)==LUA_TFUNCTION)
        {
            lua_pushstring(L,name);
            lua_pushinteger(L,arg1);
            if(lua_pcall(L,2,0,0))
            {
                if(!detached)
                    fprintf(stderr,"%s\n",lua_tostring(L,-1));
                else
                    syslog(LOG_INFO,"%s",lua_tostring(L,-1));
                lua_pop(L,1);
            }
        }else
            lua_pop(L,1);

        lua_pop(L,1);
    }

    void process_signals(lua_State* L)
    {
//printf("%i\n",lua_gettop(L));

        char buf[128];
        while(recv(__sig_pipe[0],buf,sizeof(buf),MSG_DONTWAIT)>0);

        if(__sig_usr1)
            { __sig_usr1=0; process_event(L,"SIGUSR1",0); }
        if(__sig_usr2)
            { __sig_usr2=0; process_event(L,"SIGUSR2",0); }

        if(__sig_child)
        {
            __sig_child=0;

            pid_t pid;
            int status=0;

            lua_getglobal(L,"childs");

            while((pid=wait3(&status,WNOHANG,0))>0)
            {
                int del=0;

                lua_pushinteger(L,pid);
                lua_gettable(L,-2);

                if(lua_type(L,-1)==LUA_TTABLE)
                {
                    del=1;

                    lua_getfield(L,-1,"event");

                    const char* event=lua_tostring(L,-1);
                    if(event)
                    {
                        if(WIFEXITED(status))
                            status=WEXITSTATUS(status);
                        else
                            status=128;
                                                
                        process_event(L,event,status);
                    }
                    lua_pop(L,1);
                }else
                {
                    // not found, internal?
                }

                lua_pop(L,1);

                if(del)
                {
                    lua_pushinteger(L,pid);
                    lua_pushnil(L);
                    lua_rawset(L,-3);    
                }
            }

            lua_pop(L,1);
        }        

        if(__sig_alarm)
        {
            __sig_alarm=0;

            time_t t=time(0);

            while(core::timers && core::timers->tv<=t)
            {
                core::timer_event* tmp=core::timers;

                process_event(L,core::timers->name,core::timers->sec);

                core::timers=core::timers->next;

                FREE(tmp);
            }

            core::timer_reset();
        }

//printf("%i\n",lua_gettop(L));
    }

    void process_events(lua_State* L)
    {
//printf("%i\n",lua_gettop(L));

        lua_getglobal(L,"events");

        unsigned char buf[1024];

        int n;
        while((n=recv(__event_pipe[0],buf,sizeof(buf),MSG_DONTWAIT))>0)
        {
            int num=0;

            for(unsigned char* p=buf;p<buf+sizeof(buf)-1;)
            {
                u_int16_t len=0;
                memcpy((char*)&len,p,sizeof(len));
                p+=sizeof(len);

                if(!len || len>sizeof(buf)-(p-buf))
                    break;

//printf("* '%s'\n",p);
                if(!num)
                    lua_getfield(L,-1,(char*)p);
                else
                    lua_pushlstring(L,(char*)p,len-1);

                p+=len;
                num++;
            }

            if(num>0)
            {
                if(lua_type(L,-num)==LUA_TFUNCTION)
                {
                    if(lua_pcall(L,num-1,0,0))
                    {
                        if(!detached)
                            fprintf(stderr,"%s\n",lua_tostring(L,-1));
                        else
                            syslog(LOG_INFO,"%s",lua_tostring(L,-1));
                        lua_pop(L,1);
                    }
                }else
                    lua_pop(L,num);
            }
        }

        lua_pop(L,1);

//printf("%i\n",lua_gettop(L));
    }


    void add_http_hdr_to_table(lua_State* L,char* p1,int idx)
    {
        if(!idx)
        {
            lua_pushstring(L,"reqline");
            lua_newtable(L);

            int nn=1;
            for(char* pp1=p1,*pp2;pp1;pp1=pp2)
            {
                pp2=strchr(pp1,' ');
                if(pp2)
                {
                    *pp2=0;
                    pp2++;
                    while(*pp2 && *pp2==' ')
                        pp2++;
                }
                if(*pp1)
                {
                    if(nn==2)
                    {
                        while(*pp1 && pp1[1]=='/')
                            pp1++;
                    }
                    lua_pushinteger(L,nn++);
                    lua_pushstring(L,pp1);
                    lua_rawset(L,-3);
                }                                
            }
            lua_rawset(L,-3);
        }else
        {
            char* p3=strchr(p1,':');
            if(p3)
            {
                *p3=0;
                p3++;
                while(*p3 && *p3==' ')
                    p3++;

                if(*p3=='\"')
                {
                    p3++;
                    char* p=strchr(p3,'\"');
                    if(p)
                        *p=0;
                }

                if(*p1 && *p3)
                {
                    for(char* pp=p1;*pp;pp++)
                        *pp=tolower(*pp);

                    lua_pushstring(L,p1);
                    lua_pushstring(L,p3);
                    lua_rawset(L,-3);
                }
            }
        }
    }

    void process_ssdp(lua_State* L)
    {
//printf("%i\n",lua_gettop(L));
        char buf[4096];
        int nbuf=0;

        char from[64]="";

        lua_getglobal(L,"events");
        
        while((nbuf=core::ssdp_mcast_grp.recv(ssdp_downstream,buf,sizeof(buf)-1,from,MSG_DONTWAIT))>0)
        {
            buf[nbuf]=0;

            static const char ssdp_tag[]="SSDP";

            lua_getfield(L,-1,ssdp_tag);

            if(lua_type(L,-1)==LUA_TFUNCTION)
            {
                lua_pushstring(L,ssdp_tag);
                lua_pushstring(L,from);

                lua_newtable(L);

                int idx=0;

                for(char* p1=buf,*p2;p1;p1=p2)
                {
                    while(*p1 && (*p1==' ' || *p1=='\r' || *p1=='\n' || *p1=='\t'))
                        p1++;

                    p2=strpbrk(p1,"\r\n");
                    if(p2)
                        { *p2=0; p2++; }

                    if(*p1)
                        add_http_hdr_to_table(L,p1,idx++);

                }

                if(lua_pcall(L,3,0,0))
                {
                    if(!detached)
                        fprintf(stderr,"%s\n",lua_tostring(L,-1));
                    else
                        syslog(LOG_INFO,"%s",lua_tostring(L,-1));
                    lua_pop(L,1);
                }
            }else
                lua_pop(L,1);


        }

        lua_pop(L,1);

//printf("%i\n",lua_gettop(L));

    }

    void process_http(lua_State* L,listener* l)
    {
        int fd;
        sockaddr_in sin;
        socklen_t sin_len=sizeof(sin);

        while((fd=accept(l->fd,(sockaddr*)&sin,&sin_len))>=0)
        {
            fcntl(fd,F_SETFL,fcntl(fd,F_GETFL,0)&(~O_NONBLOCK));

            char name[64];
            int n=snprintf(name,sizeof(name),"%s",l->name);
            if(n<0 || n>=sizeof(name))
                name[sizeof(name)-1]=0;
            int port=l->port;

            char from[64]="";
            sprintf(from,"%s:%i",inet_ntoa(sin.sin_addr),ntohs(sin.sin_port));

            pid_t pid=fork_process(0);

            if(!pid)
            {
                signal(SIGHUP,SIG_IGN);
                signal(SIGPIPE,SIG_DFL);
                signal(SIGINT,SIG_DFL);
                signal(SIGQUIT,SIG_DFL);
                signal(SIGTERM,SIG_DFL);
                signal(SIGALRM,SIG_DFL);
                signal(SIGUSR1,SIG_DFL);
                signal(SIGUSR2,SIG_DFL);
                signal(SIGCHLD,SIG_DFL);

                sigset_t full_sig_set;
                sigfillset(&full_sig_set);
                sigprocmask(SIG_UNBLOCK,&full_sig_set,0);

                int on=1;
                setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,&on,sizeof(on));

                FILE* fp=fdopen(fd,"a+");
                if(fp)
                {
//printf("%i\n",lua_gettop(L));
                    http_client_fp=fp;

                    lua_getglobal(L,"events");

                    lua_getfield(L,-1,name);

                    if(lua_type(L,-1)==LUA_TFUNCTION)
                    {
                        lua_pushstring(L,name);
                        lua_pushstring(L,from);
                        lua_pushinteger(L,port);

                        lua_newtable(L);

                        {
                            char tmp[1024];

                            int idx=0;

                            alarm(core::http_timeout);

                            while(fgets(tmp,sizeof(tmp),fp))
                            {
                                char* p=strpbrk(tmp,"\r\n");
                                if(p)
                                    *p=0;
                                if(!*tmp)
                                    break;
                                add_http_hdr_to_table(L,tmp,idx++);
                            }

                            lua_getfield(L,-1,"content-length");
                            if(lua_type(L,-1)!=LUA_TNIL)
                            {
                                int len=lua_tointeger(L,-1);
                                lua_pop(L,1);

                                if(len>0)
                                {
                                    luaL_Buffer B;
                                    luaL_buffinit(L,&B);
                                
                                    int n=0;
                                    while((n=fread(tmp,1,sizeof(tmp)>len?len:sizeof(tmp),fp))>0 && len>0)
                                    {
                                        luaL_addlstring(&B,tmp,n);
                                        len-=n;
                                    }

                                    luaL_pushresult(&B);
                                    lua_setfield(L,-2,"data");
                                }

                            }else
                                lua_pop(L,1);

                            alarm(0);                               // reset read timer

                            fseek(fp,0,SEEK_END);                   // Solaris fix
                        }

                        if(lua_pcall(L,4,0,0))
                        {
                            if(!detached)
                                fprintf(stderr,"%s\n",lua_tostring(L,-1));
                            else
                                syslog(LOG_INFO,"%s",lua_tostring(L,-1));
                            lua_pop(L,1);
                        }

                    }else
                        lua_pop(L,1);

                    lua_pop(L,1);

                    fclose(fp);
                }else
                    close(fd);

//printf("%i\n",lua_gettop(L));

                exit(0);
            }else if(pid!=(pid_t)-1)
            {
                lua_getglobal(L,"childs");
                lua_pushinteger(L,pid);
                lua_newtable(L);
                lua_rawset(L,-3);
                lua_pop(L,1);
            }

            close(fd);
        }
    }

    int base64enc(unsigned char* src,int nsrc,unsigned char* dst,int ndst);

}

int core::base64enc(unsigned char* src,int nsrc,unsigned char* dst,int ndst)
{
    int x=nsrc*4;
    int y=x%3;
    int nndst=x/3+(y?4-y:0);

    if(ndst<=nndst)
	return -1;

    static const char tbl[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    x=nsrc/3;
    for(unsigned long i=0;i<x;i++)
    {
        dst[0]=tbl[(src[0]>>2)&0x3f];
	dst[1]=tbl[(src[1]>>4&0x0f)|(src[0]<<4&0x30)];
	dst[2]=tbl[(src[1]<<2&0x3c)|(src[2]>>6&0x03)];
	dst[3]=tbl[src[2]&0x3f];
	src+=3;
	dst+=4;
    }

    nsrc-=x*3;
    if(nsrc==1)
    {
	dst[0]=tbl[(src[0]>>2)&0x3f];
	dst[1]=tbl[(src[0]<<4&0x30)];
	dst[2]='=';
	dst[3]='=';
	dst+=4;
    }else if(nsrc==2)
    {
        dst[0]=tbl[(src[0]>>2)&0x3f];
	dst[1]=tbl[(src[1]>>4&0x0f)|(src[0]<<4&0x30)];
	dst[2]=tbl[(src[1]<<2&0x3c)];
	dst[3]='=';
	dst+=4;

    }
    *dst=0;

    return 0;
}

static int lua_core_openlog(lua_State* L)
{
    const char* s=lua_tostring(L,1);
    const char* p=lua_tostring(L,2);

    if(!s)
        s="???";
    if(!p)
        p="";
    
    int f=LOG_SYSLOG;
    if(!strncmp(p,"local",5))
    {
        if(p[5]>47 && p[5]<56 && !p[6])
            f=((16+(p[5]-48))<<3);
    }else if(!strcmp(p,"daemon"))
        f=LOG_DAEMON;

    openlog(s,LOG_PID,f);
                                                    
    return 0;
}

static int lua_core_log(lua_State* L)
{
    char ss[512];
    int n=0;

    int count=lua_gettop(L);

    lua_getglobal(L,"tostring");

    for(int i=1;i<=count;i++)
    {
        lua_pushvalue(L,-1);
        lua_pushvalue(L,i);
        lua_call(L,1,1);

        size_t l=0;
        const char* s=lua_tolstring(L,-1,&l);
        if(s)
        {
            int m=sizeof(ss)-n;
            if(l>=m)
                l=m-1;

            memcpy(ss+n,s,l);
            n+=l;

            if(i<count)
            {
                if(n<sizeof(ss)-1)
                    ss[n++]=' ';
            }

        }
        lua_pop(L,1);
    }

    ss[n]=0;

    if(core::detached)
        syslog(LOG_INFO,"%s",ss);
    else
        fprintf(stdout,"%s\n",ss);

    return 0;
}

static int lua_core_detach(lua_State* L)
{
    pid_t pid=fork();
    if(pid==-1)
        return luaL_error(L,"can't fork process");
    else if(pid>0)
        exit(0);

    int fd=open("/dev/null",O_RDWR);
    if(fd>=0)
    {
        for(int i=0;i<3;i++)
            dup2(fd,i);
        close(fd);
    }else
        for(int i=0;i<3;i++)
            close(i);

    core::detached=1;
    lua_register(L,"print",lua_core_log);
    
    return 0;
}



static int lua_core_touchpid(lua_State* L)
{
    const char* s=lua_tostring(L,1);
    if(!s)
        return 0;

    FILE* fp=fopen(s,"r");
    if(fp)
        return luaL_error(L,"pid file already is exist");

    fp=fopen(s,"w");
    if(fp)
    {
        fprintf(fp,"%i",getpid());
        fclose(fp);
    }

    return 0;
}

// free(rc)!
char* core::parse_command_line(const char* cmd,char** dst,int n)
{
    char* s=strdup(cmd);

    int st=0;
    char* tok=0;
    int i=0;

    for(char* p=s;*p && i<n-1;p++)
    {
        switch(st)
        {
        case 0:
            if(*p==' ') continue;
            tok=p;
            if(*p=='\'' || *p=='\"') st=1; else st=2;
            break;
        case 1:
            if(*p=='\'' || *p=='\"')
                { *p=0; st=0; dst[i++]=tok+1; tok=0; }
            break;
        case 2:
            if(*p==' ')
                { *p=0; st=0; dst[i++]=tok; tok=0; }
            break;
        }
    }
    if(tok && i<n-1)
        dst[i++]=tok;

    dst[i]=0;

    return s;
}

static int lua_core_spawn(lua_State* L)
{
    const char* cmd=lua_tostring(L,1);
    const char* event=lua_tostring(L,2);

    int rc=0;

    if(!cmd || !event)
    {
        lua_pushinteger(L,rc);
        return 1;
    }

    pid_t pid=core::fork_process(1);

    if(pid!=-1)
    {
        if(!pid)
        {
            char* argv[128];
            core::parse_command_line(cmd,argv,sizeof(argv)/sizeof(*argv));
            if(argv[0])
                execvp(argv[0],argv);
            exit(127);
        }else
        {
            rc=1;

            lua_getglobal(L,"childs");

            lua_pushinteger(L,pid);
            lua_newtable(L);

            lua_pushstring(L,"cmd");
            lua_pushstring(L,cmd);
            lua_rawset(L,-3);

            lua_pushstring(L,"event");
            lua_pushstring(L,event);
            lua_rawset(L,-3);

            lua_rawset(L,-3);

            lua_pop(L,1);
        }
    }

    lua_pushinteger(L,rc);

    return 1;
}

static int lua_core_fspawn(lua_State* L)
{
    if(lua_type(L,1)!=LUA_TFUNCTION)
        return 0;

    int args=lua_gettop(L)-1;

    pid_t pid=core::fork_process(0);
    if(!pid)
    {
        signal(SIGHUP,SIG_IGN);
        signal(SIGPIPE,SIG_DFL);
        signal(SIGINT,SIG_DFL);
        signal(SIGQUIT,SIG_DFL);
        signal(SIGTERM,SIG_DFL);
        signal(SIGALRM,SIG_DFL);
        signal(SIGUSR1,SIG_DFL);
        signal(SIGUSR2,SIG_DFL);
        signal(SIGCHLD,SIG_DFL);

        sigset_t full_sig_set;
        sigfillset(&full_sig_set);
        sigprocmask(SIG_UNBLOCK,&full_sig_set,0);

        if(lua_pcall(L,args,0,0))
        {
            if(!core::detached)
                fprintf(stderr,"%s\n",lua_tostring(L,-1));
            else
                syslog(LOG_INFO,"%s",lua_tostring(L,-1));
            lua_pop(L,1);
        }
        exit(0);
    }

    return 0;
}

static int lua_core_timer(lua_State* L)
{
    int sec=lua_tointeger(L,1);
    const char* event=lua_tostring(L,2);

    int rc=0;

    if(!sec || !event)
    {
        lua_pushinteger(L,rc);
        return 1;
    }

    core::timer_add(sec,event);

    core::timer_reset();

    lua_pushinteger(L,rc);

    return 1;
}

static int lua_core_uuid(lua_State* L)
{
    char buf[64];
    mcast::uuid_gen(buf);

    lua_pushstring(L,buf);

    return 1;
}

static int lua_core_sendevent(lua_State* L)
{
    unsigned char buf[1024],*p=buf;

    int num=lua_gettop(L);

    u_int16_t ll;

    for(int i=1;i<=num;i++)
    {
        size_t l=0;
        const char* s=lua_tolstring(L,i,&l);

        if(!s)
            s="";

        l+=1;

        ll=l+sizeof(ll)*2;

        if(ll>sizeof(buf)-(p-buf))
            break;

        ll=l;
        memcpy(p,(char*)&ll,sizeof(ll));
        p+=sizeof(ll);
        memcpy((char*)p,s,l);
        p+=l;
    }

    ll=0;
    memcpy(p,(char*)&ll,sizeof(ll));
    p+=sizeof(ll);

    send(core::__event_pipe[1],buf,p-buf,0);

    return 0;
}


static int lua_core_mainloop(lua_State* L)
{
    using namespace core;

    start_time=time(0);

    if(__sig_quit)
        return 0;

    setsid();

    if(socketpair(PF_LOCAL,SOCK_STREAM,0,__sig_pipe))
        return luaL_error(L,"socketpair fail, can't create signal pipe");
    if(socketpair(PF_LOCAL,SOCK_DGRAM,0,__event_pipe))
        return luaL_error(L,"socketpair fail, can't create event pipe");


    struct sigaction action;
    sigset_t full_sig_set;

    memset((char*)&action,0,sizeof(action));
    sigfillset(&action.sa_mask);
    action.sa_handler=sig_handler;
    sigfillset(&full_sig_set);

    signal(SIGHUP,SIG_IGN);
    signal(SIGPIPE,SIG_IGN);
    sigaction(SIGINT,&action,0);
    sigaction(SIGQUIT,&action,0);
    sigaction(SIGTERM,&action,0);
    sigaction(SIGALRM,&action,0);
    sigaction(SIGUSR1,&action,0);
    sigaction(SIGUSR2,&action,0);
    sigaction(SIGCHLD,&action,0);

    sigprocmask(SIG_BLOCK,&full_sig_set,0);

    while(!__sig_quit)
    {
        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(__sig_pipe[0],&fdset);
        FD_SET(__event_pipe[0],&fdset);

        int nfd=__sig_pipe[0]>__event_pipe[0]?__sig_pipe[0]:__event_pipe[0];

        if(ssdp_downstream!=-1)
        {
            FD_SET(ssdp_downstream,&fdset);
            nfd=nfd<ssdp_downstream?ssdp_downstream:nfd;
        }

        for(listener* ll=listeners;ll;ll=ll->next)
        {
            FD_SET(ll->fd,&fdset);
            nfd=nfd<ll->fd?ll->fd:nfd;
        }

        nfd++;

        sigprocmask(SIG_UNBLOCK,&full_sig_set,0);

        int rc=select(nfd,&fdset,0,0,0);

        sigprocmask(SIG_BLOCK,&full_sig_set,0);

        if(rc==-1)
        {
            if(errno==EINTR)
            {
                process_signals(L);
                continue;
            }else
                break;
        }else if(!rc)
            continue;

        if(FD_ISSET(__sig_pipe[0],&fdset))
            process_signals(L);

        if(FD_ISSET(__event_pipe[0],&fdset))
            process_events(L);

        if(ssdp_downstream!=-1 && FD_ISSET(ssdp_downstream,&fdset))
            process_ssdp(L);

        for(listener* ll=listeners;ll;ll=ll->next)
            if(FD_ISSET(ll->fd,&fdset))
                process_http(L,ll);
    }

    sigprocmask(SIG_UNBLOCK,&full_sig_set,0);

    lua_getglobal(L,"atexit");
    lua_pushnil(L);
    while(lua_next(L,-2))
    {
        if(lua_type(L,-1)==LUA_TFUNCTION)
        {
            if(lua_pcall(L,0,0,0))
            {
                if(!detached)
                    fprintf(stderr,"%s\n",lua_tostring(L,-1));
                else
                    syslog(LOG_INFO,"%s",lua_tostring(L,-1));
                lua_pop(L,1);
            }
        }else
            lua_pop(L,1);
    }
    lua_pop(L,1);

    ssdp_done();
    listener_clear();

#ifndef WITHOUT_OPENSSL
    if(ssl_ctx!=NULL)
        SSL_CTX_free(ssl_ctx);
#endif /* WITHOUT_OPENSSL */

    signal(SIGTERM,SIG_IGN);
    signal(SIGCHLD,SIG_IGN);
    kill(0,SIGTERM);

    for(int i=0;i<sizeof(__sig_pipe)/sizeof(*__sig_pipe);i++)
        close(__sig_pipe[i]);
    for(int i=0;i<sizeof(__event_pipe)/sizeof(*__event_pipe);i++)
        close(__event_pipe[i]);

    timer_clear();

    return 0;
}

static int lua_ssdp_init(lua_State* L)
{
    const char* iface=lua_tostring(L,1);
    int ttl=lua_tointeger(L,2);
    int loop=lua_tointeger(L,3);
    int debug=lua_tointeger(L,4);

    if(!iface)
        iface="eth0";

    if(ttl<1)
        ttl=1;

    if(!core::detached && debug>0)
    {
        mcast::verb_fp=stderr;

        if(debug>1)
            mcast::debug=1;
    }

    core::ssdp_done();

    int rc=0;

    if(!core::ssdp_mcast_grp.init("239.255.255.250:1900",iface,ttl,loop))
    {
        core::ssdp_upstream=core::ssdp_mcast_grp.upstream();
        if(core::ssdp_upstream!=-1)
        {
            core::ssdp_downstream=core::ssdp_mcast_grp.join();
            if(core::ssdp_downstream!=-1)
                rc=1;
            else
                core::ssdp_done();
        }
    }

    lua_pushinteger(L,rc);    

    return 1;
}

static int lua_ssdp_send(lua_State* L)
{
    size_t l=0;
    const char* s=lua_tolstring(L,1,&l);

    const char* to=lua_tostring(L,2);

    if(core::ssdp_upstream==-1 || !s || l<1)
        return 0;

    core::ssdp_mcast_grp.send(core::ssdp_upstream,s,l,to);

    return 0;
}

static int lua_ssdp_interface(lua_State* L)
{
    if(core::ssdp_downstream!=-1)
    {
        lua_pushstring(L,core::ssdp_mcast_grp.interface);
        return 1;
    }
    return 0;
}

static int lua_http_listen(lua_State* L)
{
    int port=lua_tointeger(L,1);
    const char* name=lua_tostring(L,2);
    const char* host=lua_tostring(L,3);
    int backlog=lua_tointeger(L,4);

    int rc=core::listener_add(host,port,name,backlog);
    
    lua_pushinteger(L,rc?0:1);

    return 1;
}

static int lua_http_send(lua_State* L)
{
    size_t l=0;
    const char* s=lua_tolstring(L,1,&l);

    if(core::http_client_fp)
        fwrite(s,1,l,core::http_client_fp);

    return 0;
}

static int lua_http_sendfile(lua_State* L)
{
    const char* s=lua_tostring(L,1);

    if(!s || !core::http_client_fp)
        return 0;

    fflush(core::http_client_fp);

    int fd=open(s,O_RDONLY|O_LARGEFILE);
    if(fd==-1)
        return 0;

    if(lua_gettop(L)>1 && lua_type(L,2)!=LUA_TNIL)
    {
        if(lseek64(fd,(off64_t)lua_tonumber(L,2),SEEK_SET)==(off64_t)-1)
        {
            close(fd);
            return 0;
        }
    }

    char buf[1024];
    ssize_t n;

    if(lua_gettop(L)>2 && lua_type(L,3)!=LUA_TNIL)
    {
        off64_t l=(off64_t)lua_tonumber(L,3);
        if(l>0)
        {
            while(l>0 && (n=read(fd,buf,sizeof(buf)>l?l:sizeof(buf)))>0)
                if(write(fileno(core::http_client_fp),buf,n)!=n)
                    break;
                else
                    l-=n;
        }
    }else
    {
        while((n=read(fd,buf,sizeof(buf)))>0)
            if(write(fileno(core::http_client_fp),buf,n)!=n)
                break;
    }

    close(fd);

    return 0;
}

static int lua_tmpl_process(lua_State* L,FILE* sfp,FILE* dfp)
{
    if(!sfp || !dfp)
        return 0;

    int st=0;

    char var[64]="";
    int nvar=0;

    for(;;)
    {
        int ch=fgetc(sfp);

        if(ch==EOF)
            break;

        switch(st)
        {
        case 0:
            if(ch=='$')
                st=1;
            else
                fputc(ch,dfp);
            break;
        case 1:
            if(ch=='{')
                st=2;
            else
            {
                fputc('$',dfp);
                if(ch!='$')
                {
                    fputc(ch,dfp);
                    st=0;
                }
            }
            break;
        case 2:
            if(ch=='}')
            {
                var[nvar]=0;

                lua_getfield(L,-1,var);

                if(lua_type(L,-1)==LUA_TFUNCTION)
                {
                    if(lua_pcall(L,0,1,0))
                    {
                        if(!core::detached)
                            fprintf(stderr,"%s\n",lua_tostring(L,-1));
                        else
                            syslog(LOG_INFO,"%s",lua_tostring(L,-1));
                    }                                                                                                                            
                }

                const char* p=lua_tostring(L,-1);
                if(!p)
                    p="";

                fprintf(dfp,"%s",p);

                lua_pop(L,1);

                nvar=0;
                st=0;
            }else
            {
                if(nvar<sizeof(var)-1)
                    var[nvar++]=ch;
            }
            break;
        }
    }

    return 0;
}


static int lua_http_sendtfile(lua_State* L)
{
    const char* s=lua_tostring(L,1);

    if(!s || !core::http_client_fp)
        return 0;

    FILE* fp=fopen(s,"r");
    if(!fp)
        return 0;

    int rc=lua_tmpl_process(L,fp,core::http_client_fp);

    fclose(fp);

    return rc;
}

static int lua_http_compile_template(lua_State* L)
{
    const char* s=lua_tostring(L,1);
    const char* d=lua_tostring(L,2);

    if(!s || !d)
        return 0;

    FILE* sfp=fopen(s,"r");
    if(!sfp)
        return 0;

    FILE* dfp=fopen(d,"w");
    if(!dfp)
    {
        fclose(sfp);
        return 0;
    }

    int rc=lua_tmpl_process(L,sfp,dfp);

    fclose(sfp);
    fclose(dfp);

    return rc;
}

int core::connect(const char* s,int port)
{
    sockaddr_in sin;
    sin.sin_family=AF_INET;
    sin.sin_port=htons(port);
    sin.sin_addr.s_addr=inet_addr(s);
    if(sin.sin_addr.s_addr==INADDR_NONE)
    {
        hostent* he=gethostbyname(s);
        if(he)
            memcpy((char*)&sin.sin_addr.s_addr,he->h_addr,sizeof(sin.sin_addr.s_addr));
    }

    if(sin.sin_addr.s_addr==INADDR_NONE)
        return 0;

    int fd=socket(sin.sin_family,SOCK_STREAM,0);
    if(fd==-1)
        return 0;

    if(connect(fd,(sockaddr*)&sin,sizeof(sin)))
    {
        close(fd);
        return 0;
    }

    int on=1;
    setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,&on,sizeof(on));
    return fd;
}

FILE* core::sock2file(int socket) {
	if (socket==0) return NULL;
    FILE* fp=fdopen(socket,"a+");
    if(!fp)
    {
        close(socket);
        return NULL;
    }
    return fp;
}

#ifndef WITHOUT_OPENSSL
BIO* core::sock2bio(int socket,SSL* ssl)
{
    if(socket==0 || ssl==NULL)
        return NULL;

    BIO* sbio=BIO_new_socket(socket,BIO_NOCLOSE);

    SSL_set_bio(ssl,sbio,sbio);

    //SSL_set_fd(ssl,socket);

    int err=SSL_connect(ssl);

    BIO* buf_io=BIO_new(BIO_f_buffer());        /* create a buffer BIO */

    BIO* ssl_bio=BIO_new(BIO_f_ssl());          /* create an ssl BIO */

    BIO_set_ssl(ssl_bio,ssl,BIO_CLOSE);         /* assign the ssl BIO to SSL */

    BIO_push(buf_io,ssl_bio);                   /* add ssl_bio to buf_io */ 

    //fbio=BIO_new(BIO_f_buffer());
    //BIO_push(fbio,sbio);

    return buf_io;
}
#endif /* WITHOUT_OPENSSL */

static int lua_http_get_url_data(const char* url,core::url_data* d)
{
    char tmp[2048];

    *d->host=0;
    *d->vhost=0;
    d->port=0;
    *d->urn=0;
    *d->auth=0;
    d->ssl=0;

    int n=snprintf(tmp,sizeof(tmp),"%s",url);
    if(n<0 || n>=sizeof(tmp))
        return -1;

    char* host=tmp;
    int port=0;

    static const char proto_tag[]="://";

    char* p=strstr(host,proto_tag);
    if(p)
    {
        *p=0;

        if(!strcasecmp(host,"https")) {
            port=443;
            d->ssl=1;
        }
        else if(!strcasecmp(host,"http"))
            port=80;
        else
            return -1;

        host=p+sizeof(proto_tag)-1;

    }

    char* urn=strchr(host,'/');

    if(urn)
        { *urn=0; urn++; }
    else
        urn=(char*)"";

    char* pp=strchr(host,'@');
    if(pp)
    {
        *pp=0;

        if(core::base64enc((unsigned char*)host,strlen(host),(unsigned char*)d->auth,sizeof(d->auth)))
            return -1;

        host=pp+1;
    }
    n=snprintf(d->vhost,sizeof(d->vhost),"%s",host);
    if(n<0 || n>=sizeof(d->vhost))
        return -1;

    p=strchr(host,':');

    if(p)
        { *p=0; p++; port=atoi(p); }

    if(!port)
        return -1;

    d->port=port;

    n=snprintf(d->urn,sizeof(d->urn),"%s%s",*urn=='/'?"":"/",urn);
    if(n<0 || n>=sizeof(d->urn))
        return -1;

    n=snprintf(d->host,sizeof(d->host),"%s",host);
    if(n<0 || n>=sizeof(d->host))
        return -1;

    return 0;
}

static size_t lua_http_read_chunk(char* ptr,size_t size,FILE* fp
#ifndef WITHOUT_OPENSSL
    ,BIO* bio
#endif /* WITHOUT_OPENSSL */
                )
{
    size_t l=0;

    while(l<size)
    {
        size_t n=0;
        if (fp)
            n=fread(ptr+l,1,size-l,fp);
#ifndef WITHOUT_OPENSSL
        else if (bio)
            n=BIO_read(bio, ptr+l, size-l);
#endif /* WITHOUT_OPENSSL */

        if(!n)
            break;
        else
            l+=n;

        if(!core::http_sendurl_wait_all)
            break;
    }

    return l;
}

static int lua_http_sendurl(lua_State* L)
{
    const char* s=lua_tostring(L,1);

    int extra_headers=lua_gettop(L)>1?lua_tointeger(L,2):0;

    const char* range=lua_gettop(L)>2?lua_tostring(L,3):0;

    int rc=0;

    char location[1024]="";

    if(!s || !core::http_client_fp)
    {
        lua_pushinteger(L,rc);
        return 1;
    }

    core::url_data url;

    if(lua_http_get_url_data(s,&url))
    {
        lua_pushinteger(L,rc);
        return 1;
    }

    alarm(core::http_timeout);

    int sock=core::connect(url.host,url.port);

    FILE* fp=NULL;

#ifndef WITHOUT_OPENSSL
    SSL* ssl=NULL;

    BIO* bio=NULL;

    if(url.ssl)
    {
        ssl=SSL_new(core::ssl_ctx);
        bio=core::sock2bio(sock,ssl);
    }else
#endif /* WITHOUT_OPENSSL */
        fp=core::sock2file(sock);

    if(!fp
#ifndef WITHOUT_OPENSSL
        && !bio
#endif /* WITHOUT_OPENSSL */
                )
    {
#ifndef WITHOUT_OPENSSL
        if(ssl)
            SSL_free(ssl);
#endif /* WITHOUT_OPENSSL */

        alarm(0);

        lua_pushinteger(L,rc);

        return 1;
    }

#ifndef WITHOUT_OPENSSL
    if (url.ssl)
    {
        BIO_printf (bio,"GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\nConnection: close\r\nCache-Control: no-cache\r\n",url.urn,url.vhost,core::user_agent);
        if(range && *range)
            BIO_printf(bio,"Range: %s\r\n",range);

        if(*url.auth)
            BIO_printf(bio,"Authorization: Basic %s\r\n",url.auth);

        BIO_printf(bio,"\r\n");
        BIO_flush(bio);
    }else
#endif /* WITHOUT_OPENSSL */
    {

        fprintf(fp,"GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\nConnection: close\r\nCache-Control: no-cache\r\n",url.urn,url.vhost,core::user_agent);
        if(range && *range)
            fprintf(fp,"Range: %s\r\n",range);

        if(*url.auth)
            fprintf(fp,"Authorization: Basic %s\r\n",url.auth);

        fprintf(fp,"\r\n");
        fflush(fp);
    }

    int idx=0;

    int status=0;

    char* tmp=(char*)malloc(core::http_sendurl_buf_size);

    while(1)
    {
#ifndef WITHOUT_OPENSSL
        if(url.ssl)
        {
            if(0>=BIO_gets(bio,tmp,core::http_sendurl_buf_size))
                break;
        }else
#endif /* WITHOUT_OPENSSL */
        {
            if(!fgets(tmp,core::http_sendurl_buf_size,fp))
                break;
        }

        char* p=strpbrk(tmp,"\r\n");

        if(p)
            *p=0;

        if(!*tmp)
            break;

        if(!idx)
        {
            p=strchr(tmp,' ');
            if(p)
            {
                *p=0;
                p++;

                if(!strcmp(tmp,"HTTP/1.1") || !strcmp(tmp,"HTTP/1.0"))
                {
                    char* p2=strchr(p,' ');
                    if(p2)
                    {
                        *p2=0;
                        status=atoi(p);
                    }
                }
            }
        }else
        {
            static const char location_tag[]="Location:";
            if(!strncasecmp(tmp,location_tag,sizeof(location_tag)-1))
            {
                char* pp=tmp+sizeof(location_tag)-1;
                while(*pp==' ')
                    pp++;

                int n=snprintf(location,sizeof(location),"%s",pp);
                if(n==-1 || n>=sizeof(location))
                    location[sizeof(location)-1]=0;
            }else if(extra_headers>0 && (status==200 || status==206))
            {
                static const char content_length_tag[]="Content-Length:";
                static const char content_range_tag[]="Content-Range:";
                if(!strncasecmp(tmp,content_length_tag,sizeof(content_length_tag)-1) || !strncasecmp(tmp,content_range_tag,sizeof(content_range_tag)-1))
                {
                    fprintf(core::http_client_fp,"%s\r\n",tmp);
//printf("<- %s\n",tmp);
                }
            }
        }

        idx++;
    }

    if(status!=200 && status!=206)
    {
        if(fp)
            fclose(fp);

#ifndef WITHOUT_OPENSSL
        if(bio)
        {
            SSL_set_shutdown(ssl, SSL_SENT_SHUTDOWN|SSL_RECEIVED_SHUTDOWN);

            BIO_free_all(bio);
        }
#endif /* WITHOUT_OPENSSL */

        alarm(0);

        lua_pushinteger(L,rc);

        if(*location)
            lua_pushstring(L,location);
        else
            lua_pushnil(L);

        if(tmp)
            free(tmp);

        return 2;
    }else
        rc=1;

    size_t n;

    if(extra_headers>0)
        fprintf(core::http_client_fp,"\r\n");

    fflush(core::http_client_fp);

    int dfd=fileno(core::http_client_fp);

    while((n=lua_http_read_chunk(tmp,core::http_sendurl_buf_size,fp
#ifndef WITHOUT_OPENSSL
        ,bio
#endif /* WITHOUT_OPENSSL */
            ))>0)
    {
        size_t ll=0;

        while(ll<n)
        {
            ssize_t nn=write(dfd,tmp+ll,n-ll);
            if(!nn || nn==(ssize_t)-1)
                break;
            else
                ll+=nn;
        }
        alarm(core::http_timeout);
    }

    alarm(0);

    if(fp)
        fclose(fp);

#ifndef WITHOUT_OPENSSL
    if(bio)
    {
        SSL_set_shutdown(ssl,SSL_SENT_SHUTDOWN|SSL_RECEIVED_SHUTDOWN);

        BIO_free_all(bio);
    }
#endif /* WITHOUT_OPENSSL */

    lua_pushinteger(L,rc);

    if(tmp)
        free(tmp);

    return 1;
}

static int lua_http_sendmcasturl(lua_State* L)
{
    const char* addr=lua_tostring(L,1);
    const char* iface=lua_gettop(L)>1?lua_tostring(L,2):0;
    int dgsize=lua_gettop(L)>2?lua_tointeger(L,3):0;

    if(dgsize<=0)
        dgsize=4096;

    int rc=0;

    if(!addr || !core::http_client_fp)
        { lua_pushinteger(L,rc); return 1; }

    alarm(core::http_timeout);

    mcast::mcast_grp grp;
    grp.init(addr,iface,1,1);

    int fd=grp.join();

    if(fd!=-1)
    {
        char* buf=(char*)malloc(dgsize);

        if(buf)
        {
            fflush(core::http_client_fp);

            int pnum=0;

            int dfd=fileno(core::http_client_fp);

            int n;

            sockaddr_in sin;
            socklen_t sin_len=sizeof(sin);

            rc=1;

            while((n=recvfrom(fd,buf,dgsize,0,(sockaddr*)&sin,&sin_len))>0)
            {
                // TODO: extract payload from RTP

                if(mcast::verb_fp && !pnum)
                {
                    fprintf(mcast::verb_fp,"multicast source: %s:%i\n",inet_ntoa(sin.sin_addr),ntohs(sin.sin_port));
                    pnum++;
                }

                if(write(dfd,buf,n)!=n)
                    break;
                else
                    alarm(core::http_timeout);
            }

            free(buf);
        }

        grp.leave(fd);
    }

    alarm(0);

    lua_pushinteger(L,rc);

    return 1;
}


static int lua_http_flush(lua_State* L)
{
    if(core::http_client_fp)
        fflush(core::http_client_fp);

    return 0;
}


static int lua_http_notify(lua_State* L)
{
    const char* s=lua_tostring(L,1);

    const char* sid=lua_tostring(L,2);

    size_t len=0;
    const char* data=lua_tolstring(L,3,&len);

    int seq=lua_tointeger(L,4);
    if(seq<0)
        seq=0;

    if(!s || !sid || !data)
        return 0;

    core::url_data url;
    if(lua_http_get_url_data(s,&url))
        return 0;

    alarm(core::http_timeout);

    FILE* fp=core::sock2file(core::connect(url.host,url.port));
    if(!fp)
    {
        alarm(0);
        return 0;
    }

    fprintf(fp,
        "NOTIFY %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\nConnection: close\r\nContent-Type: text/xml\r\nContent-Length: %lu\r\n"
        "NT: upnp:event\r\nNTS: upnp:propchange\r\nSID: uuid:%s\r\nSEQ: %i\r\nCache-Control: no-cache\r\n\r\n",url.urn,url.vhost,core::user_agent,(unsigned long)len,sid,seq);
    fwrite(data,len,1,fp);
    fflush(fp);

    int n;
    char tmp[256];
    while((n=fread(tmp,1,sizeof(tmp),fp))>0);

    alarm(0);

    fclose(fp);

    return 0;
}

static int lua_http_download(lua_State* L)
{
    const char* s=lua_tostring(L,1);
    const char* d=lua_gettop(L)>1?lua_tostring(L,2):0;

    size_t post_data_size=0;
    const char* post_data=lua_gettop(L)>2?lua_tolstring(L,3,&post_data_size):0;

    if(!post_data)
        post_data="";

    int len=0;
    char location[1024]="";

    FILE* dfp=0;

    luaL_Buffer B;

    if(d)
    {
        dfp=fopen(d,"wb");
        if(!dfp)
        {
            lua_pushinteger(L,len);
            return 1;
        }
    }else
        luaL_buffinit(L,&B);

    if(s)
    {
        core::url_data url;

        if(!lua_http_get_url_data(s,&url))
        {
            alarm(core::http_timeout);

            int sock=core::connect(url.host,url.port);

            FILE* fp=NULL;

#ifndef WITHOUT_OPENSSL
            SSL* ssl=NULL;

            BIO* bio=NULL;

            if (url.ssl)
            {
                ssl=SSL_new(core::ssl_ctx);

                bio=core::sock2bio(sock,ssl);
            }else
#endif /* WITHOUT_OPENSSL */
                fp=core::sock2file(sock);

            if(fp
#ifndef WITHOUT_OPENSSL
                || bio
#endif /* WITHOUT_OPENSSL */
                        )
            {
#ifndef WITHOUT_OPENSSL
                if(url.ssl)
                {
                    BIO_printf(bio,
                        "%s %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\nConnection: close\r\nCache-Control: no-cache\r\n",*post_data?"POST":"GET",
                            url.urn,url.vhost,core::user_agent);
//    printf("lua_http_download bioprintf:\r\n%s %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\nConnection: close\r\nCache-Control: no-cache\r\n",*post_data?"POST":"GET",
                            //url.urn,url.vhost,core::user_agent);

                    if(*post_data)
                        BIO_printf(bio,"Content-Length: %lu\r\n",(unsigned long)post_data_size);

    //                fprintf(fp,"Accept-Charset: utf-8\r\n");

                    BIO_printf(bio,"\r\n");

                    if(*post_data)
                        BIO_write(bio,post_data,post_data_size);

                    BIO_flush(bio);
                }else
#endif /* WITHOUT_OPENSSL */
                {
                    fprintf(fp,
                        "%s %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\nConnection: close\r\nCache-Control: no-cache\r\n",*post_data?"POST":"GET",
                            url.urn,url.vhost,core::user_agent);

                    if(*post_data)
                        fprintf(fp,"Content-Length: %lu\r\n",(unsigned long)post_data_size);

    //                fprintf(fp,"Accept-Charset: utf-8\r\n");

                    fprintf(fp,"\r\n");

                    if(*post_data)
                        fwrite(post_data,post_data_size,1,fp);

                    fflush(fp);
                }

                int status=0;

                int content_length=-1;

                int n;
                char tmp[1024];
                int idx=0;

                while(1)
                {
#ifndef WITHOUT_OPENSSL
                    if(url.ssl)
                    {
                        if(0>=BIO_gets(bio,tmp,sizeof(tmp)))
                            break;
                    }else
#endif /* WITHOUT_OPENSSL */
                    {
                        if(!fgets(tmp,sizeof(tmp),fp))
                            break;
                    }

                    char* p=strpbrk(tmp,"\r\n");

                    if(p)
                        *p=0;

                    if(!*tmp)
                        break;

                    if(!idx)
                    {
                        char* pp=strchr(tmp,' ');

                        if(pp)
                        {
                            while(*pp && *pp==' ')
                                pp++;
                            char* pp2=strchr(pp,' ');
                            if(pp2)
                                *pp2=0;

                            status=atoi(pp);
                        }
                    }else
                    {
                        char* pp=strchr(tmp,':');
                        if(pp)
                        {
                            *pp=0;
                            pp++;
                            while(*pp && *pp==' ')
                                pp++;

                            if(!strcasecmp(tmp,"Content-Length"))
                                content_length=atoi(pp);
                            else if(!strcasecmp(tmp,"Location"))
                            {
                                int n=snprintf(location,sizeof(location),"%s",pp);
                                if(n==-1 || n>=sizeof(location))
                                    location[sizeof(location)-1]=0;
                            }
                        }
                    }
                    idx++;
                }

                while(1)
                {
#ifndef WITHOUT_OPENSSL
                    if(url.ssl)
                        n=BIO_read(bio,tmp,sizeof(tmp));
                    else
#endif /* WITHOUT_OPENSSL */
                        n=fread(tmp,1,sizeof(tmp),fp);

                    if(n<=0)
                        break;

                    if(dfp)
                    {
                        if(fwrite(tmp,1,n,dfp)!=n)
                            break;
                    }else
                        luaL_addlstring(&B,tmp,n);

                    len+=n;

                    alarm(core::http_timeout);
                }

                if(status!=200 || (content_length!=-1 && content_length!=len))
                    len=0;

                if(fp)
                    fclose(fp);

#ifndef WITHOUT_OPENSSL
                if(bio)
                {
                    SSL_set_shutdown(ssl, SSL_SENT_SHUTDOWN|SSL_RECEIVED_SHUTDOWN);

                    BIO_free_all(bio);
                }
#endif /* WITHOUT_OPENSSL */
            }

            alarm(0);
        }
    }


    if(dfp)
    {
        lua_pushinteger(L,len);

        fclose(dfp);

        if(!len)
            unlink(d);

    }else
    {
        luaL_pushresult(&B);

        if(!len)
        {
            lua_pop(L,1);
            lua_pushnil(L);
        }
    }

    if(*location)
        lua_pushstring(L,location);
    else
        lua_pushnil(L);

    return 2;
}

static int lua_http_get_length(lua_State* L)
{
    const char* s=lua_tostring(L,1);

    int len=0;

    char location[1024]="";

    if(s)
    {
        core::url_data url;

        if(!lua_http_get_url_data(s,&url))
        {
            alarm(core::http_timeout);

            int sock=core::connect(url.host,url.port);

            FILE* fp=NULL;

#ifndef WITHOUT_OPENSSL
            SSL* ssl=NULL;

            BIO* bio=NULL;

            if(url.ssl)
            {
                ssl=SSL_new(core::ssl_ctx);

                bio=core::sock2bio(sock,ssl);
            }else
#endif /* WITHOUT_OPENSSL */
                fp=core::sock2file(sock);

            if(fp
#ifndef WITHOUT_OPENSSL
                || bio
#endif /* WITHOUT_OPENSSL */
                        )
            {
#ifndef WITHOUT_OPENSSL
                if(url.ssl)
                {
                    BIO_printf(bio,
                        "HEAD %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\nConnection: close\r\nCache-Control: no-cache\r\n\r\n",
                            url.urn,url.vhost,core::user_agent);

                    BIO_flush(bio);
                }else
#endif /* WITHOUT_OPENSSL */
                {
                    fprintf(fp,
                        "HEAD %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\nConnection: close\r\nCache-Control: no-cache\r\n\r\n",
                            url.urn,url.vhost,core::user_agent);

                    fflush(fp);
                }

                int status=0;

                int content_length=-1;

                int n;

                char tmp[1024];

                int idx=0;

                while(1)
                {
#ifndef WITHOUT_OPENSSL
                    if(url.ssl)
                    {
                        if(0>=BIO_gets(bio,tmp,sizeof(tmp)))
                            break;
                    }else
#endif /* WITHOUT_OPENSSL */
                    {
                        if(!fgets(tmp,sizeof(tmp),fp))
                            break;
                    }

                    char* p=strpbrk(tmp,"\r\n");

                    if(p)
                        *p=0;

                    if(!*tmp)
                        break;

                    if(!idx)
                    {
                        char* pp=strchr(tmp,' ');
                        if(pp)
                        {
                            while(*pp && *pp==' ')
                                pp++;
                            char* pp2=strchr(pp,' ');
                            if(pp2)
                                *pp2=0;

                            status=atoi(pp);
                        }
                    }else
                    {
                        char* pp=strchr(tmp,':');

                        if(pp)
                        {
                            *pp=0;
                            pp++;

                            while(*pp && *pp==' ')
                                pp++;

                            if(!strcasecmp(tmp,"Content-Length"))
                                content_length=atoi(pp);
                            else if(!strcasecmp(tmp,"Location"))
                            {
                                int n=snprintf(location,sizeof(location),"%s",pp);
                                if(n==-1 || n>=sizeof(location))
                                    location[sizeof(location)-1]=0;
                            }
                        }
                    }
                    idx++;
                }

                if(content_length>0)
                    len=content_length;

                if(fp)
                    fclose(fp);

#ifndef WITHOUT_OPENSSL
                if(bio)
                {
                    SSL_set_shutdown(ssl, SSL_SENT_SHUTDOWN|SSL_RECEIVED_SHUTDOWN);
                    BIO_free_all(bio);
                }
#endif /* WITHOUT_OPENSSL */
            }

            alarm(0);
        }
    }

    lua_pushinteger(L,len);

    if(*location)
        lua_pushstring(L,location);
    else
        lua_pushnil(L);

    return 2;
}


static int lua_http_timeout(lua_State* L)
{
    int t=lua_tointeger(L,1);

    if(t<1)
        t=15;

    core::http_timeout=t;

    return 0;
}

static int lua_http_sendurl_buffer_size(lua_State* L)
{
    int size=lua_tointeger(L,1);

    int all=lua_tointeger(L,2);

    if(size>0)
        core::http_sendurl_buf_size=size;

    if(all>=0)
        core::http_sendurl_wait_all=all>0?1:0;

    return 0;
}

static int lua_http_user_agent(lua_State* L)
{
    const char* s=lua_tostring(L,1);

    if(s && *s)
    {
        int n=snprintf(core::user_agent,sizeof(core::user_agent),"%s",s);
        if(n==-1 || n>=sizeof(core::user_agent))
            core::user_agent[sizeof(core::user_agent)-1]=0;
    }

    return 0;
}


static int lua_core_readpidfile(const char* path)
{
    FILE* fp=fopen(path,"r");
    if(!fp)
        return -1;

    int pid=0;
    int rc=fscanf(fp,"%i",&pid);

    fclose(fp);

    if(rc>0 && pid>0)
        return pid;

    return -1;
}

static int lua_core_restart(lua_State* L)
{
    const char* pidfile=lua_tostring(L,1);
    const char* cmd=lua_tostring(L,2);

    if(pidfile && *pidfile && cmd && *cmd)
    {
        int main_pid=lua_core_readpidfile(pidfile);

        if(main_pid>0)
        {
            pid_t pid=fork();

            if(!pid)
            {
                close(fileno(core::http_client_fp));

                signal(SIGHUP,SIG_IGN);
                signal(SIGPIPE,SIG_IGN);
                signal(SIGINT,SIG_IGN);
                signal(SIGQUIT,SIG_IGN);
                signal(SIGTERM,SIG_IGN);
                signal(SIGALRM,SIG_IGN);
                signal(SIGUSR1,SIG_IGN);
                signal(SIGUSR2,SIG_IGN);
                signal(SIGCHLD,SIG_IGN);

                for(int i=0;i<3;i++)
                    close(i);

                sleep(1);

                if(!kill(main_pid,SIGTERM))
                {
                    for(int i=0;i<5;i++)
                    {
                        main_pid=lua_core_readpidfile(pidfile);
                        if(main_pid<0)
                            break;

                        sleep(1);
                    }

                    if(main_pid<0)
                        int rc=system(cmd);

                    exit(0);
                }
            }else if(pid>0)
            {
                lua_pushboolean(L,1);

                return 1;
            }
        }
    }

    lua_pushboolean(L,0);

    return 1;
}

static int lua_core_uptime(lua_State* L)
{
    time_t uptime=time(0)-core::start_time;

    int days=uptime/86400;
    uptime%=86400;

    int hours=uptime/3600;
    uptime%=3600;

    int minutes=uptime/60;
    uptime%=60;

    char buf[256];
    int n=snprintf(buf,sizeof(buf),"%i days, %i:%.2i:%.2i",days,hours,minutes,(int)uptime);
    if(n==-1 || n>=sizeof(buf))
        n=sizeof(buf)-1;

    lua_pushlstring(L,buf,n);

    return 1;
}

int luaopen_luaxcore(lua_State* L)
{
    mcast::uuid_init();

#ifndef WITHOUT_OPENSSL
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
    core::ssl_ctx=SSL_CTX_new(SSLv23_client_method());
#else
    core::ssl_ctx=SSL_CTX_new(TLS_client_method());
#endif /* OPENSSL_VERSION_NUMBER */
    SSL_CTX_set_verify(core::ssl_ctx, SSL_VERIFY_NONE, NULL);
#endif /* WITHOUT_OPENSSL */

    static const luaL_Reg lib_core[]=
    {
        {"detach",lua_core_detach},
        {"openlog",lua_core_openlog},
        {"log",lua_core_log},
        {"touchpid",lua_core_touchpid},
        {"spawn",lua_core_spawn},
        {"fspawn",lua_core_fspawn},
        {"timer",lua_core_timer},
        {"uuid",lua_core_uuid},
        {"sendevent",lua_core_sendevent},
        {"mainloop",lua_core_mainloop},
        {"restart",lua_core_restart},
        {"uptime",lua_core_uptime},
        {0,0}
    };

    static const luaL_Reg lib_ssdp[]=
    {
        {"init",lua_ssdp_init},
        {"send",lua_ssdp_send},
        {"interface",lua_ssdp_interface},
        {0,0}
    };

    static const luaL_Reg lib_http[]=
    {
        {"listen",lua_http_listen},
        {"send",lua_http_send},
        {"sendfile",lua_http_sendfile},
        {"sendtfile",lua_http_sendtfile},
        {"compile_template",lua_http_compile_template},
        {"sendurl",lua_http_sendurl},
        {"sendmcasturl",lua_http_sendmcasturl},
        {"flush",lua_http_flush},
        {"notify",lua_http_notify},
        {"download",lua_http_download},
        {"get_length",lua_http_get_length},
        {"timeout",lua_http_timeout},
        {"sendurl_buffer_size",lua_http_sendurl_buffer_size},
        {"user_agent",lua_http_user_agent},
        {0,0}
    };

    luaL_register(L,"core",lib_core);
    luaL_register(L,"ssdp",lib_ssdp);
    luaL_register(L,"http",lib_http);

    lua_newtable(L);
    lua_setglobal(L,"events");

    lua_newtable(L);
    lua_setglobal(L,"childs");

    lua_newtable(L);
    lua_setglobal(L,"atexit");

    return 0;
}
