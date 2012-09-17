/*
 * bits/stackinfo.h -- v850-specific pthread definitions
 *
 *  Copyright (C) 2003  NEC Electronics Corporation
 *  Copyright (C) 2003  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 */

/* This file contains a bit of information about the stack allocation
   of the processor.  */

#ifndef _STACKINFO_H
#define _STACKINFO_H	1

/* On v80 the stack grows down.  */
#define _STACK_GROWS_DOWN	1

#endif	/* stackinfo.h */
