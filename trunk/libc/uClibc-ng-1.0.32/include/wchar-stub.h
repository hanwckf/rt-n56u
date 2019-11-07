/* This wchar.h is used if wchar support is disabled in uClibc.
 * We still want to provide a few basic definitions as the basic
 * C standard requires them.  And it makes our lives easier with
 * no additional overhead.
 */

#ifndef _WCHAR_H
#define _WCHAR_H

typedef unsigned int wint_t;
#define WEOF (0xffffffffu)

#endif
