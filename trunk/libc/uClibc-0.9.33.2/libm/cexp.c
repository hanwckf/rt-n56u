/*
 * Copyright (c) 2011 William Pitcock <nenolod@dereferenced.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <features.h>
#include <math.h>
#include <complex.h>

__complex__ double cexp(__complex__ double z)
{
	__complex__ double ret;
	double r_exponent = exp(__real__ z);

	__real__ ret = r_exponent * cos(__imag__ z);
	__imag__ ret = r_exponent * sin(__imag__ z);

	return ret;
}
libm_hidden_def(cexp)

libm_hidden_proto(cexpf)
__complex__ float cexpf(__complex__ float z)
{
	__complex__ float ret;
	double r_exponent = exp(__real__ z);

	__real__ ret = r_exponent * cosf(__imag__ z);
	__imag__ ret = r_exponent * sinf(__imag__ z);

	return ret;
}
libm_hidden_def(cexpf)

#if defined __UCLIBC_HAS_LONG_DOUBLE_MATH__ && !defined __NO_LONG_DOUBLE_MATH
libm_hidden_proto(cexpl)
__complex__ long double cexpl(__complex__ long double z)
{
	__complex__ long double ret;
	long double r_exponent = expl(__real__ z);

	__real__ ret = r_exponent * cosl(__imag__ z);
	__imag__ ret = r_exponent * sinl(__imag__ z);

	return ret;
}
libm_hidden_def(cexpl)
#endif
