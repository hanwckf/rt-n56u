/* vi: set sw=4 ts=4: */
/*
 * Mini hwclock implementation for busybox
 *
 * Copyright (C) 2002 Robert Griebl <griebl@gmx.de>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
//config:config HWCLOCK
//config:	bool "hwclock (5.8 kb)"
//config:	default y
//config:	help
//config:	The hwclock utility is used to read and set the hardware clock
//config:	on a system. This is primarily used to set the current time on
//config:	shutdown in the hardware clock, so the hardware will keep the
//config:	correct time when Linux is _not_ running.
//config:
//config:config FEATURE_HWCLOCK_ADJTIME_FHS
//config:	bool "Use FHS /var/lib/hwclock/adjtime"
//config:	default n  # util-linux-ng in Fedora 13 still uses /etc/adjtime
//config:	depends on HWCLOCK
//config:	help
//config:	Starting with FHS 2.3, the adjtime state file is supposed to exist
//config:	at /var/lib/hwclock/adjtime instead of /etc/adjtime. If you wish
//config:	to use the FHS behavior, answer Y here, otherwise answer N for the
//config:	classic /etc/adjtime path.
//config:
//config:	pathname.com/fhs/pub/fhs-2.3.html#VARLIBHWCLOCKSTATEDIRECTORYFORHWCLO

//applet:IF_HWCLOCK(APPLET(hwclock, BB_DIR_SBIN, BB_SUID_DROP))

//kbuild:lib-$(CONFIG_HWCLOCK) += hwclock.o

#include "libbb.h"
/* After libbb.h, since it needs sys/types.h on some systems */
#include <sys/utsname.h>
#include "rtc_.h"


//musl has no __MUSL__ or similar define to check for,
//but its <sys/types.h> has these lines:
// #define __NEED_fsblkcnt_t
// #define __NEED_fsfilcnt_t
#if defined(__linux__) && defined(__NEED_fsblkcnt_t) && defined(__NEED_fsfilcnt_t)
# define LIBC_IS_MUSL 1
# include <sys/syscall.h>
#else
# define LIBC_IS_MUSL 0
#endif


/* diff code is disabled: it's not sys/hw clock diff, it's some useless
 * "time between hwclock was started and we saw CMOS tick" quantity.
 * It's useless since hwclock is started at a random moment,
 * thus the quantity is also random, useless. Showing 0.000000 does not
 * deprive us from any useful info.
 *
 * SHOW_HWCLOCK_DIFF code in this file shows the difference between system
 * and hw clock. It is useful, but not compatible with standard hwclock.
 * Thus disabled.
 */
#define SHOW_HWCLOCK_DIFF 0


#if !SHOW_HWCLOCK_DIFF
# define read_rtc(pp_rtcname, sys_tv, utc) read_rtc(pp_rtcname, utc)
#endif
static time_t read_rtc(const char **pp_rtcname, struct timeval *sys_tv, int utc)
{
	struct tm tm_time;
	int fd;

	fd = rtc_xopen(pp_rtcname, O_RDONLY);

	rtc_read_tm(&tm_time, fd);

#if SHOW_HWCLOCK_DIFF
	{
		int before = tm_time.tm_sec;
		while (1) {
			rtc_read_tm(&tm_time, fd);
			xgettimeofday(sys_tv);
			if (before != (int)tm_time.tm_sec)
				break;
		}
	}
#endif

	if (ENABLE_FEATURE_CLEAN_UP)
		close(fd);

	return rtc_tm2time(&tm_time, utc);
}

static void show_clock(const char **pp_rtcname, int utc)
{
#if SHOW_HWCLOCK_DIFF
	struct timeval sys_tv;
#endif
	time_t t = read_rtc(pp_rtcname, &sys_tv, utc);

#if ENABLE_LOCALE_SUPPORT
	/* Standard hwclock uses locale-specific output format */
	char cp[64];
	struct tm *ptm = localtime(&t);
	strftime(cp, sizeof(cp), "%c", ptm);
#else
	char *cp = ctime(&t);
	chomp(cp);
#endif

#if !SHOW_HWCLOCK_DIFF
	printf("%s  0.000000 seconds\n", cp);
#else
	{
		long diff = sys_tv.tv_sec - t;
		if (diff < 0 /*&& tv.tv_usec != 0*/) {
			/* Why we need diff++? */
			/* diff >= 0 is ok: | diff < 0, can't just use tv.tv_usec: */
			/*   45.520820      |   43.520820 */
			/* - 44.000000      | - 45.000000 */
			/* =  1.520820      | = -1.479180, not -2.520820! */
			diff++;
			/* Should be 1000000 - tv.tv_usec, but then we must check tv.tv_usec != 0 */
			sys_tv.tv_usec = 999999 - sys_tv.tv_usec;
		}
		printf("%s  %ld.%06lu seconds\n", cp, diff, (unsigned long)sys_tv.tv_usec);
	}
#endif
}

static void set_kernel_tz(const struct timezone *tz)
{
#if LIBC_IS_MUSL
	/* musl libc does not pass tz argument to syscall
	 * because "it's deprecated by POSIX, therefore it's fine
	 * if we gratuitously break stuff" :(
	 */
#if !defined(SYS_settimeofday) && defined(SYS_settimeofday_time32)
# define SYS_settimeofday SYS_settimeofday_time32
#endif
	int ret = syscall(SYS_settimeofday, NULL, tz);
#else
	int ret = settimeofday(NULL, tz);
#endif
	if (ret)
		bb_simple_perror_msg_and_die("settimeofday");
}

/*
 * At system boot, kernel may set system time from RTC,
 * but it knows nothing about timezones. If RTC is in local time,
 * then system time is wrong - it is offset by timezone.
 * --systz option corrects system time if RTC is in local time,
 * and (always) sets in-kernel timezone.
 * (Unlike --hctosys, it does not read the RTC).
 *
 * util-linux's code has this comment:
 *  RTC   | settimeofday calls
 *  ------|-------------------------------------------------
 *  Local | 1st) warps system time*, sets PCIL* and kernel tz
 *  UTC   | 1st) locks warp_clock 2nd) sets kernel tz
 *               * only on first call after boot
 * (PCIL is "persistent_clock_is_local" kernel internal flag,
 * it makes kernel save RTC in local time, not UTC.)
 */
static void set_kernel_timezone_and_clock(int utc, const struct timeval *hctosys)
{
	time_t cur;
	struct tm *broken;
	struct timezone tz = { 0 };

	/* if --utc, prevent kernel's warp_clock() with a dummy call */
	if (utc)
		set_kernel_tz(&tz);

	/* Set kernel's timezone offset based on userspace one */
//It's tempting to call tzset() and use libc global "timezone" variable
//...but it does NOT include DST shift (IOW: it's WRONG, usually by one hour,
//if DST is in effect!) Thus this ridiculous dance:
	cur = time(NULL);
	broken = localtime(&cur);
	tz.tz_minuteswest = -broken->tm_gmtoff / 60;
	/*tz.tz_dsttime = 0; already is */
	set_kernel_tz(&tz); /* MIGHT warp_clock() if 1st call since boot */

	if (hctosys) /* it's --hctosys: set time too */
		xsettimeofday(hctosys);
}

static void to_sys_clock(const char **pp_rtcname, int utc)
{
	struct timeval tv;

	tv.tv_sec = read_rtc(pp_rtcname, NULL, utc);
	tv.tv_usec = 0;
	return set_kernel_timezone_and_clock(utc, &tv);
}

static void from_sys_clock(const char **pp_rtcname, int utc)
{
#if 1
	struct timeval tv;
	struct tm tm_time;
	int rtc;

	rtc = rtc_xopen(pp_rtcname, O_WRONLY);
	xgettimeofday(&tv);
	/* Prepare tm_time */
	if (sizeof(time_t) == sizeof(tv.tv_sec)) {
		if (utc)
			gmtime_r((time_t*)&tv.tv_sec, &tm_time);
		else
			localtime_r((time_t*)&tv.tv_sec, &tm_time);
	} else {
		time_t t = tv.tv_sec;
		if (utc)
			gmtime_r(&t, &tm_time);
		else
			localtime_r(&t, &tm_time);
	}
#else
/* Bloated code which tries to set hw clock with better precision.
 * On x86, even though code does set hw clock within <1ms of exact
 * whole seconds, apparently hw clock (at least on some machines)
 * doesn't reset internal fractional seconds to 0,
 * making all this a pointless exercise.
 */
	/* If we see that we are N usec away from whole second,
	 * we'll sleep for N-ADJ usecs. ADJ corrects for the fact
	 * that CPU is not infinitely fast.
	 * On infinitely fast CPU, next wakeup would be
	 * on (exactly_next_whole_second - ADJ). On real CPUs,
	 * this difference between current time and whole second
	 * is less than ADJ (assuming system isn't heavily loaded).
	 */
	/* Small value of 256us gives very precise sync for 2+ GHz CPUs.
	 * Slower CPUs will fail to sync and will go to bigger
	 * ADJ values. qemu-emulated armv4tl with ~100 MHz
	 * performance ends up using ADJ ~= 4*1024 and it takes
	 * 2+ secs (2 tries with successively larger ADJ)
	 * to sync. Even straced one on the same qemu (very slow)
	 * takes only 4 tries.
	 */
#define TWEAK_USEC 256
	unsigned adj = TWEAK_USEC;
	struct tm tm_time;
	struct timeval tv;
	int rtc = rtc_xopen(pp_rtcname, O_WRONLY);

	/* Try to catch the moment when whole second is close */
	while (1) {
		unsigned rem_usec;
		time_t t;

		xgettimeofday(&tv);

		t = tv.tv_sec;
		rem_usec = 1000000 - tv.tv_usec;
		if (rem_usec < adj) {
			/* Close enough */
 small_rem:
			t++;
		}

		/* Prepare tm_time from t */
		if (utc)
			gmtime_r(&t, &tm_time); /* may read /etc/xxx (it takes time) */
		else
			localtime_r(&t, &tm_time); /* same */

		if (adj >= 32*1024) {
			break; /* 32 ms diff and still no luck?? give up trying to sync */
		}

		/* gmtime/localtime took some time, re-get cur time */
		xgettimeofday(&tv);

		if (tv.tv_sec < t /* we are still in old second */
		 || (tv.tv_sec == t && tv.tv_usec < adj) /* not too far into next second */
		) {
			break; /* good, we are in sync! */
		}

		rem_usec = 1000000 - tv.tv_usec;
		if (rem_usec < adj) {
			t = tv.tv_sec;
			goto small_rem; /* already close to next sec, don't sleep */
		}

		/* Try to sync up by sleeping */
		usleep(rem_usec - adj);

		/* Jump to 1ms diff, then increase fast (x2): EVERY loop
		 * takes ~1 sec, people won't like slowly converging code here!
		 */
	//bb_error_msg("adj:%d tv.tv_usec:%d", adj, (int)tv.tv_usec);
		if (adj < 512)
			adj = 512;
		/* ... and if last "overshoot" does not look insanely big,
		 * just use it as adj increment. This makes convergence faster.
		 */
		if (tv.tv_usec < adj * 8) {
			adj += tv.tv_usec;
			continue;
		}
		adj *= 2;
	}
	/* Debug aid to find "optimal" TWEAK_USEC with nearly exact sync.
	 * Look for a value which makes tv_usec close to 999999 or 0.
	 * For 2.20GHz Intel Core 2: optimal TWEAK_USEC ~= 200
	 */
	//bb_error_msg("tv.tv_usec:%d", (int)tv.tv_usec);
#endif

	tm_time.tm_isdst = 0;
	xioctl(rtc, RTC_SET_TIME, &tm_time);

	if (ENABLE_FEATURE_CLEAN_UP)
		close(rtc);
}

// hwclock from util-linux 2.36.1
// hwclock [function] [option...]
//Functions:
// -r, --show           display the RTC time
//     --get            display drift corrected RTC time
//     --set            set the RTC according to --date
// -s, --hctosys        set the system time from the RTC
// -w, --systohc        set the RTC from the system time
//     --systz          send timescale configurations to the kernel
// -a, --adjust         adjust the RTC to account for systematic drift
//     --predict        predict the drifted RTC time according to --date
//Options:
// -u, --utc            the RTC timescale is UTC
// -l, --localtime      the RTC timescale is Local
// -f, --rtc <file>     use an alternate file to /dev/rtc0
//     --directisa      use the ISA bus instead of /dev/rtc0 access
//     --date <time>    date/time input for --set and --predict
//     --delay <sec>    delay used when set new RTC time
//     --update-drift   update the RTC drift factor
//     --noadjfile      do not use /etc/adjtime
//     --adjfile <file> use an alternate file to /etc/adjtime
//     --test           dry run; implies --verbose
// -v, --verbose        display more details

//usage:#define hwclock_trivial_usage
//usage:	IF_LONG_OPTS(
//usage:       "[-swul] [--systz] [-f DEV]"
//usage:	)
//usage:	IF_NOT_LONG_OPTS(
//usage:       "[-swult] [-f DEV]"
//usage:	)
//usage:#define hwclock_full_usage "\n\n"
//usage:       "Show or set hardware clock (RTC)\n"
///////:     "\n	-r	Show RTC time"
///////-r is default, don't bother showing it in help
//usage:     "\n	-s	Set system time from RTC"
//usage:     "\n	-w	Set RTC from system time"
//usage:	IF_LONG_OPTS(
//usage:     "\n	--systz	Set in-kernel timezone, correct system time"
//usage:     "\n		if RTC is kept in local time"
//usage:	)
//usage:     "\n	-f DEV	Use specified device (e.g. /dev/rtc2)"
//usage:     "\n	-u	Assume RTC is kept in UTC"
//usage:     "\n	-l	Assume RTC is kept in local time"
//usage:     "\n		(if neither is given, read from "ADJTIME_PATH")"

//TODO: get rid of incompatible -t alias to --systz?

#define HWCLOCK_OPT_LOCALTIME   0x01
#define HWCLOCK_OPT_UTC         0x02
#define HWCLOCK_OPT_SHOW        0x04
#define HWCLOCK_OPT_HCTOSYS     0x08
#define HWCLOCK_OPT_SYSTOHC     0x10
#define HWCLOCK_OPT_SYSTZ       0x20
#define HWCLOCK_OPT_RTCFILE     0x40

int hwclock_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int hwclock_main(int argc UNUSED_PARAM, char **argv)
{
	const char *rtcname = NULL;
	unsigned opt;
	int utc;
#if ENABLE_LONG_OPTS
	static const char hwclock_longopts[] ALIGN1 =
		"localtime\0" No_argument "l"
		"utc\0"       No_argument "u"
		"show\0"      No_argument "r"
		"hctosys\0"   No_argument "s"
		"systohc\0"   No_argument "w"
		"systz\0"     No_argument "t" /* short opt is non-standard */
		"rtc\0"       Required_argument "f"
		;
#endif
	opt = getopt32long(argv,
		"^""lurswtf:v" /* -v is accepted and ignored */
		"\0"
		"r--wst:w--rst:s--wrt:t--rsw:l--u:u--l",
		hwclock_longopts,
		&rtcname
	);

	/* If -u or -l wasn't given, check if we are using utc */
	if (opt & (HWCLOCK_OPT_UTC | HWCLOCK_OPT_LOCALTIME))
		utc = (opt & HWCLOCK_OPT_UTC);
	else
		utc = rtc_adjtime_is_utc();

	if (opt & HWCLOCK_OPT_HCTOSYS)
		to_sys_clock(&rtcname, utc);
	else if (opt & HWCLOCK_OPT_SYSTOHC)
		from_sys_clock(&rtcname, utc);
	else if (opt & HWCLOCK_OPT_SYSTZ)
		set_kernel_timezone_and_clock(utc, NULL);
	else
		/* default HWCLOCK_OPT_SHOW */
		show_clock(&rtcname, utc);

	return 0;
}
