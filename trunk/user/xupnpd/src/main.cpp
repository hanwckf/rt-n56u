/* 
 * Copyright (C) 2011-2015 Anton Burdinuk
 * clark15b@gmail.com
 * https://tsdemuxer.googlecode.com/svn/trunk/xupnpd
 */

#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "luacompat.h"
#include "luaxlib.h"
#include "luaxcore.h"
#include "luajson.h"

int main(int argc,char** argv)
{
#if 0
    const char* p=strrchr(argv[0],'/');

    int rc;

    if(p)
    {
        char location[512];
        int n=p-argv[0];
        if(n>=sizeof(location))
            n=sizeof(location)-1;
        strncpy(location,argv[0],n);
        location[n]=0;

        rc=chdir(location);

        argv[0]=(char*)p+1;
    }

    const char* root=getenv("XUPNPDROOTDIR");
    if(root && *root)
        rc=chdir(root);

    {
        FILE* fp=fopen("xupnpd.lua","r");
        if(fp)
            fclose(fp);
        else
            rc=chdir("/usr/share/xupnpd/");
    }
#else
    int rc = chdir("/etc_ro/xupnpd");
#endif

    lua_State* L=lua_open();
    if(L)
    {
        luaL_openlibs(L);
        luaopen_luaxlib(L);
        luaopen_luaxcore(L);
        luaopen_luajson(L);

        lua_newtable(L);
        for(int i=0;i<argc;i++)
        {
            lua_pushinteger(L,i+1);
            lua_pushstring(L,argv[i]);
            lua_rawset(L,-3);        
        }
        lua_setglobal(L,"arg");

#if 0
//        char initfile[128];
//        snprintf(initfile,sizeof(initfile),"%s.lua",argv[0]);
        const char initfile[]="xupnpd.lua";
#else
        const char initfile[]="/etc/storage/xupnpd/xupnpd.lua";
#endif

        if(luaL_loadfile(L,initfile) || lua_pcall(L,0,0,0))
        {
            const char* s=lua_tostring(L,-1);

            if(core::detached)
                syslog(LOG_INFO,"%s",s);
            else
                fprintf(stderr,"%s\n",s);

            lua_pop(L,1);
        }

        lua_close(L);
    }                                    
    
    return 0;
}
