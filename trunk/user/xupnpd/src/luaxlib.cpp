/* 
 * Copyright (C) 2011-2015 Anton Burdinuk
 * clark15b@gmail.com
 * https://tsdemuxer.googlecode.com/svn/trunk/xupnpd
 */

#include "luaxlib.h"
#include <string.h>
#include <stdio.h>
#include "soap.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include "luacompat.h"
#include "mem.h"
#include "md5.h"
#include "compat.h"

namespace util
{
    int get_file_ext(const char* path,char* dst,int len)
    {
        static const unsigned char t[256]=
        {
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
            0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
            0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
            0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
            0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
            0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
            0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
            0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
            0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
            0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
            0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
            0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
            0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
            0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
            0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
            0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
        };

        *dst=0;

        int n=strlen(path);
        if(!n)
            return 0;

        const char* p=path+n;

        while(p>path && p[-1]!='.' && p[-1]!='/' && p[-1]!='\\')
            p--;

        if(p==path || p[-1]!='.')
            return 0;

        n=strlen(p);
        if(n>=len)
            n=len-1;

        for(int i=0;i<n;i++)
            dst[i]=t[p[i]];

        dst[n]=0;

        return n;
    }

    char* url_decode(char* s)
    {
        static const unsigned char t[256]=
        {
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        };

        char* ptr=s;

        unsigned char* d=(unsigned char*)s;

        while(*s)
        {
            if(*s=='+')
                *d=' ';
            else if(*s=='%')
            {
                if(!s[1] || !s[2])
                    break;

                unsigned char c1=t[s[1]];
                unsigned char c2=t[s[2]];

                if(c1==0xff || c2==0xff)
                    break;
                *d=((c1<<4)&0xf0)|c2;
                s+=2;
            }else if((unsigned char*)s!=d)
                *d=*s;

            s++;
            d++;
        }
        *d=0;

        return ptr;
    }
}


static void lua_push_soap_node(lua_State* L,soap::node* node)
{
    lua_pushstring(L,node->name?node->name:"???");

    if(!node->beg)
        lua_pushlstring(L,node->data,node->len);
    else
    {
        lua_newtable(L);

        for(soap::node* p=node->beg;p;p=p->next)
            lua_push_soap_node(L,p);
    }

    lua_rawset(L,-3);
}


static int lua_soap_parse(lua_State* L)
{
    size_t l=0;

    const char* s=lua_tolstring(L,1,&l);
    if(!s)
        s="";

    lua_newtable(L);

    soap::node root;
    soap::ctx ctx(&root);

    ctx.begin();
    if(!ctx.parse(s,l) && !ctx.end() && root.beg)
        lua_push_soap_node(L,root.beg);

    root.clear();

    return 1;
}

static void lua_push_xml_node(lua_State* L,soap::node* node)
{
    if(node->name)
    {
        lua_pushstring(L,"@name");
        lua_pushstring(L,node->name);
        lua_rawset(L,-3);
    }

    if(node->attr)
    {
        lua_pushstring(L,"@attr");
        lua_pushlstring(L,node->attr,node->attr_len);
        lua_rawset(L,-3);
    }

    if(!node->beg)
    {
        lua_pushstring(L,"@value");
        lua_pushlstring(L,node->data,node->len);
    }else
    {
        lua_pushstring(L,"@elements");
        lua_newtable(L);

        int idx=0;
        for(soap::node* p=node->beg;p;p=p->next)
        {
            lua_pushinteger(L,++idx);
            lua_newtable(L);
            lua_push_xml_node(L,p);

            if(node->name)
            {
                lua_pushstring(L,p->name);
                lua_pushvalue(L,-2);
                lua_rawset(L,-7);
            }

            lua_rawset(L,-3);
        }
    }

    lua_rawset(L,-3);
}

static int lua_xml_decode(lua_State* L)
{
    size_t l=0;

    const char* s=lua_tolstring(L,1,&l);
    if(!s)
        s="";

    lua_newtable(L);

    soap::node root;
    soap::ctx ctx(&root);
    ctx.attributes=1;

    ctx.begin();

    if(!ctx.parse(s,l) && !ctx.end() && root.beg)
    {
        lua_pushstring(L,root.beg->name?root.beg->name:"???");
        lua_newtable(L);
        lua_push_xml_node(L,root.beg);
        lua_rawset(L,-3);
    }

    root.clear();

    return 1;
}

static int lua_soap_find(lua_State* L)
{
    int top=lua_gettop(L);

    const char* s=lua_tostring(L,1);
    if(!s)
        s="";

    int depth=0;

    for(char *p1=(char*)s,*p2=0;p1;p1=p2)
    {
        if(lua_type(L,-1)!=LUA_TTABLE)
        {
            if(depth)
                lua_pop(L,1);

            lua_pushnil(L);
            break;
        }

        int l=0;
        p2=strchr(p1,'/');
        if(p2)
            { l=p2-p1; p2++; }
        else
            l=strlen(p1);
        if(l)
        {
            lua_pushlstring(L,p1,l);
            lua_gettable(L,-2);

            if(depth)
                lua_remove(L,-2);
        }

        depth++;
    }

    return 1;    
}

static void lua_xml_enc(const unsigned char* s,soap::string_builder* b)
{
    while(*s)
    {
        switch(*s)
        {
        case '<':  b->add("&lt;",4);   break;
        case '>':  b->add("&gt;",4);   break;
        case '&':  b->add("&amp;",5);  break;
        case '\"': b->add("&quot;",6); break;
        case '\'': b->add("&apos;",6); break;
        default:   b->add(*s);         break;
        }
        *s++;
    }
}

static void lua_serialize_soap_node(lua_State* L,int idx,soap::string_builder* b)
{
    if(lua_type(L,idx)==LUA_TTABLE)
    {
        lua_pushnil(L);
        while(lua_next(L,idx))
        {
            int tt=lua_gettop(L);

            size_t l=0;
            const char* s=lua_tolstring(L,tt-1,&l);

            b->add('<'); b->add(s,l); b->add('>');

            lua_serialize_soap_node(L,tt,b);

            b->add("</",2); b->add(s,l); b->add('>');

            lua_pop(L,1);
        }
    }else
        lua_xml_enc((unsigned char*)lua_tostring(L,idx),b);
}

static int lua_soap_serialize(lua_State* L)
{
    soap::string_builder b;

    int n=lua_gettop(L);

    for(int i=1;i<=n;i++)
        lua_serialize_soap_node(L,i,&b);

    soap::string s;
    b.swap(s);

    lua_pushlstring(L,s.c_str(),s.length());

    return 1;
}

static int lua_soap_serialize_vector(lua_State* L)
{
    if(lua_type(L,1)!=LUA_TTABLE)
        return 0;

    soap::string_builder b;

//printf("%i\n",lua_gettop(L));

    lua_pushnil(L);
    while(lua_next(L,1))
    {
        if(lua_type(L,-1)!=LUA_TTABLE)
            lua_pop(L,1);
        else
        {
            lua_rawgeti(L,-1,1);
            lua_rawgeti(L,-2,2);

            size_t nname=0;
            const char* name=lua_tolstring(L,-2,&nname);

            size_t nval=0;
            const char* val=lua_tolstring(L,-1,&nval);

            b.add('<'); b.add(name,nname); b.add('>');

//            lua_xml_enc((unsigned char*)lua_tostring(L,-1),&b);
            b.add(val,nval);

            b.add("</",2); b.add(name,nname); b.add('>');

            lua_pop(L,3);
        }
    }

//printf("%i\n",lua_gettop(L));

    soap::string s;
    b.swap(s);

    lua_pushlstring(L,s.c_str(),s.length());

    return 1;
}

static void lua_m3u_parse_track_ext(lua_State* L,const char* track_ext,int tidx)
{
    const char* name=0;
    int nname=0;
    const char* value=0;
    int nvalue=0;

    int st=0;

    for(const char* p=track_ext;;p++)
    {
        switch(st)
        {
        case 0:
            if(*p!=' ')
                { name=p; st=1; }
            break;
        case 1:
            if(*p=='=')
                { nname=p-name; st=2; }
            else if(*p==' ') st=0;
            break;
        case 2:
            if(*p=='\"')
                st=10;
            else
                { value=p; st=3; }
            break;
        case 3:
            if(*p==' ' || *p==0)
            {
                nvalue=p-value;

                if(nname>0 && nvalue>0)
                {
                    lua_pushlstring(L,name,nname);
                    lua_pushlstring(L,value,nvalue);
                    lua_rawset(L,tidx);
                }

                nname=0;
                nvalue=0;
                st=0;
            }
            break;
        case 10:
            if(*p=='\"')
                { nname=0; st=0; }
            else
                { value=p; st=11; }
            break;
        case 11:
            if(*p=='\"')
            {
                nvalue=p-value;

                if(nname>0 && nvalue>0)
                {
                    lua_pushlstring(L,name,nname);
                    lua_pushlstring(L,value,nvalue);
                    lua_rawset(L,tidx);
                }

                nname=0;
                nvalue=0;
                st=0;
            }
            break;
        }

        if(!*p)
            break;
    }
}


/*
t.name - name of playlist
t.size - number of elements
t.elements - array of elements
-------------
t.elements[i].name - element name
t.elements[i].url  - element url
*/
static int lua_m3u_parse(lua_State* L)
{
//printf("* %i\n",lua_gettop(L));
    const char* name=lua_tostring(L,1);
    if(!name)
        name="";

    const char* ext=strrchr(name,'.');

    if(!ext || strcasecmp(ext+1,"m3u"))
        return 0;

    FILE* fp=fopen(name,"r");

    if(!fp)
        return 0;
    else
    {
        lua_newtable(L);

/*
        lua_pushstring(L,"path");
        lua_pushstring(L,name);
        lua_rawset(L,-3);
*/

        char playlist_name[256]="";

        {
            const char* fname=strrchr(name,'/');
            if(!fname)
                fname=name;
            else
                fname++;

            int n=ext-fname;

            if(n>=sizeof(playlist_name))
                n=sizeof(playlist_name)-1;
            memcpy(playlist_name,fname,n);
            playlist_name[n]=0;
        }

        lua_pushstring(L,"name");
        lua_pushstring(L,playlist_name);
        lua_rawset(L,-3);

        lua_pushstring(L,"elements");
        lua_newtable(L);
        int idx=1;

        char track_name[256]="";
        char track_ext[512]="";
        char track_url[256]="";

        char buf[256];
        while(fgets(buf,sizeof(buf),fp))
        {
            char* beg=buf;
            while(*beg && (*beg==' ' || *beg=='\t'))
                beg++;

            char* p=strpbrk(beg,"\r\n");
            if(p)
                *p=0;
            else
                p=beg+strlen(beg);

            while(p>beg && (p[-1]==' ' || p[-1]=='\t'))
                p--;
            *p=0;

            p=beg;

            if(!strncmp(p,"\xEF\xBB\xBF",3))    // skip BOM
                p+=3;

            if(!*p)
                continue;

            if(*p=='#')
            {
                p++;
                static const char tag[]="EXTINF:";
                static const char tag_m3u[]="EXTM3U ";
                if(!strncmp(p,tag,sizeof(tag)-1))
                {
                    p+=sizeof(tag)-1;
                    while(*p && *p==' ')
                        p++;

                    char* p2=strchr(p,',');
                    if(p2)
                    {
                        *p2=0;
                        p2++;

                        char* p3=strchr(p,' ');
                        if(p3)
                        {
                            p3++;
                            while(*p3 && *p3==' ')
                                p3++;

                            int n=snprintf(track_ext,sizeof(track_ext),"%s",p3);
                            if(n==-1 || n>=sizeof(track_ext))
                                track_ext[sizeof(track_ext)-1]=0;
                        }

                        int n=snprintf(track_name,sizeof(track_name),"%s",p2);
                        if(n==-1 || n>=sizeof(track_name))
                            track_name[sizeof(track_name)-1]=0;
                    }
                }else if(!strncmp(p,tag_m3u,sizeof(tag_m3u)-1))
                {
                    p+=sizeof(tag_m3u)-1;
                    while(*p && *p==' ')
                        p++;
                    lua_m3u_parse_track_ext(L,p,-5);
                }
            }else
            {
                int n=snprintf(track_url,sizeof(track_url),"%s",p);
                if(n==-1 || n>=sizeof(track_url))
                    track_url[sizeof(track_url)-1]=0;

                lua_pushinteger(L,idx++);

                lua_newtable(L);

                lua_pushstring(L,"name");
                lua_pushstring(L,track_name);
                lua_rawset(L,-3);

                static const char file_tag[]="file://";

                if(strncmp(track_url,file_tag,sizeof(file_tag)-1))
                {
                    lua_pushstring(L,"url");
                    lua_pushstring(L,track_url);
                    lua_rawset(L,-3);
                }else
                {
                    char* _track_url=track_url+sizeof(file_tag)-1;

                    lua_pushstring(L,"path");
                    lua_pushstring(L,_track_url);
                    lua_rawset(L,-3);

                    char* pp=strrchr(track_url,'/');
                    if(pp)
                        pp++;
                    else
                        pp=track_url;

                    lua_pushstring(L,"url");
                    lua_pushstring(L,pp);
                    lua_rawset(L,-3);

                    int fd=open(_track_url,O_RDONLY|O_LARGEFILE);
                    if(fd!=-1)
                    {
                        off64_t len=lseek64(fd,0,SEEK_END);
                        if(len!=(off64_t)-1)
                        {
                            lua_pushstring(L,"length");
                            lua_pushnumber(L,len);
                            lua_rawset(L,-3);
                        }
                        close(fd);
                    }
                }

                lua_m3u_parse_track_ext(L,track_ext,-3);

                lua_rawset(L,-3);

                *track_name=0;
                *track_ext=0;
                *track_url=0;
            }
        }

        lua_rawset(L,-3);

        lua_pushstring(L,"size");
        lua_pushinteger(L,idx-1);
        lua_rawset(L,-3);

        fclose(fp);
    }
//printf("* %i\n",lua_gettop(L));

    return 1;
}


static int lua_m3u_scan(lua_State* L)
{
    const char* path=lua_tostring(L,1);
    if(!path)
        path="";

    DIR* d=opendir(path);
    if(!d)
        return 0;
    else
    {
        lua_newtable(L);

        lua_pushstring(L,"filesystem");
        lua_pushboolean(L,1);
        lua_rawset(L,-3);

        {
            const char* fname=strrchr(path,'/');
            if(!fname)
                fname=path;
            else
                fname++;

            lua_pushstring(L,"name");
            lua_pushstring(L,fname);
            lua_rawset(L,-3);
        }

        const char* delimiter="/";
        if(path[strlen(path)-1]=='/')
            delimiter="";

        lua_pushstring(L,"elements");
        lua_newtable(L);
        int idx=1;

        dirent* de;
        while((de=readdir(d)))
        {
            if(de->d_name[0]!='.')
            {
                char track_url[256]="";

                int n=snprintf(track_url,sizeof(track_url),"%s%s%s",path,delimiter,de->d_name);
                if(n==-1 | n>=sizeof(track_url))
                    track_url[sizeof(track_url)-1]=0;

                DIR* dd=opendir(track_url);
                if(dd)
                {
                    closedir(dd);

                    if(strcmp(de->d_name,"lost+found"))
                    {
                        lua_pushinteger(L,idx++);

                        lua_getglobal(L,"m3u");
                        lua_getfield(L,-1,"scan");
                        lua_remove(L,-2);
                        lua_pushstring(L,track_url);
                        lua_call(L,1,1);

                        lua_rawset(L,-3);       // element
                    }
                }else
                {
                    char* p=strrchr(track_url,'/');
                    if(p)
                        p++;
                    else
                        p=track_url;

                    char* p2=strrchr(p,'.');
                    if(p2 && strcasecmp(p2+1,"srt"))
                    {
                        int fd=open(track_url,O_RDONLY|O_LARGEFILE);
                        if(fd!=-1)
                        {
                            lua_pushinteger(L,idx++);

                            lua_newtable(L);

                            lua_pushstring(L,"name");
                            lua_pushlstring(L,p,p2-p);
                            lua_rawset(L,-3);

                            lua_pushstring(L,"path");
                            lua_pushstring(L,track_url);
                            lua_rawset(L,-3);

                            lua_pushstring(L,"url");
                            lua_pushstring(L,p);
                            lua_rawset(L,-3);

                            off64_t len=lseek64(fd,0,SEEK_END);
                            if(len!=(off64_t)-1)
                            {
                                lua_pushstring(L,"length");
                                lua_pushnumber(L,len);
                                lua_rawset(L,-3);
                            }

                            lua_rawset(L,-3);       // element

                            close(fd);
                        }
                    }
                }
            }
        }

        lua_rawset(L,-3);       // elements

        lua_pushstring(L,"size");
        lua_pushinteger(L,idx-1);
        lua_rawset(L,-3);

        closedir(d);
    }

    return 1;
}

static int lua_util_geturlinfo(lua_State* L)
{
    const char* www_root=lua_tostring(L,1);
    const char* www_url=lua_tostring(L,2);

    if(!www_root)
        www_root="./";

    if(!www_url || *www_url!='/')
        return 0;

    char url[1024];
    int n=snprintf(url,sizeof(url),"%s",www_url);
    if(n<0 || n>=sizeof(url))
        return 0;

    for(char* p=url;*p;p++)
        if(*p=='\\')
            *p='/';

    if(strstr(url,"/../"))
        return 0;

    char* args=strchr(url,'?');
    if(args)
    {
        *args=0;
        args++;
    }else
        args=(char*)"";

    char path[512];

    char* p=url; while(*p=='/') p++;
    int l=strlen(www_root);
    if(l>0 && www_root[l-1]!='/')
        n=snprintf(path,sizeof(path),"%s/%s",www_root,p);
    else
        n=snprintf(path,sizeof(path),"%s%s",www_root,p);
    if(n<0 || n>=sizeof(path))
        return 0;

    lua_newtable(L);

    lua_pushstring(L,url);
    lua_setfield(L,-2,"url");

    lua_pushstring(L,"args");
    lua_newtable(L);
    for(char* p1=args,*p2;p1;p1=p2)
    {
        p2=strchr(p1,'&');
        if(p2)
        {
            *p2=0;
            p2++;
        }
        p=strchr(p1,'=');
        if(p)
        {
            *p=0;
            p++;

            if(*p1 && *p)
            {
                lua_pushstring(L,p1);
                lua_pushstring(L,util::url_decode(p));
                lua_rawset(L,-3);
            }
        }
    }
    lua_rawset(L,-3);

    lua_pushstring(L,path);
    lua_setfield(L,-2,"path");

    const char* type="none";

    DIR* d=opendir(path);
    if(d)
    {
        type="dir";
        closedir(d);
    }else
    {
        int fd=open(path,O_RDONLY|O_LARGEFILE);
        if(fd!=-1)
        {
            off64_t len=lseek64(fd,0,SEEK_END);
            if(len!=(off64_t)-1)
            {
                lua_pushstring(L,"length");
                lua_pushnumber(L,len);
                lua_rawset(L,-3);
            }
            type="file";
            close(fd);
        }
    }

    lua_pushstring(L,"type");
    lua_pushstring(L,type);
    lua_rawset(L,-3);

    char ext[32];
    if(util::get_file_ext(url,ext,sizeof(ext))>0)
    {
        lua_pushstring(L,"ext");
        lua_pushstring(L,ext);
        lua_rawset(L,-3);
    }

    return 1;
}


static int lua_util_parse_post_data(lua_State* L)
{
    const char* s=lua_tostring(L,1);

    lua_newtable(L);

    if(!s)
        return 1;

    for(const char* p1=s,*p2;p1;p1=p2)
    {
        int l=0;

        const char* p=0;

        p2=0;
        for(const char* pp=p1;*pp;pp++)
        {
            if(*pp=='&')
                { p2=pp; break ;}
            else if(*pp=='=')
                p=pp;
        }

        if(p)
        {
            lua_pushlstring(L,p1,p-p1);

            p++;

            int l=p2?p2-p:strlen(p);

            if(l>0)
            {
                char* buf=(char*)malloc(l+1);

                if(buf)
                {
                    memcpy(buf,p,l);
                    buf[l]=0;

                    lua_pushstring(L,util::url_decode(buf));

                    free(buf);
            
                }
            }else
                lua_pushnil(L);

            lua_rawset(L,-3);
        }

        if(p2)
            p2++;
    }

    return 1;
}


static int lua_util_getfext(lua_State* L)
{
    char ext[32];
    util::get_file_ext(lua_tostring(L,1),ext,sizeof(ext));
    lua_pushstring(L,ext);
    return 1;
}

static int lua_util_getflen(lua_State* L)
{
    const char* s=lua_tostring(L,1);

    if(s)
    {
        int fd=open(s,O_RDONLY|O_LARGEFILE);
        if(fd!=-1)
        {
            off64_t len=lseek64(fd,0,SEEK_END);
            if(len!=(off64_t)-1)
                lua_pushnumber(L,len);
            close(fd);
        }else
            lua_pushnil(L);
    }else
        lua_pushnil(L);

    return 1;
}

static int lua_util_xmlencode(lua_State* L)
{
    const char* s=lua_tostring(L,1);

    if(!s)
        s="";

    soap::string_builder b;

    lua_xml_enc((unsigned char*)s,&b);

    soap::string tmp;
    b.swap(tmp);

    lua_pushlstring(L,tmp.c_str(),tmp.length());
    return 1;
}

static int lua_util_urlencode(lua_State* L)
{
    static const unsigned char t[256]=
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    };

    const unsigned char* s=(unsigned char*)lua_tostring(L,1);

    if(!s)
        s=(unsigned char*)"";

    soap::string_builder b;

    while(*s)
    {
        if(t[*s])
            b.add(*s);
        else
            { char tmp[8]; int n=sprintf(tmp,"%%%.2X",(int)*s); b.add(tmp,n); }
        s++;
    }

    soap::string tmp;
    b.swap(tmp);

    lua_pushlstring(L,tmp.c_str(),tmp.length());
    return 1;
}

static int lua_util_urldecode(lua_State* L)
{
    size_t l=0;
    const char* s=lua_tolstring(L,1,&l);

    if(!s)
    {
        lua_pushstring(L,"");
        return 1;
    }
    char* p=(char*)MALLOC(l+1);
    if(p)
    {
        memcpy(p,s,l+1);
        lua_pushstring(L,util::url_decode(p));
        FREE(p);
    }else
        lua_pushstring(L,"");


    return 1;
}

// 0 - all, 1 - video, 2 - audio, 3 - images
static int lua_search_object_type(const char* exp)
{
    static const char class_tag[]     = "upnp:class";
    static const char derived_tag[]   = "derivedfrom";

    static const char video_tag[]     = "object.item.videoItem";
    static const char audio_tag[]     = "object.item.audioItem";
    static const char image_tag[]     = "object.item.imageItem";

    while(*exp && *exp==' ')
        exp++;

    if(!*exp || !strcmp(exp,"*"))
        return 0;

    const char* p=strstr(exp,class_tag);

    if(p)
    {
        p+=sizeof(class_tag)-1;

        while(*p && (*p==' ' || *p=='\t'))
            p++;

        int ok=1;
        
        if(!strncmp(p,derived_tag,sizeof(derived_tag)-1))
            p+=sizeof(derived_tag)-1;
        else if(*p=='=')
            p++;
        else
            ok=0;

        if(ok)
        {
            while(*p && (*p==' ' || *p=='\t'))
                p++;
            if(*p=='\"')
            {
                p++;

                const char* p2=strchr(p,'\"');

                if(p2)
                {
                    char tmp[64];

                    int n=p2-p;

                    if(n>=sizeof(tmp))
                        n=sizeof(tmp)-1;

                    strncpy(tmp,p,n);
                    tmp[n]=0;

                    if(!strncmp(tmp,video_tag,sizeof(video_tag)-1))
                        return 1;
                    else if(!strncmp(tmp,audio_tag,sizeof(audio_tag)-1))
                        return 2;
                    else if(!strncmp(tmp,image_tag,sizeof(image_tag)-1))
                        return 3;
                }
            }
        }
    }

    return -1;
}

static int lua_util_upnp_search_object_type(lua_State* L)
{
    const char* s=lua_tostring(L,1);
    if(!s)
        s="";

    lua_pushinteger(L,lua_search_object_type(s));
    return 1;
}

static int lua_util_getpid(lua_State* L)
{
    lua_pushinteger(L,getpid());
    return 1;
}

static int lua_util_kill(lua_State* L)
{
    int pid=lua_tointeger(L,1);
    if(pid>0)
        kill(pid,SIGTERM);
    return 0;
}

static int lua_util_multipart_split(lua_State* L)
{
    const char* s=lua_tostring(L,1);

    if(!s)
        return 0;

    while(*s && *s==' ')
        s++;

    const char* p=strpbrk(s,"\r\n");
    if(!p)
        return 0;

    char delimiter[128];

    int dn=p-s;
    if(dn>=sizeof(delimiter))
        return 0;

    memcpy(delimiter,s,dn);
    delimiter[dn]=0;

    int idx=1;

    lua_newtable(L);

    for(const char* p1=s,*p2;p1;p1=p2)
    {
        p1+=dn;
        while(*p1=='\r' || *p1=='\n')
            p1++;

        p2=strstr(p1,delimiter);
        if(p2)
        {
            const char* p3=p2;
            if(p3[-1]=='\n')
                p3--;
            if(p3[-1]=='\r')
                p3--;
            
            lua_pushinteger(L,idx++);
            lua_pushlstring(L,p1,p3-p1);
            lua_rawset(L,-3);
        }        
    }

    return 1;
}


static int lua_util_dir(lua_State* L)
{
    const char* path=lua_tostring(L,1);
    if(!path)
        path="";


    DIR* d=opendir(path);
    if(!d)
        return 0;
    else
    {
        lua_newtable(L);

        int idx=1;

        dirent* de;
        while((de=readdir(d)))
        {
            if(de->d_name[0]!='.')
            {
                lua_pushinteger(L,idx++);
                lua_pushstring(L,de->d_name);
                lua_rawset(L,-3);
            }
        }

        closedir(d);
    }

    return 1;
}

static int lua_util_md5_string_hash(lua_State* L)
{
    size_t l=0;
    const char* s=lua_tolstring(L,1,&l);

    if(!s)
        lua_pushnil(L);
    else
    {
        MD5_CTX ctx;
        MD5_Init(&ctx);            
        MD5_Update(&ctx,(unsigned char*)s,l);

        unsigned char tmp[16];
        MD5_Final(tmp,&ctx);

        char buf[sizeof(tmp)*2];

        static const char hex[]="0123456789abcdef";
        for(int i=0,j=0;i<sizeof(tmp);i++)
        {
            buf[j++]=hex[(tmp[i]>>4)&0x0f];
            buf[j++]=hex[tmp[i]&0x0f];
        }

        lua_pushlstring(L,buf,sizeof(buf));
    }

    return 1;
}


static int lua_util_md5(lua_State* L)
{
    const char* path=lua_tostring(L,1);
    if(!path)
        lua_pushnil(L);
    else
    {
        FILE* fp=fopen(path,"rb");
        if(!fp)
            lua_pushnil(L);
        else
        {
            MD5_CTX ctx;
            MD5_Init(&ctx);

            {
                char tmp[512];
                size_t n;
                while((n=fread(tmp,1,sizeof(tmp),fp))>0)
                    MD5_Update(&ctx,(unsigned char*)tmp,n);
            }

            fclose(fp);

            unsigned char tmp[16];
            MD5_Final(tmp,&ctx);

            char buf[sizeof(tmp)*2];

            static const char hex[]="0123456789abcdef";
            for(int i=0,j=0;i<sizeof(tmp);i++)
            {
                buf[j++]=hex[(tmp[i]>>4)&0x0f];
                buf[j++]=hex[tmp[i]&0x0f];
            }

            lua_pushlstring(L,buf,sizeof(buf));
        }
    }

    return 1;
}

static int lua_util_unlink(lua_State* L)
{
    const char* path=lua_tostring(L,1);
    if(path)
        unlink(path);
    return 0;
}

static int lua_util_win1251toUTF8(lua_State* L)
{
    static const unsigned char win2utf_lead_tab[]=
    {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0,
        0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0,
        0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0,
        0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1
    };

    static const unsigned char win2utf_tab[]=
    {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
        0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
        0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
        0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
        0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
        0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0x81, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
        0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0x91, 0x4e, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
        0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
        0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
        0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f
    };

    const unsigned char* s=(unsigned char*)lua_tostring(L,1);
    if(!s)
        return 0;

    luaL_Buffer B;
    luaL_buffinit(L,&B);

    unsigned char tmp[128];
    int n=0;

    while(*s)
    {
        if(n>sizeof(tmp)-2)
        {
            luaL_addlstring(&B,(char*)tmp,n);
            n=0;
        }

        if(win2utf_lead_tab[*s])
        {
            tmp[n++]=win2utf_lead_tab[*s];
            tmp[n++]=win2utf_tab[*s];
        }else
            tmp[n++]=win2utf_tab[*s];

        s++;
    }

    if(n>0)
        luaL_addlstring(&B,(char*)tmp,n);

    luaL_pushresult(&B);

    return 1;
}

static int lua_util_sleep(lua_State* L)
{
    double n=luaL_checknumber(L,1);

    sleep((int)n);

    return 0;
}

int luaopen_luaxlib(lua_State* L)
{
    static const luaL_Reg lib_soap[]=
    {
        {"parse",lua_soap_parse},
        {"find" ,lua_soap_find},
        {"serialize",lua_soap_serialize},
        {"serialize_vector",lua_soap_serialize_vector},
        {0,0}
    };

    static const luaL_Reg lib_xml[]=
    {
        {"decode",lua_xml_decode},
        {"find" ,lua_soap_find},
        {0,0}
    };

    static const luaL_Reg lib_m3u[]=
    {
        {"parse",lua_m3u_parse},
        {"scan",lua_m3u_scan},
        {0,0}
    };

    static const luaL_Reg lib_util[]=
    {
        {"geturlinfo",lua_util_geturlinfo},
        {"getfext",lua_util_getfext},
        {"getflen",lua_util_getflen},
        {"xmlencode",lua_util_xmlencode},
        {"urlencode",lua_util_urlencode},
        {"urldecode",lua_util_urldecode},
        {"upnp_search_object_type",lua_util_upnp_search_object_type},
        {"getpid",lua_util_getpid},
        {"kill",lua_util_kill},
        {"multipart_split",lua_util_multipart_split},
        {"dir",lua_util_dir},
        {"md5",lua_util_md5},
        {"md5_string_hash",lua_util_md5_string_hash},
        {"unlink",lua_util_unlink},
        {"win1251toUTF8",lua_util_win1251toUTF8},
        {"parse_postdata",lua_util_parse_post_data},
        {"sleep",lua_util_sleep},
        {0,0}
    };

    luaL_register(L,"soap",lib_soap);
    luaL_register(L,"xml",lib_xml);
    luaL_register(L,"m3u",lib_m3u);
    luaL_register(L,"util",lib_util);

    return 0;
}
