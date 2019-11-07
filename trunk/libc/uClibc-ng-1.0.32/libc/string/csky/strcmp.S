/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#include <features.h>

#ifdef __CSKYABIV2__
#include "cskyv2/abiv2_strcmp.S"
#else
#include "cskyv1/strcmp.S"
#endif
