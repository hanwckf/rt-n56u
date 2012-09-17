/* copied from rsync */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdarg.h>
static int foo(const char *format, ...)
{
	va_list ap;
	size_t len;
	char buf[5];

	va_start(ap, format);
	len = vsnprintf(0, 0, format, ap);
	va_end(ap);
	if (len != 5) return(1);

	if (snprintf(buf, 3, "hello") != 5 || strcmp(buf, "he") != 0) return(1);

	return(0);
}
int main(void) { return foo("hello"); }
