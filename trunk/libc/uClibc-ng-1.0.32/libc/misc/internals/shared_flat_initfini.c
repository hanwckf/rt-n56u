/*
 * Copyright (C) 2006 CodeSourcery Inc
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 * This file defines the main initialisation and finalisation code for
 * shared flat libraries.  It in turn calls the initialisation and
 * finalisation code for each registered library.
 */
#include "shared_flat_lib.h"

/* A doubly-linked list of shared libraries.  Those nearer the head
 * of the list should be initialised first and finalised last.  */
struct shared_flat_lib *__first_shared_lib;
struct shared_flat_lib *__last_shared_lib;

void __shared_flat_init(void)
{
	struct shared_flat_lib *lib;
	void (**start)(void);
	void (**end)(void);

	for (lib = __first_shared_lib; lib; lib = lib->next) {
		end = lib->preinit_array_end;
		for (start = lib->preinit_array_start; start < end; start++)
			(*start)();
	}

	for (lib = __first_shared_lib; lib; lib = lib->next) {
		if (lib->init)
			lib->init();

		end = lib->init_array_end;
		for (start = lib->init_array_start; start < end; start++)
			(*start)();
	}
}

void __shared_flat_fini(void)
{
	struct shared_flat_lib *lib;
	void (**start)(void);
	void (**end)(void);

	for (lib = __last_shared_lib; lib; lib = lib->prev) {
		start = lib->fini_array_start;
		for (end = lib->fini_array_end; end > start;)
			(*--end)();

		if (lib->fini)
			lib->fini();
	}
}
