#include <tls.h>

#define RESET_VGETCPU_CACHE() \
  do {			      \
    __asm__ __volatile__ ("movl %0, %%fs:%P1\n\t"				      \
		  "movl %0, %%fs:%P2"					      \
		  :							      \
		  : "ir" (0), "i" (offsetof (struct pthread,		      \
					     header.vgetcpu_cache[0])),	      \
		    "i" (offsetof (struct pthread,			      \
				   header.vgetcpu_cache[1])));		\
  } while (0)

#include "../pthread_setaffinity.c"
