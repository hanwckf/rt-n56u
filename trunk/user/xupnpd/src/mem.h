/* 
 * Copyright (C) 2011-2015 Anton Burdinuk
 * clark15b@gmail.com
 * https://tsdemuxer.googlecode.com/svn/trunk/xupnpd
 */

#ifndef __MEM_H
#define __MEM_H

#include <stdlib.h>

#ifndef DEBUG
#define MALLOC(S) malloc(S)
#define FREE(P) free(P)
#else
#define MALLOC(S) mem::malloc(S)
#define FREE(P) mem::free(P)

#include <sys/types.h>

namespace mem
{
    extern size_t mem_total;

    void* malloc(size_t size);
    void free(void* ptr);
}


#endif /* DEBUG */

#endif
