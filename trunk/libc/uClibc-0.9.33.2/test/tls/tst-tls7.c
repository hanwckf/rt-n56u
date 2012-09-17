#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include <tls.h>
#include <link.h>
#ifdef __UCLIBC__
#include "dl-elf.h"
#include "dl-hash.h"
#endif


#define TEST_FUNCTION do_test ()
static int
do_test (void)
{
#ifdef USE_TLS
  static const char modname[] = "tst-tlsmod3.so";
  int result = 0;
  int (*fp) (void);
  void *h;
  int i;
  int modid = -1;

  for (i = 0; i < 10; ++i)
    {
      h = dlopen (modname, RTLD_LAZY);
      if (h == NULL)
	{
	  printf ("cannot open '%s': %s\n", modname, dlerror ());
	  exit (1);
	}

      /* Dirty test code here: we peek into a private data structure.
	 We make sure that the module gets assigned the same ID every
	 time.  The value of the first round is used.  */
#ifdef __UCLIBC__
      if (modid == -1)
	modid = ((struct link_map *)((struct dyn_elf *)h)->dyn)->l_tls_modid;
      else if (((struct link_map *)((struct dyn_elf *)h)->dyn)->l_tls_modid
        != (size_t) modid)
	{
	  printf ("round %d: modid now %zu, initially %d\n",
		  i,
		  ((struct link_map *)((struct dyn_elf *)h)->dyn)->l_tls_modid,
		  modid);
	  result = 1;
	}
#else
      if (modid == -1)
	modid = ((struct link_map *) h)->l_tls_modid;
      else if (((struct link_map *) h)->l_tls_modid != (size_t) modid)
	{
	  printf ("round %d: modid now %zu, initially %d\n",
		  i, ((struct link_map *) h)->l_tls_modid, modid);
	  result = 1;
	}
#endif

      fp = dlsym (h, "in_dso2");
      if (fp == NULL)
	{
	  printf ("cannot get symbol 'in_dso2': %s\n", dlerror ());
	  exit (1);
	}

      result |= fp ();

      dlclose (h);
    }

  return result;
#else
  return 0;
#endif
}


#include "../test-skeleton.c"
