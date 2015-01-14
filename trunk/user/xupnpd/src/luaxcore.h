/* 
 * Copyright (C) 2011-2015 Anton Burdinuk
 * clark15b@gmail.com
 * https://tsdemuxer.googlecode.com/svn/trunk/xupnpd
 */

#ifndef __LUAXCORE_H
#define __LUAXCORE_H

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace core
{
    extern int detached;
}

extern "C" int luaopen_luaxcore(lua_State* L);


#endif
