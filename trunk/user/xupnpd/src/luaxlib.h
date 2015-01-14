/* 
 * Copyright (C) 2011-2015 Anton Burdinuk
 * clark15b@gmail.com
 * https://tsdemuxer.googlecode.com/svn/trunk/xupnpd
 */

#ifndef __LUAXLIB_H
#define __LUAXLIB_H

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}


extern "C" int luaopen_luaxlib(lua_State* L);


#endif
