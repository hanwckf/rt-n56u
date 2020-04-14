/* 
 * Copyright (C) 2011-2015 Anton Burdinuk
 * clark15b@gmail.com
 * https://tsdemuxer.googlecode.com/svn/trunk/xupnpd
 */

#include "luajson.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>

namespace libjson
{
    static const unsigned char json_chtype[256]=
    {
	0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x00,0x00,0x01,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x03,0x04,0x04,0x00,
	0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,
	0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x02,0x00,0x03,0x00,0x00,
	0x00,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,
	0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x02,0x00,0x03,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    };

    static const char err_stack_overflow[]=		"JSON: stack overflow";
    static const char err_illegal_stack_state[]=	"JSON: illegal stack state";
    static const char err_invalid_literal_length[]=	"JSON: invalid literal length";
    static const char err_illegal_literal[]=		"JSON: illegal literal";
    static const char err_invalid_number_length[]=	"JSON: invalid number length";
    static const char err_illegal_number[]=		"JSON: illegal number";
    static const char err_illegal_character[]=		"JSON: illegal character '%c'";
    static const char err_unclosed_tag[]=		"JSON: unclosed tag";


    enum
    {
	cht_oth=0,
	cht_spc=1,
	cht_dm1=2,
	cht_dm2=3,
	cht_num=4,
	cht_lit=5
    };

    
    class json_st
    {
    public:

	enum {max_stack_depth=64};

	enum
	{
	    st_value=0,
	    st_object,
	    st_object_key,
	    st_object_value,
	    st_object_value_2,
	    st_array,
	    st_array_value,
	    st_string,
	    st_number,
	    st_literal,
	    st_delim,
	};

    protected:

	lua_State* L;
	int st[max_stack_depth];
	int key[max_stack_depth];	// array key
	int index;

    public:
	json_st(lua_State* _L):index(0),L(_L)
	{
	    for(int i=0;i<max_stack_depth;i++)
		{st[i]=st_value; key[i]=0; }
	}
	
	void push(int newst=st_value)
	{
	    if(index>=max_stack_depth)
		luaL_error(L,err_stack_overflow);
	    st[++index]=newst;
	}

	
	void pop(int n=1)
	{
	    if(index<n)
		luaL_error(L,err_illegal_stack_state);
	    index-=n;
	}
	
	void set(int newst) { st[index]=newst; }
	
	int get(int ind=0)
	{
	    int n;

	    if(ind>0 || (n=index+ind)<0)
		luaL_error(L,err_illegal_stack_state);

	    return st[n];
	}
	
	int get_top(void) { return index; }
	
	void reset_key(void) { key[index]=0; }
	int get_key(void) { return ++key[index]; }
    };
    
    void luaL_addjson_unesc_lstring(luaL_Buffer* buf,const unsigned char* p,size_t l);

    class json_tok
    {
    protected:

	lua_State* L;
	const unsigned char* ptr;

    public:
	json_tok(lua_State* _L):L(_L),ptr(0) {}
	
	void join(const unsigned char* p) { ptr=p; }

	void push_string(const unsigned char* p)
	{
	    if(ptr && p>=ptr)
	    {
		luaL_Buffer buf;
		luaL_buffinit(L,&buf);
		
		luaL_addjson_unesc_lstring(&buf,ptr,p-ptr);
		
		luaL_pushresult(&buf);
	    }
	    
	    ptr=0;
	}
	
	void push_literal(const unsigned char* p)
	{
	    if(ptr && p>=ptr)
	    {
		size_t len=p-ptr;
		
		char tmp[8];
		
		if(len>sizeof(tmp)-1)
		    luaL_error(L,err_invalid_literal_length);

		memcpy(tmp,ptr,len);
		tmp[len]=0;

		if(!strcmp(tmp,"null"))
		    lua_pushnil(L);
		else if(!strcmp(tmp,"true"))
		    lua_pushboolean(L,1);
		else if(!strcmp(tmp,"false"))
		    lua_pushboolean(L,0);
		else
		    luaL_error(L,err_illegal_literal);
	    }
	    
	    ptr=0;
	}

	void push_number(const unsigned char* p)
	{
	    if(ptr && p>=ptr)
	    {
		char tmp[64];
		
		size_t len=p-ptr;
		
		if(len>=sizeof(tmp))
		    luaL_error(L,err_invalid_number_length);

		memcpy(tmp,ptr,len);
		tmp[len]=0;
		
		char* p=0;
		double n=strtod(tmp,&p);
		
		if(*p)
		    luaL_error(L,err_illegal_number);

		lua_pushnumber(L,n);
	    }
	    
	    ptr=0;
	}
    };

    
    int lua_json_parse(lua_State* L,const unsigned char* p,size_t l);
}

int libjson::lua_json_parse(lua_State* L,const unsigned char* p,size_t l)
{
    int top=lua_gettop(L);
    
    json_st st(L);
    json_tok tok(L);
    
    for(size_t i=0;i<=l;++i)
    {
	unsigned char ch=p[i];

	int chtype=json_chtype[ch];

	switch(st.get())
	{
	case json_st::st_value:
	    if(chtype==cht_spc)
		break;
	    
	    if(ch=='[') { lua_newtable(L); st.reset_key(); if(st.get_top()>0) st.set(json_st::st_array); else st.push(json_st::st_array); }
	    else if(ch=='{') { lua_newtable(L); if(st.get_top()>0) st.set(json_st::st_object); else st.push(json_st::st_object); }
	    else if(ch=='\"') { tok.join(p+i+1); if(st.get_top()>0) st.set(json_st::st_string); else st.push(json_st::st_string); }
	    else if(chtype==cht_num) { tok.join(p+i); if(st.get_top()>0) st.set(json_st::st_number); else st.push(json_st::st_number); }
	    else if(chtype==cht_lit) { tok.join(p+i); if(st.get_top()>0) st.set(json_st::st_literal); else st.push(json_st::st_literal); }
	    else luaL_error(L,err_illegal_character,ch);

	    break;
	case json_st::st_string:
	    if(ch=='\\') { i++; break; }
	    else if(ch!='\"') break;

	    tok.push_string(p+i);
	    
	    if(st.get_top()>1) st.pop(); else st.set(json_st::st_delim);

	    break;
	case json_st::st_number:
	    if(chtype!=cht_spc && chtype!=cht_dm2)
		break;

	    tok.push_number(p+i);

	    if(st.get_top()>1) st.pop(); else st.set(json_st::st_delim);

	    i--;

	    break;
	case json_st::st_literal:
	    if(chtype!=cht_spc && chtype!=cht_dm2)
		break;

	    tok.push_literal(p+i);

	    if(st.get_top()>1) st.pop(); else st.set(json_st::st_delim);

	    i--;

	    break;
	case json_st::st_delim:

            if(ch==',') st.pop();
	    else if(chtype==cht_dm2 || ch=='\0') { st.pop(); i--; }
	    else if(chtype!=cht_spc)
		luaL_error(L,err_illegal_character,ch);

	    break;
	case json_st::st_object:
	    if(chtype==cht_spc)
		break;

	    switch(ch)
	    {
	    case '\"':
		tok.join(p+i+1);
		st.set(json_st::st_object_key);
		st.push(json_st::st_string);
		break;
	    case '}':
		if(st.get_top()>1) st.pop(); else st.set(json_st::st_delim);
		break;
	    default:
		luaL_error(L,err_illegal_character,ch);
		break;
    	    }

	    break;
	case json_st::st_object_key:
	    if(chtype==cht_spc)
		break;

	    if(ch==':')
		st.set(json_st::st_object_value);
	    else
	        luaL_error(L,err_illegal_character,ch);

	    break;
	case json_st::st_object_value:
	    if(chtype==cht_spc)
		break;

	    if(chtype!=cht_dm1 && chtype!=cht_num && chtype!=cht_lit && ch!='\"')
		luaL_error(L,err_illegal_character,ch);

	    st.set(json_st::st_object_value_2);
	    st.push(json_st::st_value);
	    i--;

	    break;
	case json_st::st_object_value_2:
	    if(chtype==cht_spc)
		break;

	    switch(ch)
	    {
	    case '}':
		lua_rawset(L,-3);
		if(st.get_top()>1) st.pop(); else st.set(json_st::st_delim);
		break;
	    case ',':
		st.set(json_st::st_object);
		lua_rawset(L,-3);
		break;
	    default:
		luaL_error(L,err_illegal_character,ch);
		break;
    	    }

	    break;
	case json_st::st_array:
	    if(chtype==cht_spc)
		break;

	    if(ch==']')
		if(st.get_top()>1) st.pop(); else st.set(json_st::st_delim);
	    else if(chtype!=cht_dm1 && chtype!=cht_num && chtype!=cht_lit && ch!='\"')
		luaL_error(L,err_illegal_character,ch);
	    else
	    {
		st.set(json_st::st_array_value);
		st.push(json_st::st_value);
		i--;
	    }

	    break;
	case json_st::st_array_value:
	    if(chtype==cht_spc)
		break;

	    switch(ch)
	    {
	    case ']':
		lua_rawseti(L,-2,st.get_key());
		if(st.get_top()>1) st.pop(); else st.set(json_st::st_delim);
		break;
	    case ',':
		lua_rawseti(L,-2,st.get_key());
		st.set(json_st::st_array);
		break;
	    default:
		luaL_error(L,err_illegal_character,ch);
		break;
    	    }

	    break;
	}
    }
    
    int s=st.get();

    if(s!=json_st::st_value)
	luaL_error(L,err_unclosed_tag);


    return lua_gettop(L)-top;
}
