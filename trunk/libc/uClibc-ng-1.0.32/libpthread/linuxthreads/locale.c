/*  Copyright (C) 2003     Manuel Novoa III
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, see
 *  <http://www.gnu.org/licenses/>.
 */

#include <features.h>
#include "pthread.h"
#include "internals.h"
#include <locale.h>
#include <assert.h>
#include <stdlib.h>

extern struct _pthread_descr_struct __pthread_initial_thread;

__locale_t __curlocale(void)
{
	pthread_descr self = thread_self();

#ifdef NDEBUG
	return THREAD_GETMEM (self, locale);
#else
	{
		__locale_t r = THREAD_GETMEM (self, locale);

		assert(r);

		return r;
	}
#endif
}

__locale_t __curlocale_set(__locale_t newloc)
{
	__locale_t oldloc;
	pthread_descr self = thread_self();

	oldloc = THREAD_GETMEM (self, locale);

	assert(newloc != LC_GLOBAL_LOCALE);
	assert(oldloc);

	THREAD_SETMEM (self, locale, newloc);

	return oldloc;
}
