#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/* We use this instead of memcmp because some broken C libraries
 * add additional nonstandard fields to struct tm... */

int tm_cmp(struct tm tm1, struct tm tm2)
{
	return  tm1.tm_sec  != tm2.tm_sec  ||
		tm1.tm_min  != tm2.tm_min  ||
		tm1.tm_hour != tm2.tm_hour ||
		tm1.tm_mday != tm2.tm_mday ||
		tm1.tm_mon  != tm2.tm_mon  ||
		tm1.tm_year != tm2.tm_year ||
		tm1.tm_wday != tm2.tm_wday ||
		tm1.tm_yday != tm2.tm_yday ||
		tm1.tm_isdst!= tm2.tm_isdst;
}

char *tm_str(struct tm tm)
{
	static int i;
	static char b[4][64];
	i = (i+1)%4;
	snprintf(b[i], sizeof b[i],
		"s=%.2d m=%.2d h=%.2d mday=%.2d mon=%.2d year=%.4d wday=%d yday=%d isdst=%d",
		tm.tm_sec, tm.tm_min, tm.tm_hour,
		tm.tm_mday, tm.tm_mon, tm.tm_year,
		tm.tm_wday, tm.tm_yday, tm.tm_isdst);
	return b[i];
}

#define TM(ss,mm,hh,md,mo,yr,wd,yd,dst) (struct tm){ \
	.tm_sec = ss, .tm_min = mm, .tm_hour = hh,    \
	.tm_mday = md, .tm_mon = mo, .tm_year = yr,    \
	.tm_wday = wd, .tm_yday = yd, .tm_isdst = dst }

#define TM_EPOCH    TM(0,0,0,1,0,70,4,0,0)
#define TM_Y2038_1S TM(7,14,3,19,0,138,2,18,0)
#define TM_Y2038    TM(8,14,3,19,0,138,2,18,0)

#define TEST_TM(r,x,m) (!tm_cmp((r),(x)) || \
(printf(__FILE__ ":%d: %s failed:\n\tresult: %s\n\texpect: %s\n", __LINE__, \
m, tm_str((r)), tm_str((x))), err++, 0) )

#define TEST(r, f, x, m) ( \
((r) = (f)) == (x) || \
(printf(__FILE__ ":%d: %s failed (" m ")\n", __LINE__, #f, r, x), err++, 0) )

int main(void)
{
	struct tm tm, *tm_p;
	time_t t;
	int err=0;

	putenv("TZ=GMT");
	tzset();

	t=0; tm_p = gmtime(&t);
	TEST_TM(*tm_p, TM_EPOCH, "gmtime(0)");

	tm = TM_Y2038_1S;
	t = mktime(&tm);
	tm = *(gmtime(&t));
	TEST_TM(*tm_p, TM_Y2038_1S, "mktime/gmtime(Y2038-1)");

	tm = TM_Y2038;
	t = mktime(&tm);
	tm = *(gmtime(&t));
	TEST_TM(*tm_p, TM_Y2038, "mktime/gmtime(Y2038)");

	/* FIXME: set a TZ var and check DST boundary conditions */

	return err;
}
