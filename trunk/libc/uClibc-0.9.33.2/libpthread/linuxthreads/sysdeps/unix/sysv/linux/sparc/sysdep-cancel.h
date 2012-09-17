#ifndef SPARC_SYSDEP_CANCEL_H
#define SPARC_SYSDEP_CANCEL_H

#if defined(__arch64__)
#include <sparc64/sysdep-cancel.h>
#else
#include <sparc32/sysdep-cancel.h>
#endif

#endif
