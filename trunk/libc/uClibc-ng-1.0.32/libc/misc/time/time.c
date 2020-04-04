/* Copyright (C) 2002-2004   Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

/* June 15, 2002     Initial Notes:
 *
 * Note: It is assumed throught that time_t is either long or unsigned long.
 *       Similarly, clock_t is assumed to be long int.
 *
 * Warning: Assumptions are made about the layout of struct tm!  It is
 *    assumed that the initial fields of struct tm are (in order):
 *    tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year, tm_wday, tm_yday
 *
 * Reached the inital goal of supporting the ANSI/ISO C99 time functions
 * as well as SUSv3's strptime.  All timezone info is obtained from the
 * TZ env variable.
 *
 * Differences from glibc worth noting:
 *
 * Leap seconds are not considered here.
 *
 * glibc stores additional timezone info the struct tm, whereas we don't.
 *
 * Alternate digits and era handling are not currently implemented.
 * The modifiers are accepted, and tested for validity with the following
 * specifier, but are ignored otherwise.
 *
 * strftime does not implement glibc extension modifiers or widths for
 *     conversion specifiers.  However it does implement the glibc
 *     extension specifiers %l, %k, and %s.  It also recognizes %P, but
 *     treats it as a synonym for %p; i.e. doesn't convert to lower case.
 *
 * strptime implements the glibc extension specifiers.  However, it follows
 *     SUSv3 in requiring at least one non-alphanumeric char between
 *     conversion specifiers.  Also, strptime only sets struct tm fields
 *     for which format specifiers appear and does not try to infer other
 *     fields (such as wday) as glibc's version does.
 *
 * TODO - Since glibc's %l and %k can space-pad their output in strftime,
 *     it might be reasonable to eat whitespace first for those specifiers.
 *     This could be done by pushing " %I" and " %H" respectively so that
 *     leading whitespace is consumed.  This is really only an issue if %l
 *     or %k occurs at the start of the format string.
 *
 * TODO - Implement getdate? tzfile? struct tm extensions?
 *
 * TODO - Rework _time_mktime to remove the dependency on long long.
 */

/* Oct 28, 2002
 *
 * Fixed allowed char check for std and dst TZ fields.
 *
 * Added several options concerned with timezone support.  The names will
 * probably change once Erik gets the new config system in place.
 *
 * Defining __TIME_TZ_FILE causes tzset() to attempt to read the TZ value
 * from the file /etc/TZ if the TZ env variable isn't set.  The file contents
 * must be the intended value of TZ, followed by a newline.  No other chars,
 * spacing, etc is allowed.  As an example, an easy way for me to init
 * /etc/TZ appropriately would be:    echo CST6CDT > /etc/TZ
 *
 * Defining __TIME_TZ_FILE_ONCE will cause all further accesses of /etc/TZ
 * to be skipped once a legal value has been read.
 *
 * Defining __TIME_TZ_OPT_SPEED will cause a tzset() to keep a copy of the
 * last TZ setting string and do a "fast out" if the current string is the
 * same.
 *
 * Nov 21, 2002   Fix an error return case in _time_mktime.
 *
 * Nov 26, 2002   Fix bug in setting daylight and timezone when no (valid) TZ.
 *   Bug reported by Arne Bernin <arne@alamut.de> in regards to freeswan.
 *
 * July 27, 2003  Adjust the struct tm extension field support.
 *   Change __tm_zone back to a ptr and add the __tm_tzname[] buffer for
 *   __tm_zone to point to.  This gets around complaints from g++.
 *  Who knows... it might even fix the PPC timezone init problem.
 *
 * July 29, 2003  Fix a bug in mktime behavior when tm_isdst was -1.
 *   Bug reported by "Sid Wade" <sid@vivato.net> in regards to busybox.
 *
 *   NOTE: uClibc mktime behavior is different than glibc's when
 *   the struct tm has tm_isdst == -1 and also had fields outside of
 *   the normal ranges.
 *
 *   Apparently, glibc examines (at least) tm_sec and guesses the app's
 *   intention of assuming increasing or decreasing time when entering an
 *   ambiguous time period at the dst<->st boundaries.
 *
 *   The uClibc behavior is to always normalize the struct tm and then
 *   try to determing the dst setting.
 *
 *   As long as tm_isdst != -1 or the time specifiec by struct tm is
 *   unambiguous (not falling in the dst<->st transition region) both
 *   uClibc and glibc should produce the same result for mktime.
 *
 * Oct 31, 2003 Kill the seperate __tm_zone and __tm_tzname[] and which
 *   doesn't work if you want the memcpy the struct.  Sigh... I didn't
 *   think about that.  So now, when the extensions are enabled, we
 *   malloc space when necessary and keep the timezone names in a linked
 *   list.
 *
 *   Fix a dst-related bug which resulted in use of uninitialized data.
 *
 * Nov 15, 2003 I forgot to update the thread locking in the last dst fix.
 *
 * Dec 14, 2003 Fix some dst issues in _time_mktime().
 *   Normalize the tm_isdst value to -1, 0, or 1.
 *   If no dst for this timezone, then reset tm_isdst to 0.
 *
 * May 7, 2004
 *   Change clock() to allow wrapping.
 *   Add timegm() function.
 *   Make lookup_tzname() static (as it should have been).
 *   Have strftime() get timezone information from the passed struct
 *     for the %z and %Z conversions when using struct tm extensions.
 *
 * Jul 24, 2004
 *   Fix 2 bugs in strftime related to glibc struct tm extensions.
 *   1) Need to negate tm_gmtoff field value when used. (bug 336).
 *   2) Deal with NULL ptr case for tm_zone field, which was causing
 *      segfaults in both the NIST/PCTS tests and the Python 2.4.1
 *      self-test suite.
 *      NOTE: We set uninitialized timezone names to "???", and this
 *            differs (intentionally) from glibc's behavior.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <langinfo.h>
#include <locale.h>
#include <fcntl.h>
#include <unistd.h>
#include <bits/uClibc_uintmaxtostr.h>
#include <bits/uClibc_mutex.h>

#if defined __UCLIBC_HAS_WCHAR__ && (defined L_wcsftime || defined L_wcsftime_l)
#include <wchar.h>
# define CHAR_T wchar_t
# define UCHAR_T unsigned int
# ifdef L_wcsftime
#  define strftime wcsftime
#  define L_strftime
#  if defined(__UCLIBC_HAS_XLOCALE__) && !defined(__UCLIBC_DO_XLOCALE)
#   define strftime_l wcsftime_l
#  endif
# endif
# ifdef L_wcsftime_l
#  define strftime_l wcsftime_l
#  define L_strftime_l
# endif
#else
# define CHAR_T char
# define UCHAR_T unsigned char
#endif

#ifndef __isleap
#define __isleap(y) ( !((y) % 4) && ( ((y) % 100) || !((y) % 400) ) )
#endif

#ifndef TZNAME_MAX
#define TZNAME_MAX _POSIX_TZNAME_MAX
#endif

#if defined (L_tzset) || defined (L_localtime_r) || defined(L_strftime) || \
    defined(L__time_mktime) || defined(L__time_mktime_tzi) || \
    ((defined(L_strftime) || defined(L_strftime_l)) && \
    defined(__UCLIBC_HAS_XLOCALE__))

void _time_tzset(int use_old_rules) attribute_hidden;

#ifndef L__time_mktime

 /* Jan 1, 2007 Z - tm = 0,0,0,1,0,107,1,0,0 */

static const time_t new_rule_starts = 1167609600;

#endif
#endif

/**********************************************************************/
/* The era code is currently unfinished. */
/*  #define ENABLE_ERA_CODE */

#define TZ_BUFLEN		(2*TZNAME_MAX + 56)

#ifdef __UCLIBC_HAS_TZ_FILE__

#include <sys/stat.h>
#include "paths.h"
/* ":<tzname>+hh:mm:ss<tzname>+hh:mm:ss,Mmm.w.d/hh:mm:ss,Mmm.w.d/hh:mm:ss" + nul */
/* 1 + 2*(1+TZNAME_MAX+1 + 9 + 7 + 9) + 1 = 2*TZNAME_MAX + 56 */

#else  /* __UCLIBC_HAS_TZ_FILE__ */

/* Probably no longer needed. */
#undef __UCLIBC_HAS_TZ_FILE_READ_MANY__

#endif /* __UCLIBC_HAS_TZ_FILE__ */

/**********************************************************************/

extern struct tm __time_tm attribute_hidden;

typedef struct {
	long gmt_offset;
	long dst_offset;
	short day;					/* for J or normal */
	short week;
	short month;
	short rule_type;			/* J, M, \0 */
	char tzname[TZNAME_MAX+1];
} rule_struct;

__UCLIBC_MUTEX_EXTERN(_time_tzlock) attribute_hidden;

extern rule_struct _time_tzinfo[2] attribute_hidden;

extern struct tm *_time_t2tm(const time_t *__restrict timer,
					int offset, struct tm *__restrict result) attribute_hidden;

extern time_t _time_mktime(struct tm *timeptr, int store_on_success) attribute_hidden;

extern struct tm *__time_localtime_tzi(const time_t *__restrict timer,
					struct tm *__restrict result,
					rule_struct *tzi) attribute_hidden;

extern time_t _time_mktime_tzi(struct tm *timeptr, int store_on_success,
					rule_struct *tzi) attribute_hidden;

/**********************************************************************/
#ifdef L_asctime

static char __time_str[26];

char *asctime(const struct tm *ptm)
{
	return asctime_r(ptm, __time_str);
}
libc_hidden_def(asctime)

#endif
/**********************************************************************/
#ifdef L_asctime_r

/* Strictly speaking, this implementation isn't correct.  ANSI/ISO specifies
 * that the implementation of asctime() be equivalent to
 *
 *   char *asctime(const struct tm *timeptr)
 *   {
 *       static char wday_name[7][3] = {
 *           "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
 *       };
 *       static char mon_name[12][3] = {
 *           "Jan", "Feb", "Mar", "Apr", "May", "Jun",
 *           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
 *       };
 *       static char result[26];
 *
 *       sprintf(result, "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n",
 *           wday_name[timeptr->tm_wday],
 *           mon_name[timeptr->tm_mon],
 *           timeptr->tm_mday, timeptr->tm_hour,
 *           timeptr->tm_min, timeptr->tm_sec,
 *           1900 + timeptr->tm_year);
 *       return result;
 *   }
 *
 * but the above is either inherently unsafe, or carries with it the implicit
 * assumption that all fields of timeptr fall within their usual ranges, and
 * that the tm_year value falls in the range [-2899,8099] to avoid overflowing
 * the static buffer.
 *
 * If we take the implicit assumption as given, then the implementation below
 * is still incorrect for tm_year values < -900, as there will be either
 * 0-padding and/or a missing negative sign for the year conversion .  But given
 * the usual use of asctime(), I think it isn't unreasonable to restrict correct
 * operation to the domain of years between 1000 and 9999.
 */

/* This is generally a good thing, but if you're _sure_ any data passed will be
 * in range, you can #undef this. */
#define SAFE_ASCTIME_R		1

static const unsigned char at_data[] = {
	'S', 'u', 'n', 'M', 'o', 'n', 'T', 'u', 'e', 'W', 'e', 'd',
	'T', 'h', 'u', 'F', 'r', 'i', 'S', 'a', 't',

	'J', 'a', 'n', 'F', 'e', 'b', 'M', 'a', 'r', 'A', 'p', 'r',
	'M', 'a', 'y', 'J', 'u', 'n', 'J', 'u', 'l', 'A', 'u', 'g',
	'S', 'e', 'p', 'O', 'c', 't', 'N', 'o', 'v', 'D', 'e', 'c',

#ifdef SAFE_ASCTIME_R
	'?', '?', '?',
#endif
	' ', '?', '?', '?',
	' ', '0',
	offsetof(struct tm, tm_mday),
	' ', '0',
	offsetof(struct tm, tm_hour),
	':', '0',
	offsetof(struct tm, tm_min),
	':', '0',
	offsetof(struct tm, tm_sec),
	' ', '?', '?', '?', '?', '\n', 0
};

char *asctime_r(register const struct tm *__restrict ptm,
				register char *__restrict buffer)
{
	int tmp;

	assert(ptm);
	assert(buffer);

#ifdef SAFE_ASCTIME_R
	memcpy(buffer, at_data + 3*(7 + 12), sizeof(at_data) - 3*(7 + 12));

	if (((unsigned int)(ptm->tm_wday)) <= 6) {
		memcpy(buffer, at_data + 3 * ptm->tm_wday, 3);
	}

	if (((unsigned int)(ptm->tm_mon)) <= 11) {
		memcpy(buffer + 4, at_data + 3*7 + 3 * ptm->tm_mon, 3);
	}
#else
	assert(((unsigned int)(ptm->tm_wday)) <= 6);
	assert(((unsigned int)(ptm->tm_mon)) <= 11);

	memcpy(buffer, at_data + 3*(7 + 12) - 3, sizeof(at_data) + 3 - 3*(7 + 12));

	memcpy(buffer, at_data + 3 * ptm->tm_wday, 3);
	memcpy(buffer + 4, at_data + 3*7 + 3 * ptm->tm_mon, 3);
#endif

#ifdef SAFE_ASCTIME_R
	buffer += 19;
	tmp = ptm->tm_year + 1900;
	if (((unsigned int) tmp) < 10000) {
		buffer += 4;
		do {
			*buffer = '0' + (tmp % 10);
			tmp /= 10;
		} while (*--buffer == '?');
	}
/*	Not sure if we should even bother ...
	} else {
		__set_errno(EOVERFLOW);
		return NULL;
	}
*/
#else  /* SAFE_ASCTIME_R */
	buffer += 23;
	tmp = ptm->tm_year + 1900;
	assert( ((unsigned int) tmp) < 10000 );
/*	Not sure if we should even bother ...
	if ( ((unsigned int) tmp) >= 10000 ) {
		__set_errno(EOVERFLOW);
		return NULL;
	}
*/
	do {
		*buffer = '0' + (tmp % 10);
		tmp /= 10;
	} while (*--buffer == '?');
#endif /* SAFE_ASCTIME_R */

	do {
		--buffer;
		tmp = *((int *)(((const char *) ptm) + (int) *buffer));
#ifdef SAFE_ASCTIME_R
		if (((unsigned int) tmp) >= 100) { /* Just check 2 digit non-neg. */
			buffer[-1] = *buffer = '?';
		} else
#else
		assert(((unsigned int) tmp) < 100); /* Just check 2 digit non-neg. */
#endif
		{
			*buffer = '0' + (tmp % 10);
#ifdef __BCC__
			buffer[-1] = '0' + (tmp/10);
#else
			buffer[-1] += (tmp/10);
#endif
		}
	} while ((buffer -= 2)[-2] == '0');

	if (*++buffer == '0') {		/* Space-pad day of month. */
		*buffer = ' ';
	}

	return buffer - 8;
}
libc_hidden_def(asctime_r)

#endif
/**********************************************************************/
#ifdef L_clock

#include <sys/times.h>

#ifndef __BCC__
#if CLOCKS_PER_SEC != 1000000L
#error unexpected value for CLOCKS_PER_SEC!
#endif
#endif

#ifdef __UCLIBC_CLK_TCK_CONST
# if __UCLIBC_CLK_TCK_CONST > CLOCKS_PER_SEC
#  error __UCLIBC_CLK_TCK_CONST > CLOCKS_PER_SEC!
# elif __UCLIBC_CLK_TCK_CONST < 1
#  error __UCLIBC_CLK_TCK_CONST < 1!
# endif
#endif

/* Note: SUSv3 notes
 *
 *   On XSI-conformant systems, CLOCKS_PER_SEC is defined to be one million.
 *
 *   The value returned by clock() may wrap around on some implementations.
 *   For example, on a machine with 32-bit values for clock_t, it wraps
 *   after 2147 seconds.
 *
 * This implies that we should bitwise and with LONG_MAX.
 */

clock_t clock(void)
{
	struct tms xtms;
	unsigned long t;

	times(&xtms);

	t = ((unsigned long) xtms.tms_utime) + xtms.tms_stime;

#ifndef __UCLIBC_CLK_TCK_CONST

# error __UCLIBC_CLK_TCK_CONST not defined!

#elif ((CLOCKS_PER_SEC % __UCLIBC_CLK_TCK_CONST) == 0)

	/* CLOCKS_PER_SEC == k * __UCLIBC_CLK_TCK_CONST for some integer k >= 1. */
	return ((t * (CLOCKS_PER_SEC/__UCLIBC_CLK_TCK_CONST)) & LONG_MAX);

#else

	/* Unlike the previous case, the scaling factor is not an integer.
	 * So when tms_utime, tms_stime, or their sum wraps, some of the
	 * "visible" bits in the return value are affected.  Nothing we
	 * can really do about this though other than handle tms_utime and
	 * tms_stime seperately and then sum.  But since that doesn't really
	 * buy us much, we don't bother. */

	return ((((t / __UCLIBC_CLK_TCK_CONST) * CLOCKS_PER_SEC)
			 + ((((t % __UCLIBC_CLK_TCK_CONST) * CLOCKS_PER_SEC)
				 / __UCLIBC_CLK_TCK_CONST))
			 ) & LONG_MAX);

#endif
}

#endif
/**********************************************************************/
#ifdef L_ctime

char *ctime(const time_t *t)
{
	/* ANSI/ISO/SUSv3 say that ctime is equivalent to the following:
	 * return asctime(localtime(t));
	 * I don't think "equivalent" means "it uses the same internal buffer",
	 * it means "gives the same resultant string".
	 *
	 * I doubt anyone ever uses weird code like:
	 * struct tm *ptm = localtime(t1); ...; ctime(t2); use(ptm);
	 * which relies on the assumption that ctime's and localtime's
	 * internal static struct tm is the same.
	 *
	 * Using localtime_r instead of localtime avoids linking in
	 * localtime's static buffer:
	 */
	struct tm xtm;
	memset(&xtm, 0, sizeof(xtm));

	return asctime(localtime_r(t, &xtm));
}
libc_hidden_def(ctime)
#endif
/**********************************************************************/
#ifdef L_ctime_r

char *ctime_r(const time_t *t, char *buf)
{
	struct tm xtm;

	return asctime_r(localtime_r(t, &xtm), buf);
}

#endif
/**********************************************************************/
#ifdef L_difftime

#include <float.h>

#if FLT_RADIX != 2
#error difftime implementation assumptions violated for you arch!
#endif

double difftime(time_t time1, time_t time0)
{
#if (LONG_MAX >> DBL_MANT_DIG) == 0

	/* time_t fits in the mantissa of a double. */
	return (double)time1 - (double)time0;

#elif ((LONG_MAX >> DBL_MANT_DIG) >> DBL_MANT_DIG) == 0

	/* time_t can overflow the mantissa of a double. */
	time_t t1, t0, d;

	d = ((time_t) 1) << DBL_MANT_DIG;
	t1 = time1 / d;
	time1 -= (t1 * d);
	t0 = time0 / d;
	time0 -= (t0*d);

	/* Since FLT_RADIX==2 and d is a power of 2, the only possible
	 * rounding error in the expression below would occur from the
	 * addition. */
	return (((double) t1) - t0) * d + (((double) time1) - time0);

#else
#error difftime needs special implementation on your arch.
#endif
}

#endif
/**********************************************************************/
#ifdef L_gmtime

struct tm *gmtime(const time_t *timer)
{
	register struct tm *ptm = &__time_tm;

	_time_t2tm(timer, 0, ptm); /* Can return NULL... */

	return ptm;
}

#endif
/**********************************************************************/
#ifdef L_gmtime_r

struct tm *gmtime_r(const time_t *__restrict timer,
					struct tm *__restrict result)
{
	return _time_t2tm(timer, 0, result);
}

#endif
/**********************************************************************/
#ifdef L_localtime

struct tm *localtime(const time_t *timer)
{
	register struct tm *ptm = &__time_tm;

	/* In this implementation, tzset() is called by localtime_r().  */

	localtime_r(timer, ptm);	/* Can return NULL... */

	return ptm;
}
libc_hidden_def(localtime)

#endif
/**********************************************************************/
#ifdef L_localtime_r

struct tm *localtime_r(register const time_t *__restrict timer,
					   register struct tm *__restrict result)
{
	__UCLIBC_MUTEX_LOCK(_time_tzlock);

	_time_tzset(*timer < new_rule_starts);

	__time_localtime_tzi(timer, result, _time_tzinfo);

	__UCLIBC_MUTEX_UNLOCK(_time_tzlock);

	return result;
}
libc_hidden_def(localtime_r)

#endif
/**********************************************************************/
#ifdef L__time_localtime_tzi

#ifdef __UCLIBC_HAS_TM_EXTENSIONS__

struct ll_tzname_item;

typedef struct ll_tzname_item {
	struct ll_tzname_item *next;
	char tzname[1];
} ll_tzname_item_t;

/* Structures form a list "UTC" -> "???" -> "tzname1" -> "tzname2"... */
static struct {
	struct ll_tzname_item *next;
	char tzname[4];
} ll_tzname_UNKNOWN = { NULL, "???" };
static const struct {
	struct ll_tzname_item *next;
	char tzname[4];
} ll_tzname_UTC = { (void*)&ll_tzname_UNKNOWN, "UTC" };

static const char *lookup_tzname(const char *key)
{
	int len;
	ll_tzname_item_t *p = (void*) &ll_tzname_UTC;

	do {
		if (strcmp(p->tzname, key) == 0)
			return p->tzname;
		p = p->next;
	} while (p != NULL);

	/* Hmm... a new name. */
	len = strnlen(key, TZNAME_MAX+1);
	if (len < TZNAME_MAX+1) { /* Verify legal length */
		p = malloc(sizeof(ll_tzname_item_t) + len);
		if (p != NULL) {
			/* Insert as 3rd item in the list. */
			p->next = ll_tzname_UNKNOWN.next;
			ll_tzname_UNKNOWN.next = p;
			return strcpy(p->tzname, key);
		}
	}

	/* Either invalid or couldn't alloc. */
	return ll_tzname_UNKNOWN.tzname;
}

#endif /* __UCLIBC_HAS_TM_EXTENSIONS__ */

static const unsigned char day_cor[] = { /* non-leap */
	31, 31, 34, 34, 35, 35, 36, 36, 36, 37, 37, 38, 38
/*	 0,  0,  3,  3,  4,  4,  5,  5,  5,  6,  6,  7,  7 */
/*	    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 */
};

/* Note: timezone locking is done by localtime_r. */

static int tm_isdst(register const struct tm *__restrict ptm,
					register rule_struct *r)
{
	long sec;
	int i, isdst, isleap, day, day0, monlen, mday;
	int oday = oday; /* ok to be uninitialized, shutting up compiler warning */

	isdst = 0;
	if (r[1].tzname[0] != 0) {
		/* First, get the current seconds offset from the start of the year.
		 * Fields of ptm are assumed to be in their normal ranges. */
		sec = ptm->tm_sec
			+ 60 * (ptm->tm_min
					+ 60 * (long)(ptm->tm_hour
								  + 24 * ptm->tm_yday));
		/* Do some prep work. */
		i = (ptm->tm_year % 400) + 1900; /* Make sure we don't overflow. */
		isleap = __isleap(i);
		--i;
		day0 = (1
				+ i	/* Normal years increment 1 wday. */
				+ (i/4)
				- (i/100)
				+ (i/400) ) % 7;
		i = 0;
		do {
			day = r->day;		/* Common for 'J' and # case. */
			if (r->rule_type == 'J') {
				if (!isleap || (day < (31+29))) {
					--day;
				}
			} else if (r->rule_type == 'M') {
				/* Find 0-based day number for 1st of the month. */
				day = 31 * r->month - day_cor[r->month - 1];
				if (isleap && (day >= 59)) {
					++day;
				}
				monlen = 31 + day_cor[r->month - 1] - day_cor[r->month];
				if (isleap && (r->month == 2)) {
					++monlen;
				}
				/* Weekday (0 is Sunday) of 1st of the month
				 * is (day0 + day) % 7. */
				mday = r->day - ((day0 + day) % 7);
				if (mday >= 0) {
					mday -= 7;	/* Back up into prev month since r->week > 0. */
				}
				mday += 7 * r->week;
				if (mday >= monlen) {
					mday -= 7;
				}
				/* So, 0-based day number is... */
				day += mday;
			}

			if (i != 0) {
				/* Adjust sec since dst->std change time is in dst. */
				sec += (r[-1].gmt_offset - r->gmt_offset);
				if (oday > day) {
					++isdst;	/* Year starts in dst. */
				}
			}
			oday = day;

			/* Now convert day to seconds and add offset and compare. */
			if (sec >= (day * 86400L) + r->dst_offset) {
				++isdst;
			}
			++r;
		} while (++i < 2);
	}

	return (isdst & 1);
}

struct tm attribute_hidden *__time_localtime_tzi(register const time_t *__restrict timer,
								register struct tm *__restrict result,
								rule_struct *tzi)
{
	time_t x[1];
	long offset;
	int days, dst;

	dst = 0;
	do {
		days = -7;
		offset = 604800L - tzi[dst].gmt_offset;
		if (*timer > (LONG_MAX - 604800L)) {
			days = -days;
			offset = -offset;
		}
		*x = *timer + offset;

		_time_t2tm(x, days, result);
		result->tm_isdst = dst;
#ifdef __UCLIBC_HAS_TM_EXTENSIONS__
# ifdef __USE_BSD
		result->tm_gmtoff = - tzi[dst].gmt_offset;
		result->tm_zone = lookup_tzname(tzi[dst].tzname);
# else
		result->__tm_gmtoff = - tzi[dst].gmt_offset;
		result->__tm_zone = lookup_tzname(tzi[dst].tzname);
# endif
#endif /* __UCLIBC_HAS_TM_EXTENSIONS__ */
	} while ((++dst < 2)
			 && ((result->tm_isdst = tm_isdst(result, tzi)) != 0));

	return result;
}

#endif
/**********************************************************************/
#ifdef L_mktime

time_t mktime(struct tm *timeptr)
{
	return  _time_mktime(timeptr, 1);
}

/* Another name for `mktime'.  */
/* time_t timelocal(struct tm *tp) */
strong_alias(mktime,timelocal)

#endif
/**********************************************************************/
#ifdef L_timegm
/* Like `mktime' but timeptr represents Universal Time, not local time. */

time_t timegm(struct tm *timeptr)
{
	rule_struct gmt_tzinfo[2];

	memset(gmt_tzinfo, 0, sizeof(gmt_tzinfo));
	strcpy(gmt_tzinfo[0].tzname, "GMT"); /* Match glibc behavior here. */

	return  _time_mktime_tzi(timeptr, 1, gmt_tzinfo);
}

#endif
/**********************************************************************/
#if defined(L_strftime) || defined(L_strftime_l) \
	|| defined(L_wcsftime) || defined(L_wcsftime_l)

#if defined(__UCLIBC_HAS_XLOCALE__) && !defined(__UCLIBC_DO_XLOCALE)

size_t strftime(CHAR_T *__restrict s, size_t maxsize,
				const CHAR_T *__restrict format,
				const struct tm *__restrict timeptr)
{
	return strftime_l(s, maxsize, format, timeptr, __UCLIBC_CURLOCALE);
}

#else  /* defined(__UCLIBC_HAS_XLOCALE__) && !defined(__UCLIBC_DO_XLOCALE) */

#define NO_E_MOD		0x80
#define NO_O_MOD		0x40

#define ILLEGAL_SPEC	0x3f

#define INT_SPEC		0x00	/* must be 0x00!! */
#define STRING_SPEC		0x10	/* must be 0x10!! */
#define CALC_SPEC		0x20
#define STACKED_SPEC	0x30

#define MASK_SPEC		0x30

/* Compatibility:
 *
 * No alternate digit (%O?) handling.  Always uses 0-9.
 * Alternate locale format (%E?) handling is broken for nontrivial ERAs.
 * glibc's %P is currently faked by %p.  This means it doesn't do lower case.
 * glibc's %k, %l, and %s are handled.
 * glibc apparently allows (and ignores) extraneous 'E' and 'O' modifiers,
 *   while they are flagged as illegal conversions here.
 */

/* Warning: Assumes ASCII values! (as do lots of other things in the lib...) */
static const unsigned char spec[] = {
	/* A */		0x03 |  STRING_SPEC | NO_E_MOD | NO_O_MOD,
	/* B */		0x04 |  STRING_SPEC | NO_E_MOD | NO_O_MOD,
	/* C */		0x0a |     INT_SPEC            | NO_O_MOD,
	/* D */		0x02 | STACKED_SPEC | NO_E_MOD | NO_O_MOD,
	/* E */		       ILLEGAL_SPEC | NO_E_MOD | NO_O_MOD,
	/* F */		0x03 | STACKED_SPEC | NO_E_MOD | NO_O_MOD,
	/* G */		0x03 |    CALC_SPEC | NO_E_MOD | NO_O_MOD,
	/* H */		0x0b |     INT_SPEC | NO_E_MOD,
	/* I */		0x0c |     INT_SPEC | NO_E_MOD,
	/* J */		       ILLEGAL_SPEC | NO_E_MOD | NO_O_MOD,
	/* K */		       ILLEGAL_SPEC | NO_E_MOD | NO_O_MOD,
	/* L */		       ILLEGAL_SPEC | NO_E_MOD | NO_O_MOD,
	/* M */		0x0d |     INT_SPEC | NO_E_MOD,
	/* N */		       ILLEGAL_SPEC | NO_E_MOD | NO_O_MOD,
	/* O */		       ILLEGAL_SPEC | NO_E_MOD | NO_O_MOD,
	/* P */		0x05 |  STRING_SPEC | NO_E_MOD | NO_O_MOD, /* glibc ; use %p */
	/* Q */		       ILLEGAL_SPEC | NO_E_MOD | NO_O_MOD,
	/* R */		0x04 | STACKED_SPEC | NO_E_MOD | NO_O_MOD,
	/* S */		0x0e |     INT_SPEC | NO_E_MOD,
	/* T */		0x05 | STACKED_SPEC | NO_E_MOD | NO_O_MOD,
	/* U */		0x04 |    CALC_SPEC | NO_E_MOD,
	/* V */		0x05 |    CALC_SPEC | NO_E_MOD,
	/* W */		0x06 |    CALC_SPEC | NO_E_MOD,
	/* X */		0x0a | STACKED_SPEC            | NO_O_MOD,
	/* Y */		0x0f |     INT_SPEC            | NO_O_MOD,
	/* Z */		0x01 |    CALC_SPEC | NO_E_MOD | NO_O_MOD,
	'?',						/* 26 */
	'?',						/* 27 */
	'?',						/* 28 */
	'?',						/* 29 */
	0,							/* 30 */
	0,							/* 31 */
	/* a */		0x00 |  STRING_SPEC | NO_E_MOD | NO_O_MOD,
	/* b */		0x01 |  STRING_SPEC | NO_E_MOD | NO_O_MOD,
	/* c */		0x08 | STACKED_SPEC            | NO_O_MOD,
	/* d */		0x00 |     INT_SPEC | NO_E_MOD,
	/* e */		0x01 |     INT_SPEC | NO_E_MOD,
	/* f */		       ILLEGAL_SPEC | NO_E_MOD | NO_O_MOD,
	/* g */		0x02 |    CALC_SPEC | NO_E_MOD | NO_O_MOD,
	/* h */		0x01 |  STRING_SPEC | NO_E_MOD | NO_O_MOD, /* same as b */
	/* i */		       ILLEGAL_SPEC | NO_E_MOD | NO_O_MOD,
	/* j */		0x08 |     INT_SPEC | NO_E_MOD | NO_O_MOD,
	/* k */		0x03 |     INT_SPEC | NO_E_MOD | NO_O_MOD, /* glibc */
	/* l */		0x04 |     INT_SPEC | NO_E_MOD | NO_O_MOD, /* glibc */
	/* m */		0x05 |     INT_SPEC | NO_E_MOD,
	/* n */		0x00 | STACKED_SPEC | NO_E_MOD | NO_O_MOD,
	/* o */		       ILLEGAL_SPEC | NO_E_MOD | NO_O_MOD,
	/* p */		0x02 |  STRING_SPEC | NO_E_MOD | NO_O_MOD,
	/* q */		       ILLEGAL_SPEC | NO_E_MOD | NO_O_MOD,
	/* r */		0x0b | STACKED_SPEC | NO_E_MOD | NO_O_MOD,
	/* s */		0x07 |    CALC_SPEC | NO_E_MOD | NO_O_MOD, /* glibc */
	/* t */		0x01 | STACKED_SPEC | NO_E_MOD | NO_O_MOD,
	/* u */		0x07 |     INT_SPEC | NO_E_MOD,
	/* v */		       ILLEGAL_SPEC | NO_E_MOD | NO_O_MOD,
	/* w */		0x02 |     INT_SPEC | NO_E_MOD,
	/* x */		0x09 | STACKED_SPEC            | NO_O_MOD,
	/* y */		0x09 |     INT_SPEC,
	/* z */		0x00 |    CALC_SPEC | NO_E_MOD | NO_O_MOD,


	/* WARNING!!! These are dependent on the layout of struct tm!!! */
#define FIELD_MAX (26+6+26)
	60 /* 61? */, 59, 23, 31, 11, 0 /* 9999 */, 6, 0 /* 365 */,

#define TP_OFFSETS (FIELD_MAX+8)
	3, /* d */
	3, /* e */
	6, /* w */
	2, /* k */
	2, /* l */
	4, /* m */
	0, /* CURRENTLY UNUSED */
	/* NOTE: u,j,y order must be preserved as 6,7,5 seq is used in the code! */
#define CALC_OFFSETS (TP_OFFSETS + 7)
	6, /* u */
	7, /* j */
	5, /* y */
	5, /* C */
	2, /* H */
	2, /* I */
	1, /* M */
	0, /* S */
	5, /* Y */
	6, /* a */
	4, /* b, h */
	2, /* p */
	6, /* A */
	4, /* B */
	2, /* P */

#define TP_CODES (TP_OFFSETS + 16 + 6)
	2 | 16, /* d */
	2, /* e */
	0 | 16, /* w */
	2, /* k */
	2 | 32 | 0, /* l */
	2 | 16 | 1, /* m */
	0, /* CURRENTLY UNUSED */
	0 | 16 | 8 , /* u */
	4 | 16 | 1, /* j */
	2 | 128 | 32 | 16 , /* y */
	2 | 128 | 64 | 32 | 16 , /* C */
	2 | 16, /* H */
	2 | 32 | 16 | 0, /* I */
	2 | 16, /* M */
	2 | 16, /* S */
	6 | 16, /* Y */
	2, /* a */
	2, /* b, h */
	2 | 64, /* p */
	2, /* A */
	2, /* B */
	2 | 64, /* P */

#define STRINGS_NL_ITEM_START (TP_CODES + 16 + 6)
	_NL_ITEM_INDEX(ABDAY_1),	/* a */
	_NL_ITEM_INDEX(ABMON_1),	/* b, h */
	_NL_ITEM_INDEX(AM_STR),		/* p */
	_NL_ITEM_INDEX(DAY_1),		/* A */
	_NL_ITEM_INDEX(MON_1),		/* B */
	_NL_ITEM_INDEX(AM_STR),		/* P -- wrong! need lower case */

#define STACKED_STRINGS_START (STRINGS_NL_ITEM_START+6)
	6, 7, 8, 16, 24, 29,		/* 6 - offsets from offset-count to strings */
	'\n', 0,					/* 2 */
	'\t', 0,					/* 2 */
	'%', 'm', '/', '%', 'd', '/', '%', 'y', 0, /* 9 - %D */
	'%', 'Y', '-', '%', 'm', '-', '%', 'd', 0, /* 9 - %F (glibc extension) */
	'%', 'H', ':', '%', 'M', 0,	/* 6 - %R*/
	'%', 'H', ':', '%', 'M', ':', '%', 'S', 0, /* 9 - %T */

#define STACKED_STRINGS_NL_ITEM_START (STACKED_STRINGS_START + 43)
	_NL_ITEM_INDEX(D_T_FMT),	/* c */
	_NL_ITEM_INDEX(D_FMT),		/* x */
	_NL_ITEM_INDEX(T_FMT),		/* X */
	_NL_ITEM_INDEX(T_FMT_AMPM), /* r */
#ifdef ENABLE_ERA_CODE
	_NL_ITEM_INDEX(ERA_D_T_FMT), /* Ec */
	_NL_ITEM_INDEX(ERA_D_FMT),	/* Ex */
	_NL_ITEM_INDEX(ERA_T_FMT),	/* EX */
#endif
};

static int load_field(int k, const struct tm *__restrict timeptr)
{
	int r;
	int r_max;

	r = ((int *) timeptr)[k];

	r_max = spec[FIELD_MAX + k];

	if (k == 7) {
		r_max = 365;
	} else if (k == 5) {
		r += 1900;
		r_max = 9999;
	}

	if ((((unsigned int) r) > r_max) || ((k == 3) && !r)) {
		r = -1;
	}

	return r;
}

#if defined __UCLIBC_HAS_WCHAR__ && (defined L_wcsftime || defined L_wcsftime_l)
static wchar_t* fmt_to_wc_1(const char *src)
{
	mbstate_t mbstate;
	size_t src_len = strlen(src);
	wchar_t *dest = (wchar_t *)malloc((src_len + 1) * sizeof(wchar_t));
	if (dest == NULL)
		return NULL;
	mbstate.__mask = 0;
	if (mbsrtowcs(dest, &src, src_len + 1, &mbstate) == (size_t) -1) {
		free(dest);
		return NULL;
	}
	return dest;
}
# define fmt_to_wc(dest, src) \
	dest = alloc[++allocno] = fmt_to_wc_1(src)
# define to_wc(dest, src) \
	dest = fmt_to_wc_1(src)
#else
# define fmt_to_wc(dest, src) (dest) = (src)
# define to_wc(dest, src) (dest) = (src)
#endif

#define MAX_PUSH 4

size_t __XL_NPP(strftime)(CHAR_T *__restrict s, size_t maxsize,
					  const CHAR_T *__restrict format,
					  const struct tm *__restrict timeptr   __LOCALE_PARAM )
{
	long tzo;
	register const CHAR_T *p;
	const CHAR_T *o;
	const char *ccp;
#ifndef __UCLIBC_HAS_TM_EXTENSIONS__
	const rule_struct *rsp;
#endif
	const CHAR_T *stack[MAX_PUSH];
#if defined __UCLIBC_HAS_WCHAR__ && (defined L_wcsftime || defined L_wcsftime_l)
	const CHAR_T *alloc[MAX_PUSH];
	int allocno = -1;
#endif
	size_t count;
	size_t o_count;
	int field_val = 0, i = 0, j, lvl;
	int x[3];			/* wday, yday, year */
	int isofm, days;
	char buf[__UIM_BUFLEN_LONG] = {0,};
	unsigned char mod;
	unsigned char code;

	/* We'll, let's get this out of the way. */
	_time_tzset(_time_mktime((struct tm *) timeptr, 0) < new_rule_starts);

	lvl = 0;
	p = format;
	count = maxsize;

LOOP:
	if (!count) {
		return 0;
	}
	if (!*p) {
		if (lvl == 0) {
			*s = 0;				/* nul-terminate */
			return maxsize - count;
		}
		p = stack[--lvl];
		goto LOOP;
	}

	o_count = 1;
	if ((*(o = (CHAR_T *)p) == '%') && (*++p != '%')) {
#if 0 /* TODO, same for strptime */
		/* POSIX.1-2008 allows %0xY %+nY %-nY etc. for certain formats.
		 * Just accept these for all (for now) */
		const int plus = *p == '+';
		CHAR_T *q = (CHAR_T *)p;
		long int o_width = __XL_NPP(strtol)(p, &q, 0 __LOCALE_ARG);
		if (o_width > 0 && o_width < 256) { /* arbitrary upper limit */
			o_count = o_width;
			if (plus) {
				*s++ = '+';
				--count;
			}
			p = q;
		} else {
			o_count = 2;
		}
#else
		o_count = 2;
#endif
		mod = ILLEGAL_SPEC;
		if ((*p == 'O') || (*p == 'E')) { /* modifier */
			mod |= ((*p == 'O') ? NO_O_MOD : NO_E_MOD);
			++o_count;
			++p;
		}
		if ((((unsigned char)(((*p) | 0x20) - 'a')) >= 26)
			|| (((code = spec[(int)(*p - 'A')]) & mod) >= ILLEGAL_SPEC)
			) {
			if (!*p) {
				--p;
				--o_count;
			}
			goto OUTPUT;
		}
		code &= ILLEGAL_SPEC;	/* modifiers are preserved in mod var. */

		if ((code & MASK_SPEC) == STACKED_SPEC) {
			if (lvl == MAX_PUSH) {
				goto OUTPUT;	/* Stack full so treat as illegal spec. */
			}
			stack[lvl++] = ++p;
			if ((code &= 0xf) < 8) {
				ccp = (const char *)(spec + STACKED_STRINGS_START + code);
				ccp += *ccp;
				fmt_to_wc(p, ccp);
				goto LOOP;
			}
			ccp = (const char *)spec + STACKED_STRINGS_NL_ITEM_START + (code & 7);
			fmt_to_wc(p, ccp);
#ifdef ENABLE_ERA_CODE
			if ((mod & NO_E_MOD) /* Actually, this means E modifier present. */
				&& (*(ccp = __XL_NPP(nl_langinfo)(_NL_ITEM(LC_TIME,
							(int)(((unsigned char *)p)[4]))
							__LOCALE_ARG
							)))
				) {
				fmt_to_wc(p, ccp);
				goto LOOP;
			}
#endif
			ccp = __XL_NPP(nl_langinfo)(_NL_ITEM(LC_TIME,
							(int)(*((unsigned char *)p)))
							__LOCALE_ARG
							);
			fmt_to_wc(p, ccp);
			goto LOOP;
		}

		ccp = (const char *)(spec + 26);	/* set to "????" */
		if ((code & MASK_SPEC) == CALC_SPEC) {

			if (*p == 's') {
				time_t t;

				/* Use a cast to silence the warning since *timeptr won't
				 * be changed. */
				if ((t = _time_mktime((struct tm *) timeptr, 0))
					== ((time_t) -1)
					) {
					o_count = 1;
					goto OUTPUT;
				}
#ifdef TIME_T_IS_UNSIGNED
				ccp = _uintmaxtostr(buf + sizeof(buf) - 1,
								  (uintmax_t) t,
								  10, __UIM_DECIMAL);
#else
				ccp = _uintmaxtostr(buf + sizeof(buf) - 1,
								  (uintmax_t) t,
								  -10, __UIM_DECIMAL);
#endif
				o_count = sizeof(buf);
				fmt_to_wc(o, ccp);
				goto OUTPUT;
			} else if (((*p) | 0x20) == 'z') { /* 'z' or 'Z' */

				if (timeptr->tm_isdst < 0) {
					/* SUSv3 specifies this behavior for 'z', but we'll also
					 * treat it as "no timezone info" for 'Z' too. */
					o_count = 0;
					goto OUTPUT;
				}

#ifdef __UCLIBC_HAS_TM_EXTENSIONS__

# ifdef __USE_BSD
#  define RSP_TZNAME		timeptr->tm_zone
#  define RSP_GMT_OFFSET	(-timeptr->tm_gmtoff)
# else
#  define RSP_TZNAME		timeptr->__tm_zone
#  define RSP_GMT_OFFSET	(-timeptr->__tm_gmtoff)
# endif

#else

#define RSP_TZNAME		rsp->tzname
#define RSP_GMT_OFFSET	rsp->gmt_offset

				__UCLIBC_MUTEX_LOCK(_time_tzlock);

				rsp = _time_tzinfo;
				if (timeptr->tm_isdst > 0) {
					++rsp;
				}
#endif

				if (*p == 'Z') {
					ccp = RSP_TZNAME;
#ifdef __UCLIBC_HAS_TM_EXTENSIONS__
					/* Sigh... blasted glibc extensions.  Of course we can't
					 * count on the pointer being valid.  Best we can do is
					 * handle NULL, which looks to be all that glibc does.
					 * At least that catches the memset() with 0 case.
					 * NOTE: We handle this case differently than glibc!
					 * It uses system timezone name (based on tm_isdst) in this
					 * case... although it always seems to use the embedded
					 * tm_gmtoff value.  What we'll do instead is treat the
					 * timezone name as unknown/invalid and return "???". */
					if (!ccp) {
						ccp = (const char *)(spec + 27); /* "???" */
					}
#endif
					assert(ccp != NULL);
#if 0
					if (!ccp) {	/* PARANOIA */
						ccp = spec+30; /* empty string */
					}
#endif
					o_count = SIZE_MAX;
					fmt_to_wc(o, ccp);
#ifdef __UCLIBC_HAS_TM_EXTENSIONS__
					goto OUTPUT;
#endif
				} else {		/* z */
					*s = '+';
					if ((tzo = -RSP_GMT_OFFSET) < 0) {
						tzo = -tzo;
						*s = '-';
					}
					++s;
					--count;

					i = tzo / 60;
					field_val = ((i / 60) * 100) + (i % 60);

					i = 16 + 6;	/* 0-fill, width = 4 */
				}
#ifndef __UCLIBC_HAS_TM_EXTENSIONS__
				__UCLIBC_MUTEX_UNLOCK(_time_tzlock);
				if (*p == 'Z') {
					goto OUTPUT;
				}
#endif
			} else {
				/* TODO: don't need year for U, W */
				for (i=0 ; i < 3 ; i++) {
					if ((x[i] = load_field(spec[CALC_OFFSETS+i],timeptr)) < 0) {
						goto OUTPUT;
					}
				}

				i = 16 + 2;		/* 0-fill, width = 2 */

				if ((*p == 'U') || (*p == 'W')) {
					field_val = ((x[1] - x[0]) + 7);
					if (*p == 'W') {
						++field_val;
					}
					field_val /= 7;
					if ((*p == 'W') && !x[0]) {
						--field_val;
					}
				} else {	/* ((*p == 'g') || (*p == 'G') || (*p == 'V')) */
ISO_LOOP:
					isofm = (((x[1] - x[0]) + 11) % 7) - 3;	/* [-3,3] */

					if (x[1] < isofm) {	/* belongs to previous year */
						--x[2];
						x[1] += 365 + __isleap(x[2]);
						goto ISO_LOOP;
					}

					field_val = ((x[1] - isofm) / 7) + 1; /* week # */
					days = 365 + __isleap(x[2]);
					isofm = ((isofm + 7*53 + 3 - days)) % 7 + days - 3; /* next year */
					if (x[1] >= isofm) { /* next year */
						x[1] -= days;
						++x[2];
						goto ISO_LOOP;
					}

					if (*p != 'V') { /* need year */
						field_val = x[2]; /* TODO: what if x[2] now 10000 ?? */
						if (*p == 'g') {
							field_val %= 100;
						} else {
							i = 16 + 6;	/* 0-fill, width = 4 */
						}
					}
				}
			}
		} else {
			i = TP_OFFSETS + (code & 0x1f);
			if ((field_val = load_field(spec[i], timeptr)) < 0) {
				goto OUTPUT;
			}

			i = spec[i+(TP_CODES - TP_OFFSETS)];

			j = (i & 128) ? 100: 12;
			if (i & 64) {
				field_val /= j;;
			}
			if (i & 32) {
				field_val %= j;
				if (((i & 128) + field_val) == 0) { /* mod 12? == 0 */
					field_val = j; /* set to 12 */
				}
			}
			field_val += (i & 1);
			if ((i & 8) && !field_val) {
				field_val += 7;
			}
		}

		if ((code & MASK_SPEC) == STRING_SPEC) {
			o_count = SIZE_MAX;
			field_val += spec[STRINGS_NL_ITEM_START + (code & 0xf)];
			ccp = __XL_NPP(nl_langinfo)(_NL_ITEM(LC_TIME, field_val)  __LOCALE_ARG);
			fmt_to_wc(o, ccp);
		} else {
#if 0 /* TODO, same for strptime */
			size_t min_count = ((i >> 1) & 3) + 1;
			if (o_count < min_count)
				o_count = min_count;
#else
			o_count = ((i >> 1) & 3) + 1;
#endif
			ccp = buf + o_count;
			do {
				*(char *)(--ccp) = '0' + (field_val % 10);
				field_val /= 10;
			} while (ccp > buf);
			if (*buf == '0') {
				*buf = ' ' + (i & 16);
			}
			fmt_to_wc(o, ccp);
		}
	}

OUTPUT:
	++p;
	while (o_count && count && *o) {
		*s++ = *o++;
		--o_count;
		--count;
	}
#if defined __UCLIBC_HAS_WCHAR__ && (defined L_wcsftime || defined L_wcsftime_l)
	if (allocno >= 0)
		free((void *)alloc[allocno--]);
#endif
	goto LOOP;
}
# ifdef L_strftime_l
libc_hidden_def(strftime_l)
# endif

#endif /* defined(__UCLIBC_HAS_XLOCALE__) && !defined(__UCLIBC_DO_XLOCALE) */

#endif
/**********************************************************************/
#if defined(L_strptime) || defined(L_strptime_l)

#define ISDIGIT(C) __isdigit_char((C))

#ifdef __UCLIBC_DO_XLOCALE
#define ISSPACE(C) isspace_l((C), locale_arg)
#else
#define ISSPACE(C) isspace((C))
#endif

#if defined(__UCLIBC_HAS_XLOCALE__) && !defined(__UCLIBC_DO_XLOCALE)

char *strptime(const char *__restrict buf, const char *__restrict format,
			   struct tm *__restrict tm)
{
	return strptime_l(buf, format, tm, __UCLIBC_CURLOCALE);
}

#else  /* defined(__UCLIBC_HAS_XLOCALE__) && !defined(__UCLIBC_DO_XLOCALE) */

/* TODO:
 * 1) %l and %k are space-padded, so "%l" by itself fails while " %l" succeeds.
 *    Both work for glibc.  So, should we always strip spaces?
 * 2) %Z
 */

/* Notes:
 * There are several differences between this strptime and glibc's strptime.
 * 1) glibc strips leading space before numeric conversions.
 * 2) glibc will read fields without whitespace in between.  SUSv3 states
 *    that you must have whitespace between conversion operators.  Besides,
 *    how do you know how long a number should be if there are leading 0s?
 * 3) glibc attempts to compute some the struct tm fields based on the
 *    data retrieved; tm_wday in particular.  I don't as I consider it
 *     another glibc attempt at mind-reading...
 */

#define NO_E_MOD		0x80
#define NO_O_MOD		0x40

#define ILLEGAL_SPEC	0x3f

#define INT_SPEC		0x00	/* must be 0x00!! */
#define STRING_SPEC		0x10	/* must be 0x10!! */
#define CALC_SPEC		0x20
#define STACKED_SPEC	0x30

#define MASK_SPEC		0x30

/* Warning: Assumes ASCII values! (as do lots of other things in the lib...) */
static const unsigned char spec[] = {
	/* A */		0x02 |  STRING_SPEC | NO_E_MOD | NO_O_MOD,
	/* B */		0x01 |  STRING_SPEC | NO_E_MOD | NO_O_MOD,
	/* C */		0x08 |     INT_SPEC            | NO_O_MOD,
	/* D */		0x01 | STACKED_SPEC | NO_E_MOD | NO_O_MOD,
	/* E */		       ILLEGAL_SPEC | NO_E_MOD | NO_O_MOD,
	/* F */		0x02 | STACKED_SPEC | NO_E_MOD | NO_O_MOD, /* glibc */
	/* G */		0x0f |     INT_SPEC | NO_E_MOD | NO_O_MOD, /* glibc */
	/* H */		0x06 |     INT_SPEC | NO_E_MOD,
	/* I */		0x07 |     INT_SPEC | NO_E_MOD,
	/* J */		       ILLEGAL_SPEC | NO_E_MOD | NO_O_MOD,
	/* K */		       ILLEGAL_SPEC | NO_E_MOD | NO_O_MOD,
	/* L */		       ILLEGAL_SPEC | NO_E_MOD | NO_O_MOD,
	/* M */		0x04 |     INT_SPEC | NO_E_MOD,
	/* N */		       ILLEGAL_SPEC | NO_E_MOD | NO_O_MOD,
	/* O */		       ILLEGAL_SPEC | NO_E_MOD | NO_O_MOD,
	/* P */		0x00 |  STRING_SPEC | NO_E_MOD | NO_O_MOD, /* glibc */
	/* Q */		       ILLEGAL_SPEC | NO_E_MOD | NO_O_MOD,
	/* R */		0x03 | STACKED_SPEC | NO_E_MOD | NO_O_MOD,
	/* S */		0x05 |     INT_SPEC | NO_E_MOD,
	/* T */		0x04 | STACKED_SPEC | NO_E_MOD | NO_O_MOD,
	/* U */		0x0c |     INT_SPEC | NO_E_MOD,
	/* V */		0x0d |     INT_SPEC | NO_E_MOD | NO_O_MOD, /* glibc */
	/* W */		0x0c |     INT_SPEC | NO_E_MOD,
	/* X */		0x0a | STACKED_SPEC            | NO_O_MOD,
	/* Y */		0x0a |     INT_SPEC            | NO_O_MOD,
	/* Z */		0x02 |    CALC_SPEC | NO_E_MOD | NO_O_MOD, /* glibc */

	/* WARNING! This assumes orderings:
	 *    AM,PM
	 *    ABDAY_1-ABDAY-7,DAY_1-DAY_7
	 *    ABMON_1-ABMON_12,MON_1-MON12
	 * Also, there are exactly 6 bytes between 'Z' and 'a'.
	 */
#define STRINGS_NL_ITEM_START (26)
	_NL_ITEM_INDEX(AM_STR),		/* p (P) */
	_NL_ITEM_INDEX(ABMON_1),	/* B, b */
	_NL_ITEM_INDEX(ABDAY_1),	/* A, a */
	2,
	24,
	14,

	/* a */		0x02 |  STRING_SPEC | NO_E_MOD | NO_O_MOD,
	/* b */		0x01 |  STRING_SPEC | NO_E_MOD | NO_O_MOD,
	/* c */		0x08 | STACKED_SPEC            | NO_O_MOD,
	/* d */		0x00 |     INT_SPEC | NO_E_MOD,
	/* e */		0x00 |     INT_SPEC | NO_E_MOD,
	/* f */		       ILLEGAL_SPEC | NO_E_MOD | NO_O_MOD,
	/* g */		0x0e |     INT_SPEC | NO_E_MOD | NO_O_MOD, /* glibc */
	/* h */		0x01 |  STRING_SPEC | NO_E_MOD | NO_O_MOD,
	/* i */		       ILLEGAL_SPEC | NO_E_MOD | NO_O_MOD,
	/* j */		0x01 |     INT_SPEC | NO_E_MOD | NO_O_MOD,
	/* k */		0x06 |     INT_SPEC | NO_E_MOD,            /* glibc */
	/* l */		0x07 |     INT_SPEC | NO_E_MOD,            /* glibc */
	/* m */		0x02 |     INT_SPEC | NO_E_MOD,
	/* n */		0x00 | STACKED_SPEC | NO_E_MOD | NO_O_MOD,
	/* o */		       ILLEGAL_SPEC | NO_E_MOD | NO_O_MOD,
	/* p */		0x00 |  STRING_SPEC | NO_E_MOD | NO_O_MOD,
	/* q */		       ILLEGAL_SPEC | NO_E_MOD | NO_O_MOD,
	/* r */		0x0b | STACKED_SPEC | NO_E_MOD | NO_O_MOD,
	/* s */		0x00 |    CALC_SPEC | NO_E_MOD | NO_O_MOD, /* glibc */
	/* t */		0x00 | STACKED_SPEC | NO_E_MOD | NO_O_MOD,
	/* u */		0x0b |     INT_SPEC | NO_E_MOD | NO_O_MOD, /* glibc */
	/* v */		       ILLEGAL_SPEC | NO_E_MOD | NO_O_MOD,
	/* w */		0x03 |     INT_SPEC | NO_E_MOD,
	/* x */		0x09 | STACKED_SPEC            | NO_O_MOD,
	/* y */		0x09 |     INT_SPEC,
	/* z */		0x01 |    CALC_SPEC | NO_E_MOD | NO_O_MOD, /* glibc */

#define INT_FIELD_START (26+6+26)
	/* (field #) << 3  + lower bound (0|1) + correction 0:none, 2:-1, 4:-1900
	 * followed by upper bound prior to correction with 1=>366 and 2=>9999. */
	/* d, e */	(3 << 3) + 1 + 0, 31,
	/* j */		(7 << 3) + 1 + 2, /* 366 */ 1,
	/* m */		(4 << 3) + 1 + 2, 12,
	/* w */		(6 << 3) + 0 + 0, 6,
	/* M */		(1 << 3) + 0 + 0, 59,
	/* S */		0        + 0 + 0, 60,
	/* H (k) */	(2 << 3) + 0 + 0, 23,
	/* I (l) */	(9 << 3) + 1 + 0, 12, /* goes with 8 -- am/pm */
	/* C */		(10<< 3) + 0 + 0, 99,
	/* y */		(11<< 3) + 0 + 0, 99,
	/* Y */		(5 << 3) + 0 + 4, /* 9999 */ 2,
	/* u */		(6 << 3) + 1 + 0, 7,
	/* The following are processed and range-checked, but ignored otherwise. */
	/* U, W */	(12<< 3) + 0 + 0, 53,
	/* V */		(12<< 3) + 1 + 0, 53,
	/* g */		(12<< 3) + 0 + 0, 99,
	/* G */		(12<< 3) + 0 /*+ 4*/, /* 9999 */ 2, /* Note: -1 or 10000? */

#define STACKED_STRINGS_START (INT_FIELD_START+32)
	5, 6, 14, 22, 27,			/* 5 - offsets from offset-count to strings */
	' ', 0,						/* 2 - %n or %t */
	'%', 'm', '/', '%', 'd', '/', '%', 'y', 0, /* 9 - %D */
	'%', 'Y', '-', '%', 'm', '-', '%', 'd', 0, /* 9 - %F (glibc extension) */
	'%', 'H', ':', '%', 'M', 0,	/* 6 - %R*/
	'%', 'H', ':', '%', 'M', ':', '%', 'S', 0, /* 9 - %T */

#define STACKED_STRINGS_NL_ITEM_START (STACKED_STRINGS_START + 40)
	_NL_ITEM_INDEX(D_T_FMT),	/* c */
	_NL_ITEM_INDEX(D_FMT),		/* x */
	_NL_ITEM_INDEX(T_FMT),		/* X */
	_NL_ITEM_INDEX(T_FMT_AMPM), /* r */
#ifdef ENABLE_ERA_CODE
	_NL_ITEM_INDEX(ERA_D_T_FMT), /* Ec */
	_NL_ITEM_INDEX(ERA_D_FMT),	/* Ex */
	_NL_ITEM_INDEX(ERA_T_FMT),	/* EX */
#endif
};

#define MAX_PUSH 4

char *__XL_NPP(strptime)(const char *__restrict buf, const char *__restrict format,
					 struct tm *__restrict tm   __LOCALE_PARAM)
{
	register const char *p;
	char *o;
	const char *stack[MAX_PUSH];
	int i, j, lvl;
	int fields[13];
	unsigned char mod;
	unsigned char code;

	i = 0;
	do {
		fields[i] = INT_MIN;
	} while (++i < 13);

	lvl = 0;
	p = format;

LOOP:
	if (!*p) {
		if (lvl == 0) {			/* Done. */
			if (fields[6] == 7) { /* Cleanup for %u here since just once. */
				fields[6] = 0;	/* Don't use mod in case unset. */
			}

			i = 0;
			do {				/* Store the values into tm. */
				if (fields[i] != INT_MIN) {
					((int *) tm)[i] = fields[i];
				}
			} while (++i < 8);

			return (char *) buf; /* Success. */
		}
		p = stack[--lvl];
		goto LOOP;
	}

	if ((*p == '%') && (*++p != '%')) {
		mod = ILLEGAL_SPEC;
		if ((*p == 'O') || (*p == 'E')) { /* Modifier? */
			mod |= ((*p == 'O') ? NO_O_MOD : NO_E_MOD);
			++p;
		}

		if (!*p
			|| (((unsigned char)(((*p) | 0x20) - 'a')) >= 26)
			|| (((code = spec[(int)(*p - 'A')]) & mod) >= ILLEGAL_SPEC)
			) {
			return NULL;		/* Illegal spec. */
		}

		if ((code & MASK_SPEC) == STACKED_SPEC) {
			if (lvl == MAX_PUSH) {
				return NULL;	/* Stack full so treat as illegal spec. */
			}
			stack[lvl++] = ++p;
			if ((code &= 0xf) < 8) {
				p = ((const char *) spec) + STACKED_STRINGS_START + code;
				p += *((unsigned char *)p);
				goto LOOP;
			}

			p = ((const char *) spec) + STACKED_STRINGS_NL_ITEM_START
				+ (code & 7);
#ifdef ENABLE_ERA_CODE
			if ((mod & NO_E_MOD) /* Actually, this means E modifier present. */
				&& (*(o = __XL_NPP(nl_langinfo)(_NL_ITEM(LC_TIME,
							(int)(((unsigned char *)p)[4]))
							__LOCALE_ARG
							)))
				) {
				p = o;
				goto LOOP;
			}
#endif
			p = __XL_NPP(nl_langinfo)(_NL_ITEM(LC_TIME,
							(int)(*((unsigned char *)p)))
							__LOCALE_ARG
							);
			goto LOOP;
		}

		++p;

		if ((code & MASK_SPEC) == STRING_SPEC) {
			code &= 0xf;
			j = spec[STRINGS_NL_ITEM_START + 3 + code];
			i = _NL_ITEM(LC_TIME, spec[STRINGS_NL_ITEM_START + code]);
			/* Go backwards to check full names before abreviations. */
			do {
				--j;
				o = __XL_NPP(nl_langinfo)(i+j   __LOCALE_ARG);
				if (!__XL_NPP(strncasecmp)(buf, o, strlen(o)   __LOCALE_ARG) && *o) {
					do {		/* Found a match. */
						++buf;
					} while (*++o);
					if (!code) { /* am/pm */
						fields[8] = j * 12;
						if (fields[9] >= 0) { /* We have a previous %I or %l. */
							fields[2] = fields[9] + fields[8];
						}
					} else {	/* day (4) or month (6) */
						fields[2 + (code << 1)]
							= j % (spec[STRINGS_NL_ITEM_START + 3 + code] >> 1);
					}
					goto LOOP;
				}
			} while (j);
			return NULL;		/* Failed to match. */
		}

		if ((code & MASK_SPEC) == CALC_SPEC) {
			if ((code &= 0xf) < 1) { /* s or z*/
				time_t t;

				o = (char *) buf;
				i = errno;
				__set_errno(0);
				if (!ISSPACE(*buf)) { /* Signal an error if whitespace. */
#ifdef TIME_T_IS_UNSIGNED
					t = __XL_NPP(strtoul)(buf, &o, 10   __LOCALE_ARG);
#else
					t = __XL_NPP(strtol)(buf, &o, 10   __LOCALE_ARG);
#endif
				}
				if ((o == buf) || errno) { /* Not a number or overflow. */
					return NULL;
				}
				__set_errno(i);	/* Restore errno. */
				buf = o;

				if (!code) {	/* s */
					localtime_r(&t, tm); /* TODO: check for failure? */
					i = 0;
					do {		/* Now copy values from tm to fields. */
						 fields[i] = ((int *) tm)[i];
					} while (++i < 8);
				}
			}
			/* TODO: glibc treats %Z as a nop.  For now, do the same. */
			goto LOOP;
		}

		assert((code & MASK_SPEC) == INT_SPEC);
		{
			register const unsigned char *x;
			code &= 0xf;
			x = spec + INT_FIELD_START + (code << 1);
			if ((j = x[1]) < 3) { /* upper bound (inclusive) */
				j = ((j==1) ? 366 : 9999);
			}
			i = -1;
			while (ISDIGIT(*buf)) {
				if (i < 0) {
					i = 0;
				}
				if ((i = 10*i + (*buf - '0')) > j) { /* Overflow. */
					return NULL;
				}
				++buf;
			}
			if (i < (*x & 1)) {	/* This catches no-digit case too. */
				return NULL;
			}
			if (*x & 2) {
				--i;
			}
			if (*x & 4) {
				i -= 1900;
			}

			if (*x == (9 << 3) + 1 + 0) { /* %I or %l */
				if (i == 12) {
					i = 0;
				}
				if (fields[8] >= 0) { /* We have a previous %p or %P. */
					fields[2] = i + fields[8];
				}
			}

			fields[(*x) >> 3] = i;

			if (((unsigned char)(*x - (10 << 3) + 0 + 0)) <= 8) { /* %C or %y */
				if ((j = fields[10]) < 0) {	/* No %C, so i must be %y data. */
					if (i <= 68) { /* Map [0-68] to 2000+i */
						i += 100;
					}
				} else {		/* Have %C data, but what about %y? */
					if ((i = fields[11]) < 0) {	/* No %y data. */
						i = 0;	/* Treat %y val as 0 following glibc's example. */
					}
					i += 100*(j - 19);
				}
				fields[5] = i;
			}
		}
		goto LOOP;
	} else if (ISSPACE(*p)) {
		++p;
		while (ISSPACE(*buf)) {
			++buf;
		}
		goto LOOP;
	} else if (*buf++ == *p++) {
		goto LOOP;
	}
	return NULL;
}
# ifdef L_strptime_l
libc_hidden_def(strptime_l)
# endif

#endif /* defined(__UCLIBC_HAS_XLOCALE__) && !defined(__UCLIBC_DO_XLOCALE) */

#endif
/**********************************************************************/
#ifdef L_time

#ifndef __BCC__
#error The uClibc version of time is in sysdeps/linux/common.
#endif

time_t time(register time_t *tloc)
{
	struct timeval tv;
	register struct timeval *p = &tv;

	gettimeofday(p, NULL);		/* This should never fail... */

	if (tloc) {
		*tloc = p->tv_sec;
	}

	return p->tv_sec;
}

#endif
/**********************************************************************/
#ifdef L_tzset

static const char vals[] = {
	'T', 'Z', 0,				/* 3 */
	'U', 'T', 'C', 0,			/* 4 */
	25, 60, 60, 1,				/* 4 */
	'.', 1,						/* M */
	5, '.', 1,
	6,  0,  0,					/* Note: overloaded for non-M non-J case... */
	0, 1, 0,					/* J */
	',', 'M',      '4', '.', '1', '.', '0',
	',', 'M', '1', '0', '.', '5', '.', '0', 0,
	',', 'M',      '3', '.', '2', '.', '0',
	',', 'M', '1', '1', '.', '1', '.', '0', 0
};

#define TZ    vals
#define UTC   (vals + 3)
#define RANGE (vals + 7)
#define RULE  (vals + 11 - 1)
#define DEFAULT_RULES (vals + 22)
#define DEFAULT_2007_RULES (vals + 38)

/* Initialize to UTC. */
int daylight = 0;
long timezone = 0;
char *tzname[2] = { (char *) UTC, (char *) (UTC-1) };

__UCLIBC_MUTEX_INIT(_time_tzlock, PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP);

rule_struct _time_tzinfo[2];

static const char *getoffset(register const char *e, long *pn)
{
	register const char *s = RANGE-1;
	long n;
	int f;

	n = 0;
	f = -1;
	do {
		++s;
		if (__isdigit_char(*e)) {
			f = *e++ - '0';
		}
		if (__isdigit_char(*e)) {
			f = 10 * f + (*e++ - '0');
		}
		if (((unsigned int)f) >= *s) {
			return NULL;
		}
		n = (*s) * n + f;
		f = 0;
		if (*e == ':') {
			++e;
			--f;
		}
	} while (*s > 1);

	*pn = n;
	return e;
}

static const char *getnumber(register const char *e, int *pn)
{
#ifdef __BCC__
	/* bcc can optimize the counter if it thinks it is a pointer... */
	register const char *n = (const char *) 3;
	int f;

	f = 0;
	while (n && __isdigit_char(*e)) {
		f = 10 * f + (*e++ - '0');
		--n;
	}

	*pn = f;
	return (n == (const char *) 3) ? NULL : e;
#else  /* __BCC__ */
	int n, f;

	n = 3;
	f = 0;
	while (n && __isdigit_char(*e)) {
		f = 10 * f + (*e++ - '0');
		--n;
	}

	*pn = f;
	return (n == 3) ? NULL : e;
#endif /* __BCC__ */
}


#ifdef __UCLIBC_HAS_TZ_FILE__

#ifndef __UCLIBC_HAS_TZ_FILE_READ_MANY__
static smallint TZ_file_read;		/* Let BSS initialization set this to 0. */
#endif

static char *read_TZ_file(char *buf)
{
	int r;
	int fd;
	char *p = NULL;

	fd = open(__UCLIBC_TZ_FILE_PATH__, O_RDONLY);
	if (fd >= 0) {
#if 0
		/* TZ are small *files*. On files, short reads
		 * only occur on EOF (unlike, say, pipes).
		 * The code below is pedanticallly more correct,
		 * but this way we always read at least twice:
		 * 1st read is short, 2nd one is zero bytes.
		 */
		size_t todo = TZ_BUFLEN;
		p = buf;
		do {
			r = read(fd, p, todo);
			if (r < 0)
				goto ERROR;
			if (r == 0)
				break;
			p += r;
			todo -= r;
		} while (todo);
#else
		/* Shorter, and does one fewer read syscall */
		r = read(fd, buf, TZ_BUFLEN);
		if (r < 0)
			goto ERROR;
		p = buf + r;
#endif
		if ((p > buf) && (p[-1] == '\n')) { /* Must end with newline */
			p[-1] = 0;
			p = buf;
#ifndef __UCLIBC_HAS_TZ_FILE_READ_MANY__
			TZ_file_read = 1;
#endif
		} else {
ERROR:
			p = NULL;
		}
		close(fd);
	}
#ifdef __UCLIBC_FALLBACK_TO_ETC_LOCALTIME__
	else {
		fd = open("/etc/localtime", O_RDONLY);
		if (fd >= 0) {
			r = read(fd, buf, TZ_BUFLEN);
			if (r != TZ_BUFLEN
			 || strncmp(buf, "TZif", 4) != 0
			 || (unsigned char)buf[4] < 2
			 || lseek(fd, -TZ_BUFLEN, SEEK_END) < 0
			) {
				goto ERROR;
			}
			/* tzfile.h from tzcode database says about TZif2+ files:
			**
			** If tzh_version is '2' or greater, the above is followed by a second instance
			** of tzhead and a second instance of the data in which each coded transition
			** time uses 8 rather than 4 chars,
			** then a POSIX-TZ-environment-variable-style string for use in handling
			** instants after the last transition time stored in the file
			** (with nothing between the newlines if there is no POSIX representation for
			** such instants).
			*/
			r = read(fd, buf, TZ_BUFLEN);
			if (r <= 0 || buf[--r] != '\n')
				goto ERROR;
			buf[r] = 0;
			while (r != 0) {
				if (buf[--r] == '\n') {
					p = buf + r + 1;
#ifndef __UCLIBC_HAS_TZ_FILE_READ_MANY__
					TZ_file_read = 1;
#endif
					break;
				}
			} /* else ('\n' not found): p remains NULL */
			close(fd);
		}
	}
#endif /* __UCLIBC_FALLBACK_TO_ETC_LOCALTIME__ */
	return p;
}

#endif /* __UCLIBC_HAS_TZ_FILE__ */

void tzset(void)
{
	_time_tzset((time(NULL)) < new_rule_starts);
}

void _time_tzset(int use_old_rules)
{
	register const char *e;
	register char *s;
	long off = 0;
	short *p;
	rule_struct new_rules[2];
	int n, count, f;
	char c;
#ifdef __UCLIBC_HAS_TZ_FILE__
	char buf[TZ_BUFLEN];
#endif
#ifdef __UCLIBC_HAS_TZ_CACHING__
	static char oldval[TZ_BUFLEN]; /* BSS-zero'd. */
#endif

	/* Put this inside the lock to prevent the possibility of two different
	 * timezones being used in a threaded app. */
	__UCLIBC_MUTEX_LOCK(_time_tzlock);

	e = getenv(TZ);				/* TZ env var always takes precedence. */

#if defined(__UCLIBC_HAS_TZ_FILE__) && !defined(__UCLIBC_HAS_TZ_FILE_READ_MANY__)
	if (e) {
		/* Never use TZfile if TZ env var is set. */
		TZ_file_read = 0;
	}
	if (TZ_file_read) {
		/* We already parsed TZfile before, skip everything. */
		goto FAST_DONE;
	}
#endif

	/* Warning!!!  Since uClibc doesn't do lib locking, the following is
	 * potentially unsafe in a multi-threaded program since it is remotely
	 * possible that another thread could call setenv() for TZ and overwrite
	 * the string being parsed.  So, don't do that... */

#ifdef __UCLIBC_HAS_TZ_FILE__
	if (!e)
		e = read_TZ_file(buf);
#endif
	if (!e		/* TZ env var not set and no TZfile (or bad TZfile) */
	 || !*e		/* or set to empty string. */
	) {
		goto ILLEGAL;
	}

	if (*e == ':') {			/* Ignore leading ':'. */
		++e;
	}

#ifdef __UCLIBC_HAS_TZ_CACHING__
	if (strcmp(e, oldval) == 0) {
		/* Same string as last time... nothing to do. */
		goto FAST_DONE;
	}
	/* Make a copy of the TZ env string.  It won't be nul-terminated if
	 * it is too long, but it that case it will be illegal and will be reset
	 * to the empty string anyway. */
	strncpy(oldval, e, TZ_BUFLEN);
#endif

	count = 0;
	new_rules[1].tzname[0] = 0;
LOOP:
	/* Get std or dst name. */
	c = 0;
	if (*e == '<') {
		++e;
		c = '>';
	}

	s = new_rules[count].tzname;
	n = 0;
	while (*e
	    && isascii(*e)		/* SUSv3 requires char in portable char set. */
	    && (isalpha(*e)
		|| (c && (isalnum(*e) || (*e == '+') || (*e == '-')))
	       )
	) {
		*s++ = *e++;
		if (++n > TZNAME_MAX) {
			goto ILLEGAL;
		}
	}
	*s = 0;

	if ((n < 3)					/* Check for minimum length. */
	 || (c && (*e++ != c))	/* Match any quoting '<'. */
	) {
		goto ILLEGAL;
	}

	/* Get offset */
	s = (char *) e;
	if ((*e != '-') && (*e != '+')) {
		if (count && !__isdigit_char(*e)) {
			off -= 3600;		/* Default to 1 hour ahead of std. */
			goto SKIP_OFFSET;
		}
		--e;
	}

	++e;
	e = getoffset(e, &off);
	if (!e) {
		goto ILLEGAL;
	}

	if (*s == '-') {
		off = -off;				/* Save off in case needed for dst default. */
	}
SKIP_OFFSET:
	new_rules[count].gmt_offset = off;

	if (!count) {
		new_rules[1].gmt_offset = off; /* Shouldn't be needed... */
		if (*e) {
			++count;
			goto LOOP;
		}
	} else {					/* OK, we have dst, so get some rules. */
		count = 0;
		if (!*e) {				/* No rules so default to US rules. */
			e = use_old_rules ? DEFAULT_RULES : DEFAULT_2007_RULES;
#ifdef DEBUG_TZSET
			if (e == DEFAULT_RULES)
				printf("tzset: Using old rules.\n");
			else if (e == DEFAULT_2007_RULES)
				printf("tzset: Using new rules\n");
			else
				printf("tzset: Using undefined rules\n");
#endif
		}

		do {
			if (*e++ != ',') {
				goto ILLEGAL;
			}

			n = 365;
			s = (char *) RULE;
			c = *e++;
			if (c == 'M') {
				n = 12;
			} else if (c == 'J') {
				s += 8;
			} else {
				--e;
				c = 0;
				s += 6;
			}

			p = &new_rules[count].rule_type;
			*p = c;
			if (c != 'M') {
				p -= 2;
			}

			do {
				++s;
				e = getnumber(e, &f);
				if (!e
				 || ((unsigned int)(f - s[1]) > n)
				 || (*s && (*e++ != *s))
				) {
					goto ILLEGAL;
				}
				*--p = f;
				s += 2;
				n = *s;
			} while (n > 0);

			off = 2 * 60 * 60;	/* Default to 2:00:00 */
			if (*e == '/') {
				++e;
				e = getoffset(e, &off);
				if (!e) {
					goto ILLEGAL;
				}
			}
			new_rules[count].dst_offset = off;
		} while (++count < 2);

		if (*e) {
ILLEGAL:
#ifdef __UCLIBC_HAS_TZ_CACHING__
			oldval[0] = 0; /* oldval = "" */
#endif
			memset(_time_tzinfo, 0, sizeof(_time_tzinfo));
			strcpy(_time_tzinfo[0].tzname, UTC);
			goto DONE;
		}
	}

	memcpy(_time_tzinfo, new_rules, sizeof(new_rules));
DONE:
	tzname[0] = _time_tzinfo[0].tzname;
	tzname[1] = _time_tzinfo[1].tzname;
	daylight = !!_time_tzinfo[1].tzname[0];
	timezone = _time_tzinfo[0].gmt_offset;

#if (defined(__UCLIBC_HAS_TZ_FILE__) && !defined(__UCLIBC_HAS_TZ_FILE_READ_MANY__)) || \
	defined(__UCLIBC_HAS_TZ_CACHING__)
FAST_DONE:
#endif
	__UCLIBC_MUTEX_UNLOCK(_time_tzlock);
}
libc_hidden_def(tzset)
#endif
/**********************************************************************/
/*  #ifdef L_utime */

/* utime is a syscall in both linux and elks. */
/*  int utime(const char *path, const struct utimbuf *times) */

/*  #endif */
/**********************************************************************/
/* Non-SUSv3 */
/**********************************************************************/
#ifdef L_utimes

#ifndef __BCC__
#error The uClibc version of utimes is in sysdeps/linux/common.
#endif

#include <utime.h>

int utimes(const char *filename, register const struct timeval *tvp)
{
	register struct utimbuf *p = NULL;
	struct utimbuf utb;

	if (tvp) {
		p = &utb;
		p->actime = tvp[0].tv_sec;
		p->modtime = tvp[1].tv_sec;
	}
	return utime(filename, p);
}

#endif
/**********************************************************************/
#ifdef L__time_t2tm

static const uint16_t _vals[] = {
	60, 60, 24, 7 /* special */, 36524, 1461, 365, 0
};

static const unsigned char days[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, /* non-leap */
	    29,
};

#ifdef __UCLIBC_HAS_TM_EXTENSIONS__
static const char utc_string[] = "UTC";
#endif

/* Notes:
 * If time_t is 32 bits, then no overflow is possible.
 * It time_t is > 32 bits, this needs to be adjusted to deal with overflow.
 */

/* Note: offset is the correction in _days_ to *timer! */

struct tm attribute_hidden *_time_t2tm(const time_t *__restrict timer,
					  int offset, struct tm *__restrict result)
{
	register int *p;
	time_t t1, t, v;
	int wday = wday; /* ok to be uninitialized, shutting up warning */

	{
		register const uint16_t *vp;
		t = *timer;
		p = (int *) result;
		p[7] = 0;
		vp = _vals;
		do {
			if ((v = *vp) == 7) {
				/* Overflow checking, assuming time_t is long int... */
#if (LONG_MAX > INT_MAX) && (LONG_MAX > 2147483647L)
#if (INT_MAX == 2147483647L) && (LONG_MAX == 9223372036854775807L)
				/* Valid range for t is [-784223472856L, 784223421720L].
				 * Outside of this range, the tm_year field will overflow. */
				if (((unsigned long)(t + offset- -784223472856L))
					> (784223421720L - -784223472856L)
					) {
					return NULL;
				}
#else
#error overflow conditions unknown
#endif
#endif

				/* We have days since the epoch, so caluclate the weekday. */
#if defined(__BCC__) && TIME_T_IS_UNSIGNED
				wday = (t + 4) % (*vp);	/* t is unsigned */
#else
				wday = ((int)((t % (*vp)) + 11)) % ((int)(*vp)); /* help bcc */
#endif
				/* Set divisor to days in 400 years.  Be kind to bcc... */
				v = ((time_t)(vp[1])) << 2;
				++v;
				/* Change to days since 1/1/1601 so that for 32 bit time_t
				 * values, we'll have t >= 0.  This should be changed for
				 * archs with larger time_t types.
				 * Also, correct for offset since a multiple of 7. */

				/* TODO: Does this still work on archs with time_t > 32 bits? */
				t += (135140L - 366) + offset; /* 146097 - (365*30 + 7) -366 */
			}
#if defined(__BCC__) && TIME_T_IS_UNSIGNED
			t -= ((t1 = t / v) * v);
#else
			if ((t -= ((t1 = t / v) * v)) < 0) {
				t += v;
				--t1;
			}
#endif

			if ((*vp == 7) && (t == v-1)) {
				--t;			/* Correct for 400th year leap case */
				++p[4];			/* Stash the extra day... */
			}

#if defined(__BCC__) && 0
			*p = t1;
			if (v <= 60) {
				*p = t;
				t = t1;
			}
			++p;
#else
			if (v <= 60) {
				*p++ = t;
				t = t1;
			} else {
				*p++ = t1;
			}
#endif
		} while (*++vp);
	}

	if (p[-1] == 4) {
		--p[-1];
		t = 365;
	}

	*p += ((int) t);			/* result[7] .. tm_yday */

	p -= 2;						/* at result[5] */

#if (LONG_MAX > INT_MAX) && (LONG_MAX > 2147483647L)
	/* Protect against overflow.  TODO: Unecessary if int arith wraps? */
	*p = ((((p[-2]<<2) + p[-1])*25 + p[0])<< 2) + (p[1] - 299); /* tm_year */
#else
	*p = ((((p[-2]<<2) + p[-1])*25 + p[0])<< 2) + p[1] - 299; /* tm_year */
#endif

	p[1] = wday;				/* result[6] .. tm_wday */

	{
		register const unsigned char *d = days;

		wday = 1900 + *p;
		if (__isleap(wday)) {
			d += 11;
		}

		wday = p[2] + 1;		/* result[7] .. tm_yday */
		*--p = 0;				/* at result[4] .. tm_mon */
		while (wday > *d) {
			wday -= *d;
			if (*d == 29) {
				d -= 11;		/* Backup to non-leap Feb. */
			}
			++d;
			++*p;				/* Increment tm_mon. */
		}
		p[-1] = wday;			/* result[3] .. tm_mday */
	}
	/* TODO -- should this be 0? */
	p[4] = 0;					/* result[8] .. tm_isdst */
#ifdef __UCLIBC_HAS_TM_EXTENSIONS__
# ifdef __USE_BSD
	result->tm_gmtoff = 0;
	result->tm_zone = utc_string;
# else
	result->__tm_gmtoff = 0;
	result->__tm_zone = utc_string;
# endif
#endif /* __UCLIBC_HAS_TM_EXTENSIONS__ */

	return result;
}

#endif
/**********************************************************************/
#ifdef L___time_tm

struct tm __time_tm;	/* Global shared by gmtime() and localtime(). */

#endif
/**********************************************************************/
#ifdef L__time_mktime

time_t attribute_hidden _time_mktime(struct tm *timeptr, int store_on_success)
{
	time_t t;

	__UCLIBC_MUTEX_LOCK(_time_tzlock);

	tzset();

	t = _time_mktime_tzi(timeptr, store_on_success, _time_tzinfo);

	__UCLIBC_MUTEX_UNLOCK(_time_tzlock);

	return t;
}

#endif
/**********************************************************************/
#ifdef L__time_mktime_tzi

static const unsigned char __vals[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, /* non-leap */
	    29,
};

time_t attribute_hidden _time_mktime_tzi(struct tm *timeptr, int store_on_success,
						rule_struct *tzi)
{
#ifdef __BCC__
	long days, secs;
#else
	long long secs;
#endif
	time_t t;
	struct tm x;
	/* 0:sec  1:min  2:hour  3:mday  4:mon  5:year  6:wday  7:yday  8:isdst */
	register int *p = (int *) &x;
	register const unsigned char *s;
	int d, default_dst;

	memcpy(p, timeptr, sizeof(struct tm));

	if (!tzi[1].tzname[0]) { /* No dst in this timezone, */
		p[8] = 0;				/* so set tm_isdst to 0. */
	}

	default_dst = 0;
	if (p[8]) {					/* Either dst or unknown? */
		default_dst = 1;		/* Assume advancing (even if unknown). */
		p[8] = ((p[8] > 0) ? 1 : -1); /* Normalize so abs() <= 1. */
	}

	d = 400;
	p[5] = (p[5] - ((p[6] = p[5]/d) * d)) + (p[7] = p[4]/12);
	if ((p[4] -= 12 * p[7]) < 0) {
		p[4] += 12;
		--p[5];
	}

	s = __vals;
	d = (p[5] += 1900);			/* Correct year.  Now between 1900 and 2300. */
	if (__isleap(d)) {
		s += 11;
	}

	p[7] = 0;
	d = p[4];
	while (d) {
		p[7] += *s;
		if (*s == 29) {
			s -= 11;			/* Backup to non-leap Feb. */
		}
		++s;
		--d;
	}

	_time_tzset (x.tm_year < 2007);	/* tm_year was expanded above */

#ifdef __BCC__
	d = p[5] - 1;
	days = -719163L + ((long)d)*365 + ((d/4) - (d/100) + (d/400) + p[3] + p[7]);
	secs = p[0] + 60*( p[1] + 60*((long)(p[2])) )
		+ tzi[default_dst].gmt_offset;
DST_CORRECT:
	if (secs < 0) {
		secs += 120009600L;
		days -= 1389;
	}
	if ( ((unsigned long)(days + secs/86400L)) > 49710L) {
		t = ((time_t)(-1));
		goto DONE;
	}
	secs += (days * 86400L);
#else
	d = p[5] - 1;
	d = -719163L + d*365 + (d/4) - (d/100) + (d/400);
	secs = p[0]
		+ tzi[default_dst].gmt_offset
		+ 60*( p[1]
			   + 60*(p[2]
					 + 24*(((146073L * ((long long)(p[6])) + d)
							+ p[3]) + p[7])));

DST_CORRECT:
	if (((unsigned long long)(secs - LONG_MIN))
		> (((unsigned long long)LONG_MAX) - LONG_MIN)
		) {
		t = ((time_t)(-1));
		goto DONE;
	}
#endif

	d = ((struct tm *)p)->tm_isdst;
	t = secs;

	__time_localtime_tzi(&t, (struct tm *)p, tzi);

	if (t == ((time_t)(-1))) {	/* Remember, time_t can be unsigned. */
		goto DONE;
	}

	if ((d < 0) && (((struct tm *)p)->tm_isdst != default_dst)) {
#ifdef __BCC__
		secs -= (days * 86400L);
#endif
		secs += (tzi[1-default_dst].gmt_offset
				 - tzi[default_dst].gmt_offset);
		goto DST_CORRECT;
	}


	if (store_on_success) {
		memcpy(timeptr, p, sizeof(struct tm));
	}


DONE:
	return t;
}

#endif
/**********************************************************************/
#if (defined(L_wcsftime) || defined(L_wcsftime_l))

/* Implemented via strftime / strftime_l wchar_t variants */

#endif
/**********************************************************************/
#ifdef L_dysize
/* Return the number of days in YEAR.  */

int dysize(int year)
{
	return __isleap(year) ? 366 : 365;
}

#endif
/**********************************************************************/
