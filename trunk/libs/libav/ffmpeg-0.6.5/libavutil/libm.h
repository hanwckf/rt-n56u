/*
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * Replacements for frequently missing libm functions
 */

#ifndef AVUTIL_LIBM_H
#define AVUTIL_LIBM_H

#include <math.h>
#include "config.h"
#include "attributes.h"

#if !HAVE_EXP2
#undef exp2
#define exp2(x) exp((x) * 0.693147180559945)
#endif /* HAVE_EXP2 */

#if !HAVE_EXP2F
#undef exp2f
#define exp2f(x) ((float)exp2(x))
#endif /* HAVE_EXP2F */

//#if !HAVE_LLRINT
#undef llrint
#define llrint(x) ((long long)rint(x))
//#endif /* HAVE_LLRINT */

//#if !HAVE_LLRINTF
#undef llrintf
#define llrintf(x) ((long long)rint(x))
//#endif /* HAVE_LLRINT */

#if !HAVE_LOG2
#undef log2
#define log2(x) (log(x) * 1.44269504088896340736)
#endif /* HAVE_LOG2 */

#if !HAVE_LOG2F
#undef log2f
#define log2f(x) ((float)log2(x))
#endif /* HAVE_LOG2F */

//#if !HAVE_LRINT
#undef lrint
#define lrint(x) (rint(x))
//#endif /* HAVE_LRINT */

//#if !HAVE_LRINTF
#undef lrintf
#define lrintf(x) ((int)(rint(x)))
//#endif /* HAVE_LRINTF */

#if !HAVE_ROUND
#undef round
#define round(x) ((x > 0) ? floor(x + 0.5) : ceil(x - 0.5))
#endif /* HAVE_ROUND */

#if !HAVE_ROUNDF
#undef roundf
#define roundf(x) ((x > 0) ? floor(x + 0.5) : ceil(x - 0.5))
#endif /* HAVE_ROUNDF */

//#if !HAVE_TRUNCF
#undef truncf
#define truncf(x) ((x > 0) ? floor(x) : ceil(x))
//#endif /* HAVE_TRUNCF */

#endif /* AVUTIL_LIBM_H */
