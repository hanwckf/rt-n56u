/* Get loaded objects program headers.

  Based on GNU C library (file: libc/elf/dl-iteratephdr.c)

  Copyright (C) 2001,2002,2003,2004,2006,2007 Free Software Foundation, Inc.
  Contributed by Jakub Jelinek <jakub@redhat.com>, 2001.

  Copyright (C) 2008 STMicroelectronics Ltd.
  Author: Carmelo Amoroso <carmelo.amoroso@st.com>

  Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
*/


#include <link.h>
#include <ldso.h>

/* we want this in libc but nowhere else */
#ifdef __USE_GNU

static int
__dl_iterate_phdr (int (*callback) (struct dl_phdr_info *info, size_t size, void *data), void *data)
{
	int ret = 0;
#ifndef __ARCH_HAS_NO_SHARED__
	struct elf_resolve *l;
	struct dl_phdr_info info;

	for (l = _dl_loaded_modules; l != NULL; l = l->next) {
		info.dlpi_addr = l->loadaddr;
		info.dlpi_name = l->libname;
		info.dlpi_phdr = l->ppnt;
		info.dlpi_phnum = l->n_phent;
		ret = callback (&info, sizeof (struct dl_phdr_info), data);
		if (ret)
			break;
	}
#endif
	return ret;
}

# ifdef SHARED

weak_alias(__dl_iterate_phdr, dl_iterate_phdr)

# else

/* dl-support.c defines these and initializes them early on.  */
extern ElfW(Phdr) *_dl_phdr;
extern size_t _dl_phnum;

int
dl_iterate_phdr (int (*callback) (struct dl_phdr_info *info,
                                  size_t size, void *data), void *data)
{
  if (_dl_phnum != 0)
    {
      /* This entry describes this statically-linked program itself.  */
      struct dl_phdr_info info;
      int ret;
#if defined(__FRV_FDPIC__) || defined(__BFIN_FDPIC__) || defined(__FDPIC__)
      info.dlpi_addr.map = NULL;
      info.dlpi_addr.got_value = NULL;
#elif defined(__DSBT__)
      info.dlpi_addr.map = NULL;
#else
      info.dlpi_addr = 0;
#endif
      info.dlpi_name = "";
      info.dlpi_phdr = _dl_phdr;
      info.dlpi_phnum = _dl_phnum;
      ret = (*callback) (&info, sizeof (struct dl_phdr_info), data);
      if (ret)
        return ret;
    }
   /* Then invoke callback on loaded modules, if any */
  return __dl_iterate_phdr (callback, data);
}

# endif
#endif
