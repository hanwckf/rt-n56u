/* gen.h: Generic definitions shared throughout the elfparts library.
 *
 * Copyright (C) 1999-2001 by Brian Raiter, under the GNU General
 * Public License. No warranty. See COPYING for details.
 */

#ifndef	_gen_h_
#define	_gen_h_

#include	<stdlib.h>
#include	<assert.h>

#ifndef TRUE
#define	TRUE		1
#define	FALSE		0
#endif

/* Allocates memory.
 */
#define	xalloc(p, n)	(((p) = realloc((p), (n))) ? (p)	\
				: (assert(!"Out of memory!"), (void*)0))

/* Allocates memory for a part's contents.
 */
#define	palloc(p)	(xalloc((p)->part, (p)->size))

#endif
