/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

#define __DO_LARGEFILE
#define FTELL         ftello64
#define OFFSET_TYPE   __off64_t
#include "ftello.c"
