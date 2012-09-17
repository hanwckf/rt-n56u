/* XXX add ops from glibc sysdeps/sparc/sparc64/soft-fp */

#include <stdio.h>
#include <stdlib.h>

static void fakedef(void)
{
	fputs("Unimplemented _Q* func called, exiting\n", stderr);
	exit(-1);
}

#ifdef __sparc_v9__
# define fakedef(sym) strong_alias(fakedef, _Qp_##sym)
#else
# define fakedef(sym) strong_alias(fakedef, _Q_##sym)
#endif

fakedef(fne)
fakedef(feq)
fakedef(div)
fakedef(flt)
fakedef(fgt)
fakedef(mul)
fakedef(fge)
fakedef(qtoux)
fakedef(uxtoq)
fakedef(sub)
fakedef(dtoq)
fakedef(qtod)
fakedef(qtos)
fakedef(stoq)
fakedef(itoq)
fakedef(add)
#ifndef __sparc_v9__
fakedef(qtou)
fakedef(utoq)
fakedef(cmp)
fakedef(cmpe)
fakedef(fle)
fakedef(lltoq)
fakedef(neg)
fakedef(qtoi)
fakedef(qtoll)
fakedef(qtoull)
fakedef(sqrt)
fakedef(ulltoq)
#endif
