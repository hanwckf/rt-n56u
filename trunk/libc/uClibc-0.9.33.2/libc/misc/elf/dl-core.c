/*
 * This contains all symbols and functions to support
 * dynamic linking into static libc.

 * Copyright (c) 2008  STMicroelectronics Ltd
 * Author: Carmelo Amoroso <carmelo.amoroso@st.com>
 *
 * Based on draft work by Peter S. Mazinger <ps.m@gmx.net>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 */

#ifdef SHARED
#error "This file is not suitable for linking into dynamic libc"
#else
/* Include ldso symbols and functions used into static libc */
#include "../../../ldso/ldso/dl-symbols.c"
#endif

