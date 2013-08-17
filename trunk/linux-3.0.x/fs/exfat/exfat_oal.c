/* Some of the source code in this file came from "linux/fs/fat/misc.c".  */
/*
 *  linux/fs/fat/misc.c
 *
 *  Written 1992,1993 by Werner Almesberger
 *  22/11/2000 - Fixed fat_date_unix2dos for dates earlier than 01/01/1980
 *         and date_dos2unix for date==0 by Igor Zhbanov(bsg@uniyar.ac.ru)
 */

/*
 *  Copyright (C) 2012-2013 Samsung Electronics Co., Ltd.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <linux/semaphore.h>
#include <linux/time.h>

#include "exfat_config.h"
#include "exfat_global.h"
#include "exfat_api.h"
#include "exfat_oal.h"

DECLARE_MUTEX(z_sem);

INT32 sm_init(struct semaphore *sm)
{
	sema_init(sm, 1);
	return(0);
}

INT32 sm_P(struct semaphore *sm)
{
	down(sm);
	return 0;
}

void sm_V(struct semaphore *sm)
{
	up(sm);
}

extern struct timezone sys_tz;

#define UNIX_SECS_1980   315532800L

#if BITS_PER_LONG == 64
#define UNIX_SECS_2108   4354819200L
#endif

#define DAYS_DELTA_DECADE	(365 * 10 + 2)
#define NO_LEAP_YEAR_2100    (120)
#define IS_LEAP_YEAR(y)  (!((y) & 3) && (y) != NO_LEAP_YEAR_2100)

#define SECS_PER_MIN     (60)
#define SECS_PER_HOUR    (60 * SECS_PER_MIN)
#define SECS_PER_DAY     (24 * SECS_PER_HOUR)

#define MAKE_LEAP_YEAR(leap_year, year)                         \
	do {                                                    \
		if (unlikely(year > NO_LEAP_YEAR_2100))         \
			leap_year = ((year + 3) / 4) - 1;       \
		else                                            \
			leap_year = ((year + 3) / 4);           \
	} while(0)



static time_t accum_days_in_year[] = {
        0,   0, 31, 59, 90,120,151,181,212,243,273,304,334, 0, 0, 0,
};


TIMESTAMP_T *tm_current(TIMESTAMP_T *tp)
{
	struct timespec ts = CURRENT_TIME_SEC;
	time_t second = ts.tv_sec;
	time_t day, leap_day, month, year;

	second -= sys_tz.tz_minuteswest * SECS_PER_MIN;

	if (second < UNIX_SECS_1980) {
		tp->sec  = 0;
		tp->min  = 0;
		tp->hour = 0;
		tp->day  = 1;
		tp->mon  = 1;
		tp->year = 0;
		return(tp);
	}
#if BITS_PER_LONG == 64
	if (second >= UNIX_SECS_2108) {
		tp->sec  = 59;
		tp->min  = 59;
		tp->hour = 23;
		tp->day  = 31;
		tp->mon  = 12;
		tp->year = 127;
		return(tp);
	}
#endif

	day = second / SECS_PER_DAY - DAYS_DELTA_DECADE;
	year = day / 365;

	MAKE_LEAP_YEAR(leap_day, year);
	if (year * 365 + leap_day > day)
		year--;

	MAKE_LEAP_YEAR(leap_day, year);

	day -= year * 365 + leap_day;

	if (IS_LEAP_YEAR(year) && day == accum_days_in_year[3]) {
		month = 2;
	} else {
		if (IS_LEAP_YEAR(year) && day > accum_days_in_year[3])
			day--;
		for (month = 1; month < 12; month++) {
			if (accum_days_in_year[month + 1] > day)
				break;
		}
	}
	day -= accum_days_in_year[month];

	tp->sec  = second % SECS_PER_MIN;
	tp->min  = (second / SECS_PER_MIN) % 60;
	tp->hour = (second / SECS_PER_HOUR) % 24;
	tp->day  = day + 1;
	tp->mon  = month;
	tp->year = year;

	return(tp);
}
