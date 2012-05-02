/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/* Shared library add-on to iptables to add TIME matching support. */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <iptables.h>
#include <linux/netfilter_ipv4/ipt_time.h>
#include <time.h>

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"TIME v%s options:\n"
" --timestart value --timestop value --days listofdays\n"
"          timestart value : HH:MM:SS\n"
"          timestop  value : HH:MM:SS\n"
"          listofdays value: a list of days to apply -> ie. Mon,Tue,Wed,Thu,Fri. Case sensitive\n",
IPTABLES_VERSION);
}

static struct option opts[] = {
	{ "timestart", 1, 0, '1' },
	{ "timestop", 1, 0, '2' },
	{ "days", 1, 0, '3'},
	{0}
};

/* Initialize the match. */
static void
init(struct ipt_entry_match *m, unsigned int *nfcache)
{
	/* caching not yet implemented */
        *nfcache |= NFC_UNKNOWN;
}

static int
parse_time(const char *time)
{
	int hours, minutes, seconds;

	if (sscanf(time, "%d:%d:%d", &hours, &minutes, &seconds) == 3)
		return (hours * 60 * 60 + minutes * 60 + seconds);

	/* If we are here, there was a problem ..*/
	exit_error(PARAMETER_PROBLEM,
		   "invalid time `%s' specified, should be HH:MM:SS format", time);
}

static const char *days[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

static int
parse_days(const char *string)
{
	char *comma;
	int i, mask = 0;

	do {
		for (i = 0; i < 7; i++)
			if (!strncasecmp(string, days[i], 3))
				mask |= 1 << i;
		comma = strchr(string, ',');
		string = comma + 1;
	} while (comma);

	return mask;
}

#define IPT_TIME_START 0x01
#define IPT_TIME_STOP  0x02
#define IPT_TIME_DAYS  0x04


/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      unsigned int *nfcache,
      struct ipt_entry_match **match)
{
	struct ipt_time_info *timeinfo = (struct ipt_time_info *)(*match)->data;

	switch (c)
	{
		/* timestart */
	case '1':
		if (invert)
			exit_error(PARAMETER_PROBLEM,
                                   "unexpected '!' with --timestart");
		if (*flags & IPT_TIME_START)
                        exit_error(PARAMETER_PROBLEM,
                                   "Can't specify --timestart twice");
		timeinfo->time_start = parse_time(optarg);
		*flags |= IPT_TIME_START;
		break;
		/* timestop */
	case '2':
		if (invert)
			exit_error(PARAMETER_PROBLEM,
                                   "unexpected '!' with --timestop");
		if (*flags & IPT_TIME_STOP)
                        exit_error(PARAMETER_PROBLEM,
                                   "Can't specify --timestop twice");
		timeinfo->time_stop = parse_time(optarg);
		*flags |= IPT_TIME_STOP;
		break;

		/* days */
	case '3':
		if (invert)
			exit_error(PARAMETER_PROBLEM,
                                   "unexpected '!' with --days");
		if (*flags & IPT_TIME_DAYS)
                        exit_error(PARAMETER_PROBLEM,
                                   "Can't specify --days twice");
		timeinfo->days_match = parse_days(optarg);
		*flags |= IPT_TIME_DAYS;
		break;
	default:
		return 0;
	}
	return 1;
}

/* Final check; must have specified --timestart --timestop --days. */
static void
final_check(unsigned int flags)
{
	if (flags != (IPT_TIME_START | IPT_TIME_STOP | IPT_TIME_DAYS))
		exit_error(PARAMETER_PROBLEM,
			   "TIME match: You must specify `--timestart --timestop and --days'");
}

static void
print_days(const struct ipt_time_info *time)
{
	int i;
	char *sep = "";

	for (i = 0; i < 7; i++) {
		if (time->days_match & (1 << i)) {
			printf("%s%s", sep, days[i]);
			sep = ",";
		}
	}
}

/* Prints out the matchinfo. */
static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_match *match,
      int numeric)
{
	struct ipt_time_info *time = ((struct ipt_time_info *)match->data);

	printf(" TIME from %02d:%02d:%02d to %02d:%02d:%02d on ",
	       time->time_start / (60 * 60), (time->time_start / 60) % 60, time->time_start % 60,
	       time->time_stop / (60 * 60), (time->time_stop / 60) % 60, time->time_stop % 60);
	print_days(time);
	printf(" ");
}

/* Saves the data in parsable form to stdout. */
static void
save(const struct ipt_ip *ip, const struct ipt_entry_match *match)
{
	struct ipt_time_info *time = ((struct ipt_time_info *)match->data);

	printf(" --timestart %02d:%02d:%02d --timestop %02d:%02d:%02d --days ",
	       time->time_start / (60 * 60), (time->time_start / 60) % 60, time->time_start % 60,
	       time->time_stop / (60 * 60), (time->time_stop / 60) % 60, time->time_stop % 60);
	print_days(time);
	printf(" ");
}

/*
static
struct iptables_match timestruct
= { NULL,
    "time",
    IPTABLES_VERSION,
    IPT_ALIGN(sizeof(struct ipt_time_info)),
    IPT_ALIGN(sizeof(struct ipt_time_info)),
    &help,
    &init,
    &parse,
    &final_check,
    &print,
    &save,
    opts
};
*/

static
struct iptables_match timestruct = {
        .next           = NULL,
        .name           = "time",
        .version        = IPTABLES_VERSION,
        .size           = IPT_ALIGN(sizeof(struct ipt_time_info)),
        //.userspacesize  = offsetof(struct ipt_time_info, kerneltime),
        .userspacesize  = IPT_ALIGN(sizeof(struct ipt_time_info)),
        .help           = &help,
        .init           = &init,
        .parse          = &parse,
        .final_check    = &final_check,
        .print          = &print,
        .save           = &save,
        .extra_opts     = opts
};

void _init(void)
{
	register_match(&timestruct);
}
