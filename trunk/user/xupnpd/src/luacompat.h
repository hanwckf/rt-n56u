/* 
 * Copyright (C) 2011-2015 Anton Burdinuk
 * clark15b@gmail.com
 * https://tsdemuxer.googlecode.com/svn/trunk/xupnpd
*/

#ifndef __LUA_COMPAT_H
#define __LUA_COMPAT_H

extern "C"
{
#include <lua.h>
}

#if LUA_VERSION_NUM > 501
#define lua_open luaL_newstate
#define luaL_register(L,N,F) ( luaL_newlibtable(L,F), luaL_setfuncs(L,F,0), lua_setglobal(L,N) )
#endif

#endif /* __LUA_COMPAT_H */
