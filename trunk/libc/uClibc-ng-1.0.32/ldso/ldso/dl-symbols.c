/*
 * This contains all symbols shared between
 * dynamic linker ld.so and into static libc
 *
 * Copyright (c) 2008  STMicroelectronics Ltd
 * Author: Carmelo Amoroso <carmelo.amoroso@st.com>
 *
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 */

/*
 * This is the start of the linked list that describes all of the files present
 * in the system with pointers to all of the symbol, string, and hash tables,
 * as well as all of the other good stuff in the binary.
 */
#include <ldso.h>

struct elf_resolve *_dl_loaded_modules = NULL;

