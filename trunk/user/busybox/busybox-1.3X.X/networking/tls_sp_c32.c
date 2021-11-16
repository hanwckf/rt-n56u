/*
 * Copyright (C) 2021 Denys Vlasenko
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */
#include "tls.h"

#define SP_DEBUG          0
#define FIXED_SECRET      0
#define FIXED_PEER_PUBKEY 0

#if SP_DEBUG
# define dbg(...) fprintf(stderr, __VA_ARGS__)
static void dump_hex(const char *fmt, const void *vp, int len)
{
	char hexbuf[32 * 1024 + 4];
	const uint8_t *p = vp;

	bin2hex(hexbuf, (void*)p, len)[0] = '\0';
	dbg(fmt, hexbuf);
}
#else
# define dbg(...) ((void)0)
# define dump_hex(...) ((void)0)
#endif

#undef DIGIT_BIT
#define DIGIT_BIT  32
typedef int32_t sp_digit;

/* The code below is taken from parts of
 *  wolfssl-3.15.3/wolfcrypt/src/sp_c32.c
 * and heavily modified.
 * Header comment is kept intact:
 */

/* sp.c
 *
 * Copyright (C) 2006-2018 wolfSSL Inc.
 *
 * This file is part of wolfSSL.
 *
 * wolfSSL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * wolfSSL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA
 */

/* Implementation by Sean Parkinson. */

typedef struct sp_point {
	sp_digit x[2 * 10];
	sp_digit y[2 * 10];
	sp_digit z[2 * 10];
	int infinity;
} sp_point;

/* The modulus (prime) of the curve P256. */
static const sp_digit p256_mod[10] = {
	0x3ffffff,0x3ffffff,0x3ffffff,0x003ffff,0x0000000,
	0x0000000,0x0000000,0x0000400,0x3ff0000,0x03fffff,
};

#define p256_mp_mod ((sp_digit)0x000001)

/* Write r as big endian to byte aray.
 * Fixed length number of bytes written: 32
 *
 * r  A single precision integer.
 * a  Byte array.
 */
static void sp_256_to_bin(sp_digit* r, uint8_t* a)
{
	int i, j, s = 0, b;

	for (i = 0; i < 9; i++) {
		r[i+1] += r[i] >> 26;
		r[i] &= 0x3ffffff;
	}
	j = 256 / 8 - 1;
	a[j] = 0;
	for (i = 0; i < 10 && j >= 0; i++) {
		b = 0;
		a[j--] |= r[i] << s; b += 8 - s;
		if (j < 0)
			break;
		while (b < 26) {
			a[j--] = r[i] >> b; b += 8;
			if (j < 0)
				break;
		}
		s = 8 - (b - 26);
		if (j >= 0)
			a[j] = 0;
		if (s != 0)
			j++;
	}
}

/* Read big endian unsigned byte aray into r.
 *
 * r  A single precision integer.
 * a  Byte array.
 * n  Number of bytes in array to read.
 */
static void sp_256_from_bin(sp_digit* r, int max, const uint8_t* a, int n)
{
	int i, j = 0, s = 0;

	r[0] = 0;
	for (i = n-1; i >= 0; i--) {
		r[j] |= ((sp_digit)a[i]) << s;
		if (s >= 18) {
			r[j] &= 0x3ffffff;
			s = 26 - s;
			if (j + 1 >= max)
				break;
			r[++j] = a[i] >> s;
			s = 8 - s;
		}
		else
			s += 8;
	}

	for (j++; j < max; j++)
		r[j] = 0;
}

/* Convert a point of big-endian 32-byte x,y pair to type sp_point. */
static void sp_256_point_from_bin2x32(sp_point* p, const uint8_t *bin2x32)
{
	memset(p, 0, sizeof(*p));
	/*p->infinity = 0;*/
	sp_256_from_bin(p->x, 2 * 10, bin2x32, 32);
	sp_256_from_bin(p->y, 2 * 10, bin2x32 + 32, 32);
	//static const uint8_t one[1] = { 1 };
	//sp_256_from_bin(p->z, 2 * 10, one, 1);
	p->z[0] = 1;
}

/* Compare a with b.
 *
 * return -ve, 0 or +ve if a is less than, equal to or greater than b
 * respectively.
 */
static sp_digit sp_256_cmp_10(const sp_digit* a, const sp_digit* b)
{
	sp_digit r;
	int i;
	for (i = 9; i >= 0; i--) {
		r = a[i] - b[i];
		if (r != 0)
			break;
	}
	return r;
}

/* Compare two numbers to determine if they are equal.
 *
 * return 1 when equal and 0 otherwise.
 */
static int sp_256_cmp_equal_10(const sp_digit* a, const sp_digit* b)
{
	return sp_256_cmp_10(a, b) == 0;
}

/* Normalize the values in each word to 26 bits. */
static void sp_256_norm_10(sp_digit* a)
{
	int i;
	for (i = 0; i < 9; i++) {
		a[i+1] += a[i] >> 26;
		a[i] &= 0x3ffffff;
	}
}

/* Add b to a into r. (r = a + b) */
static void sp_256_add_10(sp_digit* r, const sp_digit* a, const sp_digit* b)
{
	int i;
	for (i = 0; i < 10; i++)
		r[i] = a[i] + b[i];
}

/* Sub b from a into r. (r = a - b) */
static void sp_256_sub_10(sp_digit* r, const sp_digit* a, const sp_digit* b)
{
	int i;
	for (i = 0; i < 10; i++)
		r[i] = a[i] - b[i];
}

/* Shift number left one bit. Bottom bit is lost. */
static void sp_256_rshift1_10(sp_digit* r, sp_digit* a)
{
	int i;
	for (i = 0; i < 9; i++)
		r[i] = ((a[i] >> 1) | (a[i + 1] << 25)) & 0x3ffffff;
	r[9] = a[9] >> 1;
}

/* Mul a by scalar b and add into r. (r += a * b) */
static void sp_256_mul_add_10(sp_digit* r, const sp_digit* a, sp_digit b)
{
	int64_t tb = b;
	int64_t t = 0;
	int i;

	for (i = 0; i < 10; i++) {
		t += (tb * a[i]) + r[i];
		r[i] = t & 0x3ffffff;
		t >>= 26;
	}
	r[10] += t;
}

/* Multiply a and b into r. (r = a * b) */
static void sp_256_mul_10(sp_digit* r, const sp_digit* a, const sp_digit* b)
{
	int i, j, k;
	int64_t c;

	c = ((int64_t)a[9]) * b[9];
	r[19] = (sp_digit)(c >> 26);
	c = (c & 0x3ffffff) << 26;
	for (k = 17; k >= 0; k--) {
		for (i = 9; i >= 0; i--) {
			j = k - i;
			if (j >= 10)
				break;
			if (j < 0)
				continue;
			c += ((int64_t)a[i]) * b[j];
		}
		r[k + 2] += c >> 52;
		r[k + 1] = (c >> 26) & 0x3ffffff;
		c = (c & 0x3ffffff) << 26;
	}
	r[0] = (sp_digit)(c >> 26);
}

/* Square a and put result in r. (r = a * a) */
static void sp_256_sqr_10(sp_digit* r, const sp_digit* a)
{
	int i, j, k;
	int64_t c;

	c = ((int64_t)a[9]) * a[9];
	r[19] = (sp_digit)(c >> 26);
	c = (c & 0x3ffffff) << 26;
	for (k = 17; k >= 0; k--) {
		for (i = 9; i >= 0; i--) {
			j = k - i;
			if (j >= 10 || i <= j)
				break;
			if (j < 0)
				continue;
			c += ((int64_t)a[i]) * a[j] * 2;
		}
		if (i == j)
			c += ((int64_t)a[i]) * a[i];
		r[k + 2] += c >> 52;
		r[k + 1] = (c >> 26) & 0x3ffffff;
		c = (c & 0x3ffffff) << 26;
	}
	r[0] = (sp_digit)(c >> 26);
}

/* Divide the number by 2 mod the modulus (prime). (r = a / 2 % m) */
static void sp_256_div2_10(sp_digit* r, const sp_digit* a, const sp_digit* m)
{
	if (a[0] & 1)
		sp_256_add_10(r, a, m);
	sp_256_norm_10(r);
	sp_256_rshift1_10(r, r);
}

/* Add two Montgomery form numbers (r = a + b % m) */
static void sp_256_mont_add_10(sp_digit* r, const sp_digit* a, const sp_digit* b,
		const sp_digit* m)
{
	sp_256_add_10(r, a, b);
	sp_256_norm_10(r);
	if ((r[9] >> 22) > 0)
		sp_256_sub_10(r, r, m);
	sp_256_norm_10(r);
}

/* Subtract two Montgomery form numbers (r = a - b % m) */
static void sp_256_mont_sub_10(sp_digit* r, const sp_digit* a, const sp_digit* b,
		const sp_digit* m)
{
	sp_256_sub_10(r, a, b);
	if (r[9] >> 22)
		sp_256_add_10(r, r, m);
	sp_256_norm_10(r);
}

/* Double a Montgomery form number (r = a + a % m) */
static void sp_256_mont_dbl_10(sp_digit* r, const sp_digit* a, const sp_digit* m)
{
	sp_256_add_10(r, a, a);
	sp_256_norm_10(r);
	if ((r[9] >> 22) > 0)
		sp_256_sub_10(r, r, m);
	sp_256_norm_10(r);
}

/* Triple a Montgomery form number (r = a + a + a % m) */
static void sp_256_mont_tpl_10(sp_digit* r, const sp_digit* a, const sp_digit* m)
{
	sp_256_add_10(r, a, a);
	sp_256_norm_10(r);
	if ((r[9] >> 22) > 0)
		sp_256_sub_10(r, r, m);
	sp_256_norm_10(r);
	sp_256_add_10(r, r, a);
	sp_256_norm_10(r);
	if ((r[9] >> 22) > 0)
		sp_256_sub_10(r, r, m);
	sp_256_norm_10(r);
}

/* Shift the result in the high 256 bits down to the bottom. */
static void sp_256_mont_shift_10(sp_digit* r, const sp_digit* a)
{
	int i;
	sp_digit n, s;

	s = a[10];
	n = a[9] >> 22;
	for (i = 0; i < 9; i++) {
		n += (s & 0x3ffffff) << 4;
		r[i] = n & 0x3ffffff;
		n >>= 26;
		s = a[11 + i] + (s >> 26);
	}
	n += s << 4;
	r[9] = n;
	memset(&r[10], 0, sizeof(*r) * 10);
}

/* Reduce the number back to 256 bits using Montgomery reduction.
 *
 * a   A single precision number to reduce in place.
 * m   The single precision number representing the modulus.
 * mp  The digit representing the negative inverse of m mod 2^n.
 */
static void sp_256_mont_reduce_10(sp_digit* a, const sp_digit* m, sp_digit mp)
{
	int i;
	sp_digit mu;

	if (mp != 1) {
		for (i = 0; i < 9; i++) {
			mu = (a[i] * mp) & 0x3ffffff;
			sp_256_mul_add_10(a+i, m, mu);
			a[i+1] += a[i] >> 26;
		}
		mu = (a[i] * mp) & 0x3fffffl;
		sp_256_mul_add_10(a+i, m, mu);
		a[i+1] += a[i] >> 26;
		a[i] &= 0x3ffffff;
	}
	else {
		for (i = 0; i < 9; i++) {
			mu = a[i] & 0x3ffffff;
			sp_256_mul_add_10(a+i, p256_mod, mu);
			a[i+1] += a[i] >> 26;
		}
		mu = a[i] & 0x3fffffl;
		sp_256_mul_add_10(a+i, p256_mod, mu);
		a[i+1] += a[i] >> 26;
		a[i] &= 0x3ffffff;
	}

	sp_256_mont_shift_10(a, a);
	if ((a[9] >> 22) > 0)
		sp_256_sub_10(a, a, m);
	sp_256_norm_10(a);
}

/* Multiply two Montogmery form numbers mod the modulus (prime).
 * (r = a * b mod m)
 *
 * r   Result of multiplication.
 * a   First number to multiply in Montogmery form.
 * b   Second number to multiply in Montogmery form.
 * m   Modulus (prime).
 * mp  Montogmery mulitplier.
 */
static void sp_256_mont_mul_10(sp_digit* r, const sp_digit* a, const sp_digit* b,
		const sp_digit* m, sp_digit mp)
{
	sp_256_mul_10(r, a, b);
	sp_256_mont_reduce_10(r, m, mp);
}

/* Square the Montgomery form number. (r = a * a mod m)
 *
 * r   Result of squaring.
 * a   Number to square in Montogmery form.
 * m   Modulus (prime).
 * mp  Montogmery mulitplier.
 */
static void sp_256_mont_sqr_10(sp_digit* r, const sp_digit* a, const sp_digit* m,
		sp_digit mp)
{
	sp_256_sqr_10(r, a);
	sp_256_mont_reduce_10(r, m, mp);
}

/* Invert the number, in Montgomery form, modulo the modulus (prime) of the
 * P256 curve. (r = 1 / a mod m)
 *
 * r   Inverse result.
 * a   Number to invert.
 */
#if 0
/* Mod-2 for the P256 curve. */
static const uint32_t p256_mod_2[8] = {
	0xfffffffd,0xffffffff,0xffffffff,0x00000000,
	0x00000000,0x00000000,0x00000001,0xffffffff,
};
//Bit pattern:
//2    2         2         2         2         2         2         1...1
//5    5         4         3         2         1         0         9...0         9...1
//543210987654321098765432109876543210987654321098765432109876543210...09876543210...09876543210
//111111111111111111111111111111110000000000000000000000000000000100...00000111111...11111111101
#endif
static void sp_256_mont_inv_10(sp_digit* r, sp_digit* a)
{
	sp_digit t[2*10]; //can be just [10]?
	int i;

	memcpy(t, a, sizeof(sp_digit) * 10);
	for (i = 254; i >= 0; i--) {
		sp_256_mont_sqr_10(t, t, p256_mod, p256_mp_mod);
		/*if (p256_mod_2[i / 32] & ((sp_digit)1 << (i % 32)))*/
		if (i >= 224 || i == 192 || (i <= 95 && i != 1))
			sp_256_mont_mul_10(t, t, a, p256_mod, p256_mp_mod);
	}
	memcpy(r, t, sizeof(sp_digit) * 10);
}

/* Multiply a number by Montogmery normalizer mod modulus (prime).
 *
 * r  The resulting Montgomery form number.
 * a  The number to convert.
 */
static void sp_256_mod_mul_norm_10(sp_digit* r, const sp_digit* a)
{
	int64_t t[8];
	int64_t o;
	uint32_t a32;

	/*  1  1  0 -1 -1 -1 -1  0 */
	/*  0  1  1  0 -1 -1 -1 -1 */
	/*  0  0  1  1  0 -1 -1 -1 */
	/* -1 -1  0  2  2  1  0 -1 */
	/*  0 -1 -1  0  2  2  1  0 */
	/*  0  0 -1 -1  0  2  2  1 */
	/* -1 -1  0  0  0  1  3  2 */
	/*  1  0 -1 -1 -1 -1  0  3 */
	// t[] should be calculated from "a" (converted from 26-bit to 32-bit vector a32[8])
	// according to the above matrix:
	//t[0] = 0 + a32[0] + a32[1]            - a32[3]   - a32[4]   - a32[5]   - a32[6]             ;
	//t[1] = 0          + a32[1] + a32[2]              - a32[4]   - a32[5]   - a32[6]   - a32[7]  ;
	//t[2] = 0                   + a32[2]   + a32[3]              - a32[5]   - a32[6]   - a32[7]  ;
	//t[3] = 0 - a32[0] - a32[1]            + 2*a32[3] + 2*a32[4] + a32[5]              - a32[7]  ;
	//t[4] = 0          - a32[1] - a32[2]              + 2*a32[4] + 2*a32[5] + a32[6]             ;
	//t[5] = 0                   - a32[2]   - a32[3]              + 2*a32[5] + 2*a32[6] + a32[7]  ;
	//t[6] = 0 - a32[0] - a32[1]                                  + a32[5]   + 3*a32[6] + 2*a32[7];
	//t[7] = 0 + a32[0]          - a32[2]   - a32[3]   - a32[4]   - a32[5]              + 3*a32[7];
	// We can do it "piecemeal" after each a32[i] is known, no need to store entire a32[8] vector:

#define A32 (int64_t)a32
	a32 = a[0] | (a[1] << 26);
	t[0] = 0 + A32;
	t[3] = 0 - A32;
	t[6] = 0 - A32;
	t[7] = 0 + A32;

	a32 = (a[1] >> 6) | (a[2] << 20);
	t[0] += A32    ;
	t[1]  = 0 + A32;
	t[3] -= A32    ;
	t[4]  = 0 - A32;
	t[6] -= A32    ;

	a32 = (a[2] >> 12) | (a[3] << 14);
	t[1] += A32    ;
	t[2]  = 0 + A32;
	t[4] -= A32    ;
	t[5]  = 0 - A32;
	t[7] -= A32    ;

	a32 = (a[3] >> 18) | (a[4] << 8);
	t[0] -= A32  ;
	t[2] += A32  ;
	t[3] += 2*A32;
	t[5] -= A32  ;
	t[7] -= A32  ;

	a32 = (a[4] >> 24) | (a[5] << 2) | (a[6] << 28);
	t[0] -= A32  ;
	t[1] -= A32  ;
	t[3] += 2*A32;
	t[4] += 2*A32;
	t[7] -= A32  ;

	a32 = (a[6] >> 4) | (a[7] << 22);
	t[0] -= A32  ;
	t[1] -= A32  ;
	t[2] -= A32  ;
	t[3] += A32  ;
	t[4] += 2*A32;
	t[5] += 2*A32;
	t[6] += A32  ;
	t[7] -= A32  ;

	a32 = (a[7] >> 10) | (a[8] << 16);
	t[0] -= A32  ;
	t[1] -= A32  ;
	t[2] -= A32  ;
	t[4] += A32  ;
	t[5] += 2*A32;
	t[6] += 3*A32;

	a32 = (a[8] >> 16) | (a[9] << 10);
	t[1] -= A32  ;
	t[2] -= A32  ;
	t[3] -= A32  ;
	t[5] += A32  ;
	t[6] += 2*A32;
	t[7] += 3*A32;
#undef A32

	t[1] += t[0] >> 32; t[0] &= 0xffffffff;
	t[2] += t[1] >> 32; t[1] &= 0xffffffff;
	t[3] += t[2] >> 32; t[2] &= 0xffffffff;
	t[4] += t[3] >> 32; t[3] &= 0xffffffff;
	t[5] += t[4] >> 32; t[4] &= 0xffffffff;
	t[6] += t[5] >> 32; t[5] &= 0xffffffff;
	t[7] += t[6] >> 32; t[6] &= 0xffffffff;
	o     = t[7] >> 32; t[7] &= 0xffffffff;
	t[0] += o;
	t[3] -= o;
	t[6] -= o;
	t[7] += o;
	t[1] += t[0] >> 32; //t[0] &= 0xffffffff;
	t[2] += t[1] >> 32; //t[1] &= 0xffffffff;
	t[3] += t[2] >> 32; //t[2] &= 0xffffffff;
	t[4] += t[3] >> 32; //t[3] &= 0xffffffff;
	t[5] += t[4] >> 32; //t[4] &= 0xffffffff;
	t[6] += t[5] >> 32; //t[5] &= 0xffffffff;
	t[7] += t[6] >> 32; //t[6] &= 0xffffffff; - (uint32_t)t[i] casts below accomplish masking

	r[0] = 0x3ffffff & ((sp_digit)((uint32_t)t[0]));
	r[1] = 0x3ffffff & ((sp_digit)((uint32_t)t[0] >> 26) | ((sp_digit)t[1] <<  6));
	r[2] = 0x3ffffff & ((sp_digit)((uint32_t)t[1] >> 20) | ((sp_digit)t[2] << 12));
	r[3] = 0x3ffffff & ((sp_digit)((uint32_t)t[2] >> 14) | ((sp_digit)t[3] << 18));
	r[4] = 0x3ffffff & ((sp_digit)((uint32_t)t[3] >>  8) | ((sp_digit)t[4] << 24));
	r[5] = 0x3ffffff & ((sp_digit)((uint32_t)t[4] >>  2));
	r[6] = 0x3ffffff & ((sp_digit)((uint32_t)t[4] >> 28) | ((sp_digit)t[5] <<  4));
	r[7] = 0x3ffffff & ((sp_digit)((uint32_t)t[5] >> 22) | ((sp_digit)t[6] << 10));
	r[8] = 0x3ffffff & ((sp_digit)((uint32_t)t[6] >> 16) | ((sp_digit)t[7] << 16));
	r[9] =             ((sp_digit)((uint32_t)t[7] >> 10));
}

/* Map the Montgomery form projective co-ordinate point to an affine point.
 *
 * r  Resulting affine co-ordinate point.
 * p  Montgomery form projective co-ordinate point.
 */
static void sp_256_map_10(sp_point* r, sp_point* p)
{
	sp_digit t1[2*10];
	sp_digit t2[2*10];

	sp_256_mont_inv_10(t1, p->z);

	sp_256_mont_sqr_10(t2, t1, p256_mod, p256_mp_mod);
	sp_256_mont_mul_10(t1, t2, t1, p256_mod, p256_mp_mod);

	/* x /= z^2 */
	sp_256_mont_mul_10(r->x, p->x, t2, p256_mod, p256_mp_mod);
	memset(r->x + 10, 0, sizeof(r->x) / 2);
	sp_256_mont_reduce_10(r->x, p256_mod, p256_mp_mod);
	/* Reduce x to less than modulus */
	if (sp_256_cmp_10(r->x, p256_mod) >= 0)
		sp_256_sub_10(r->x, r->x, p256_mod);
	sp_256_norm_10(r->x);

	/* y /= z^3 */
	sp_256_mont_mul_10(r->y, p->y, t1, p256_mod, p256_mp_mod);
	memset(r->y + 10, 0, sizeof(r->y) / 2);
	sp_256_mont_reduce_10(r->y, p256_mod, p256_mp_mod);
	/* Reduce y to less than modulus */
	if (sp_256_cmp_10(r->y, p256_mod) >= 0)
		sp_256_sub_10(r->y, r->y, p256_mod);
	sp_256_norm_10(r->y);

	memset(r->z, 0, sizeof(r->z));
	r->z[0] = 1;
}

/* Double the Montgomery form projective point p.
 *
 * r  Result of doubling point.
 * p  Point to double.
 */
static void sp_256_proj_point_dbl_10(sp_point* r, sp_point* p)
{
	sp_point tp;
	sp_digit t1[2*10];
	sp_digit t2[2*10];

	/* Put point to double into result */
	if (r != p)
		*r = *p; /* struct copy */

	if (r->infinity) {
		/* If infinity, don't double (work on dummy value) */
		r = &tp;
	}
	/* T1 = Z * Z */
	sp_256_mont_sqr_10(t1, r->z, p256_mod, p256_mp_mod);
	/* Z = Y * Z */
	sp_256_mont_mul_10(r->z, r->y, r->z, p256_mod, p256_mp_mod);
	/* Z = 2Z */
	sp_256_mont_dbl_10(r->z, r->z, p256_mod);
	/* T2 = X - T1 */
	sp_256_mont_sub_10(t2, r->x, t1, p256_mod);
	/* T1 = X + T1 */
	sp_256_mont_add_10(t1, r->x, t1, p256_mod);
	/* T2 = T1 * T2 */
	sp_256_mont_mul_10(t2, t1, t2, p256_mod, p256_mp_mod);
	/* T1 = 3T2 */
	sp_256_mont_tpl_10(t1, t2, p256_mod);
	/* Y = 2Y */
	sp_256_mont_dbl_10(r->y, r->y, p256_mod);
	/* Y = Y * Y */
	sp_256_mont_sqr_10(r->y, r->y, p256_mod, p256_mp_mod);
	/* T2 = Y * Y */
	sp_256_mont_sqr_10(t2, r->y, p256_mod, p256_mp_mod);
	/* T2 = T2/2 */
	sp_256_div2_10(t2, t2, p256_mod);
	/* Y = Y * X */
	sp_256_mont_mul_10(r->y, r->y, r->x, p256_mod, p256_mp_mod);
	/* X = T1 * T1 */
	sp_256_mont_mul_10(r->x, t1, t1, p256_mod, p256_mp_mod);
	/* X = X - Y */
	sp_256_mont_sub_10(r->x, r->x, r->y, p256_mod);
	/* X = X - Y */
	sp_256_mont_sub_10(r->x, r->x, r->y, p256_mod);
	/* Y = Y - X */
	sp_256_mont_sub_10(r->y, r->y, r->x, p256_mod);
	/* Y = Y * T1 */
	sp_256_mont_mul_10(r->y, r->y, t1, p256_mod, p256_mp_mod);
	/* Y = Y - T2 */
	sp_256_mont_sub_10(r->y, r->y, t2, p256_mod);
}

/* Add two Montgomery form projective points.
 *
 * r  Result of addition.
 * p  Frist point to add.
 * q  Second point to add.
 */
static void sp_256_proj_point_add_10(sp_point* r, sp_point* p, sp_point* q)
{
	sp_digit t1[2*10];
	sp_digit t2[2*10];
	sp_digit t3[2*10];
	sp_digit t4[2*10];
	sp_digit t5[2*10];

	/* Ensure only the first point is the same as the result. */
	if (q == r) {
		sp_point* a = p;
		p = q;
		q = a;
	}

	/* Check double */
	sp_256_sub_10(t1, p256_mod, q->y);
	sp_256_norm_10(t1);
	if (sp_256_cmp_equal_10(p->x, q->x)
	 && sp_256_cmp_equal_10(p->z, q->z)
	 && (sp_256_cmp_equal_10(p->y, q->y) || sp_256_cmp_equal_10(p->y, t1))
	) {
		sp_256_proj_point_dbl_10(r, p);
	}
	else {
		sp_point tp;
		sp_point *v;

		v = r;
		if (p->infinity | q->infinity) {
			memset(&tp, 0, sizeof(tp));
			v = &tp;
		}

		*r = p->infinity ? *q : *p; /* struct copy */

		/* U1 = X1*Z2^2 */
		sp_256_mont_sqr_10(t1, q->z, p256_mod, p256_mp_mod);
		sp_256_mont_mul_10(t3, t1, q->z, p256_mod, p256_mp_mod);
		sp_256_mont_mul_10(t1, t1, v->x, p256_mod, p256_mp_mod);
		/* U2 = X2*Z1^2 */
		sp_256_mont_sqr_10(t2, v->z, p256_mod, p256_mp_mod);
		sp_256_mont_mul_10(t4, t2, v->z, p256_mod, p256_mp_mod);
		sp_256_mont_mul_10(t2, t2, q->x, p256_mod, p256_mp_mod);
		/* S1 = Y1*Z2^3 */
		sp_256_mont_mul_10(t3, t3, v->y, p256_mod, p256_mp_mod);
		/* S2 = Y2*Z1^3 */
		sp_256_mont_mul_10(t4, t4, q->y, p256_mod, p256_mp_mod);
		/* H = U2 - U1 */
		sp_256_mont_sub_10(t2, t2, t1, p256_mod);
		/* R = S2 - S1 */
		sp_256_mont_sub_10(t4, t4, t3, p256_mod);
		/* Z3 = H*Z1*Z2 */
		sp_256_mont_mul_10(v->z, v->z, q->z, p256_mod, p256_mp_mod);
		sp_256_mont_mul_10(v->z, v->z, t2, p256_mod, p256_mp_mod);
		/* X3 = R^2 - H^3 - 2*U1*H^2 */
		sp_256_mont_sqr_10(v->x, t4, p256_mod, p256_mp_mod);
		sp_256_mont_sqr_10(t5, t2, p256_mod, p256_mp_mod);
		sp_256_mont_mul_10(v->y, t1, t5, p256_mod, p256_mp_mod);
		sp_256_mont_mul_10(t5, t5, t2, p256_mod, p256_mp_mod);
		sp_256_mont_sub_10(v->x, v->x, t5, p256_mod);
		sp_256_mont_dbl_10(t1, v->y, p256_mod);
		sp_256_mont_sub_10(v->x, v->x, t1, p256_mod);
		/* Y3 = R*(U1*H^2 - X3) - S1*H^3 */
		sp_256_mont_sub_10(v->y, v->y, v->x, p256_mod);
		sp_256_mont_mul_10(v->y, v->y, t4, p256_mod, p256_mp_mod);
		sp_256_mont_mul_10(t5, t5, t3, p256_mod, p256_mp_mod);
		sp_256_mont_sub_10(v->y, v->y, t5, p256_mod);
	}
}

/* Multiply the point by the scalar and return the result.
 * If map is true then convert result to affine co-ordinates.
 *
 * r     Resulting point.
 * g     Point to multiply.
 * k     Scalar to multiply by.
 * map   Indicates whether to convert result to affine.
 */
static void sp_256_ecc_mulmod_10(sp_point* r, const sp_point* g, const sp_digit* k /*, int map*/)
{
	enum { map = 1 }; /* we always convert result to affine coordinates */
	sp_point t[3];
	sp_digit n;
	int i;
	int c, y;

	memset(t, 0, sizeof(t));

	/* t[0] = {0, 0, 1} * norm */
	t[0].infinity = 1;
	/* t[1] = {g->x, g->y, g->z} * norm */
	sp_256_mod_mul_norm_10(t[1].x, g->x);
	sp_256_mod_mul_norm_10(t[1].y, g->y);
	sp_256_mod_mul_norm_10(t[1].z, g->z);

	i = 9;
	c = 22;
	n = k[i--] << (26 - c);
	for (; ; c--) {
		if (c == 0) {
			if (i == -1)
				break;

			n = k[i--];
			c = 26;
		}

		y = (n >> 25) & 1;
		n <<= 1;

		sp_256_proj_point_add_10(&t[y^1], &t[0], &t[1]);
		memcpy(&t[2], &t[y], sizeof(sp_point));
		sp_256_proj_point_dbl_10(&t[2], &t[2]);
		memcpy(&t[y], &t[2], sizeof(sp_point));
	}

	if (map)
		sp_256_map_10(r, &t[0]);
	else
		memcpy(r, &t[0], sizeof(sp_point));

	memset(t, 0, sizeof(t)); //paranoia
}

/* Multiply the base point of P256 by the scalar and return the result.
 * If map is true then convert result to affine co-ordinates.
 *
 * r     Resulting point.
 * k     Scalar to multiply by.
 * map   Indicates whether to convert result to affine.
 */
static void sp_256_ecc_mulmod_base_10(sp_point* r, sp_digit* k /*, int map*/)
{
	/* Since this function is called only once, save space:
	 * don't have "static const sp_point p256_base = {...}",
	 * it would have more zeros than data.
	 */
	static const uint8_t p256_base_bin[] = {
		/* x (big-endian) */
		0x6b,0x17,0xd1,0xf2,0xe1,0x2c,0x42,0x47,0xf8,0xbc,0xe6,0xe5,0x63,0xa4,0x40,0xf2,0x77,0x03,0x7d,0x81,0x2d,0xeb,0x33,0xa0,0xf4,0xa1,0x39,0x45,0xd8,0x98,0xc2,0x96,
		/* y */
		0x4f,0xe3,0x42,0xe2,0xfe,0x1a,0x7f,0x9b,0x8e,0xe7,0xeb,0x4a,0x7c,0x0f,0x9e,0x16,0x2b,0xce,0x33,0x57,0x6b,0x31,0x5e,0xce,0xcb,0xb6,0x40,0x68,0x37,0xbf,0x51,0xf5,
		/* z will be set to 1, infinity flag to "false" */
	};
	sp_point p256_base;

	sp_256_point_from_bin2x32(&p256_base, p256_base_bin);

	sp_256_ecc_mulmod_10(r, &p256_base, k /*, map*/);
}

/* Multiply the point by the scalar and serialize the X ordinate.
 * The number is 0 padded to maximum size on output.
 *
 * priv    Scalar to multiply the point by.
 * pub2x32 Point to multiply.
 * out32   Buffer to hold X ordinate.
 */
static void sp_ecc_secret_gen_256(const sp_digit priv[10], const uint8_t *pub2x32, uint8_t* out32)
{
	sp_point point[1];

#if FIXED_PEER_PUBKEY
	memset((void*)pub2x32, 0x55, 64);
#endif
	dump_hex("peerkey %s\n", pub2x32, 32); /* in TLS, this is peer's public key */
	dump_hex("        %s\n", pub2x32 + 32, 32);

	sp_256_point_from_bin2x32(point, pub2x32);
	dump_hex("point->x %s\n", point->x, sizeof(point->x));
	dump_hex("point->y %s\n", point->y, sizeof(point->y));

	sp_256_ecc_mulmod_10(point, point, priv);

	sp_256_to_bin(point->x, out32);
	dump_hex("out32: %s\n", out32, 32);
}

/* Generates a scalar that is in the range 1..order-1. */
#define SIMPLIFY 1
/* Add 1 to a. (a = a + 1) */
static void sp_256_add_one_10(sp_digit* a)
{
	a[0]++;
	sp_256_norm_10(a);
}
static void sp_256_ecc_gen_k_10(sp_digit k[10])
{
#if !SIMPLIFY
	/* The order of the curve P256 minus 2. */
	static const sp_digit p256_order2[10] = {
		0x063254f,0x272b0bf,0x1e84f3b,0x2b69c5e,0x3bce6fa,
		0x3ffffff,0x3ffffff,0x00003ff,0x3ff0000,0x03fffff,
	};
#endif
	uint8_t buf[32];

	for (;;) {
		tls_get_random(buf, sizeof(buf));
#if FIXED_SECRET
		memset(buf, 0x77, sizeof(buf));
#endif
		sp_256_from_bin(k, 10, buf, sizeof(buf));
#if !SIMPLIFY
		if (sp_256_cmp_10(k, p256_order2) < 0)
			break;
#else
		/* non-loopy version (and not needing p256_order2[]):
		 * if most-significant word seems that k can be larger
		 * than p256_order2, fix it up:
		 */
		if (k[9] >= 0x03fffff)
			k[9] = 0x03ffffe;
		break;
#endif
	}
	sp_256_add_one_10(k);
#undef SIMPLIFY
}

/* Makes a random EC key pair. */
static void sp_ecc_make_key_256(sp_digit privkey[10], uint8_t *pubkey)
{
	sp_point point[1];

	sp_256_ecc_gen_k_10(privkey);
	sp_256_ecc_mulmod_base_10(point, privkey);
	sp_256_to_bin(point->x, pubkey);
	sp_256_to_bin(point->y, pubkey + 32);

	memset(point, 0, sizeof(point)); //paranoia
}

void FAST_FUNC curve_P256_compute_pubkey_and_premaster(
		uint8_t *pubkey2x32, uint8_t *premaster32,
		const uint8_t *peerkey2x32)
{
	sp_digit privkey[10];

	sp_ecc_make_key_256(privkey, pubkey2x32);
	dump_hex("pubkey: %s\n", pubkey2x32, 32);
	dump_hex("        %s\n", pubkey2x32 + 32, 32);

	/* Combine our privkey and peer's public key to generate premaster */
	sp_ecc_secret_gen_256(privkey, /*x,y:*/peerkey2x32, premaster32);
	dump_hex("premaster: %s\n", premaster32, 32);
}
