/* uClibc internal malloc.
   Copyright (C) 2007 Denys Vlasenko

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License
version 2 as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, see <http://www.gnu.org/licenses/>.

*/

#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>


void (*__uc_malloc_failed)(size_t size) = NULL;
/* Seemingly superfluous assigment of NULL above prevents gas error
 * ("__uc_malloc_failed can't be equated to common symbol
 * __GI___uc_malloc_failed") in libc_hidden_data_def: */
libc_hidden_data_def(__uc_malloc_failed)

void *__uc_malloc(size_t size)
{
	void *p;

	while (1) {
		p = malloc(size);
		if (!size || p)
			return p;
		if (!__uc_malloc_failed)
			_exit(1);
		free(p);
		__uc_malloc_failed(size);
	}
}
libc_hidden_def(__uc_malloc)
