/*
 * Copyright (C) 2006 CodeSourcery Inc
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 * This file defines __shared_flat_add_library.  If a library has
 * initialistion and finalisation code, it should use this routine
 * to register itself.
 */
#include "shared_flat_lib.h"

/* The initialisation and finalisation symbols for this library.  */
extern void _init(void) attribute_hidden weak_function;
extern void _fini(void) attribute_hidden weak_function;
extern void (*__preinit_array_start[])(void) attribute_hidden;
extern void (*__preinit_array_end[])(void) attribute_hidden;
extern void (*__init_array_start[])(void) attribute_hidden;
extern void (*__init_array_end[])(void) attribute_hidden;
extern void (*__fini_array_start[])(void) attribute_hidden;
extern void (*__fini_array_end[])(void) attribute_hidden;

/* The shared_flat_lib structure that describes this library.  */
static struct shared_flat_lib this_lib = {
	0,
	0,
	__preinit_array_start,
	__preinit_array_end,
	__init_array_start,
	__init_array_end,
	__fini_array_start,
	__fini_array_end,
	_init,
	_fini
};

/* Add this_lib to the end of the global list. */
void __shared_flat_add_library(void) attribute_hidden;
void __shared_flat_add_library(void)
{
	this_lib.prev = __last_shared_lib;
	if (this_lib.prev)
		this_lib.prev->next = &this_lib;
	else
		__first_shared_lib = &this_lib;
	__last_shared_lib = &this_lib;
}
