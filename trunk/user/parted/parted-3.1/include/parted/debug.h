/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 1998-2000, 2002, 2007, 2009-2012 Free Software Foundation,
    Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PED_DEBUG_H_INCLUDED
#define PED_DEBUG_H_INCLUDED

#include <stdarg.h>

#ifdef DEBUG

typedef void (PedDebugHandler) ( const int level, const char* file, int line,
                                 const char* function, const char* msg );

extern void ped_debug_set_handler (PedDebugHandler* handler);
extern void ped_debug ( const int level, const char* file, int line,
                        const char* function, const char* msg, ... );

extern void __attribute__((__noreturn__))
ped_assert ( const char* cond_text,
                         const char* file, int line, const char* function );

#if defined __GNUC__ && !defined __JSFTRACE__

#define PED_DEBUG(level, ...) \
        ped_debug ( level, __FILE__, __LINE__, __PRETTY_FUNCTION__, \
                    __VA_ARGS__ )

#define PED_ASSERT(cond)					\
	do {							\
		if (!(cond)) {					\
			ped_assert (				\
			  #cond,				\
			  __FILE__,				\
			  __LINE__,				\
			  __PRETTY_FUNCTION__ );		\
		}						\
	} while (0)

#else /* !__GNUC__ */

/* function because variadic macros are C99 */
static void PED_DEBUG (int level, ...)
{
        va_list         va_args;

        va_start (va_args, level);
        ped_debug ( level, "unknown file", 0, "unknown function", va_args );
        va_end (va_args);
}

#define PED_ASSERT(cond)					\
	do {							\
		if (!(cond)) {					\
			ped_assert (				\
			  #cond,				\
			  "unknown",				\
			  0,					\
			  "unknown");				\
		}						\
	} while (0)

#endif /* __GNUC__ */

#else /* !DEBUG */

#define PED_ASSERT(cond)		do {} while (0)
#define PED_DEBUG(level, ...)           do {} while (0)


#endif /* DEBUG */

#endif /* PED_DEBUG_H_INCLUDED */
