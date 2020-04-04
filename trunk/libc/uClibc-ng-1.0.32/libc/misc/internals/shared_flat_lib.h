/*
 * Copyright (C) 2006 CodeSourcery Inc
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 * This file defines the shared_flat_lib structure and the global library
 * list.   The structure is used to provide something close to ELF-like
 * initialisation and finalisation when using shared flat libraries.
 */
#ifndef __SHARED_FLAT_LIB__
#define __SHARED_FLAT_LIB__

struct shared_flat_lib {
	struct shared_flat_lib *prev;
	struct shared_flat_lib *next;
	/* .preinit_array is usually only supported for executables.
	 * However, the distinction between the executable and its
	 * shared libraries isn't as pronounced for flat files; a shared
	 * library is really just a part of an executable that can be
	 * shared with other executables.  We therefore allow
	 * .preinit_array to be used in libraries too.  */
	void (**preinit_array_start)(void);
	void (**preinit_array_end)(void);
	void (**init_array_start)(void);
	void (**init_array_end)(void);
	void (**fini_array_start)(void);
	void (**fini_array_end)(void);
	void (*init)(void);
	void (*fini)(void);
};

extern struct shared_flat_lib *__first_shared_lib;
extern struct shared_flat_lib *__last_shared_lib;

#endif
