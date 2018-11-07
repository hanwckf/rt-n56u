 /*
   Linux error(3) function go around for systems that has err(3) and
   warn(3), but no error(3).  MacOS is good example of such.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation version 2.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.
*/

#include <stdarg.h>
#include <err.h>

void error(int status, int errnum, const char *format, ...) {
  va_list arg;

  va_start(arg, format);
  if (errnum == 0) {
    if (status == 0)
      vwarnx(format, arg);
    else
      verrx(status, format, arg);
  } else {
    if (status == 0)
      vwarn(format, arg);
    else
      verr(status, format, arg);
  }
  va_end(arg);
}
