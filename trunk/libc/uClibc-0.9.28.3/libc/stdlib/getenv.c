/* getenv.c for uClibc
   Erik Andersen <andersen@codepoet.org>

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  
   */

#include <string.h>
#include <unistd.h>

/* IEEE Std 1003.1-2001 says getenv need not be thread safe, so 
 * don't bother locking access to __environ */
char *getenv(const char *var)
{
    int len;
    char **ep;

    if (!(ep=__environ))
	return NULL;
    len = strlen(var);
    while(*ep) {
	if (memcmp(var, *ep, len) == 0 && (*ep)[len] == '=') {
	    return *ep + len + 1;
	}
	ep++;
    }
    return NULL;
}

