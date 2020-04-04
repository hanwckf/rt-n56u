/*
 * Copyright (C) 2016 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef _ENDIAN_H
# error "Never use <bits/endian.h> directly; include <endian.h> instead."
#endif /* _ENDIAN_H  */

#ifdef __NDS32_EB__
#define __BYTE_ORDER __BIG_ENDIAN
#else /* ! __NDS32_EB__  */
#define __BYTE_ORDER __LITTLE_ENDIAN
#endif /* ! __NDS32_EB__  */

#define __FLOAT_WORD_ORDER __BYTE_ORDER
