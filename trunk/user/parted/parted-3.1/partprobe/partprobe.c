/*
    partprobe - informs the OS kernel of partition layout
    Copyright (C) 2001-2002, 2007-2012 Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* it's best to compile this with:
 *
 * 	 CFLAGS=-Os ./configure --disable-nls --disable-shared --disable-debug
 * 	 	    --enable-discover-only
 *
 * And strip(1) afterwards!
 */

#include <config.h>

#include <parted/parted.h>

#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "closeout.h"
#include "configmake.h"
#include "progname.h"
#include "version-etc.h"

#include <locale.h>
#include "gettext.h"
#if ! ENABLE_NLS
# undef textdomain
# define textdomain(Domainname) /* empty */
# undef bindtextdomain
# define bindtextdomain(Domainname, Dirname) /* empty */
#endif

#undef _
#define _(msgid) gettext (msgid)

#define AUTHORS \
  "<http://git.debian.org/?p=parted/parted.git;a=blob_plain;f=AUTHORS>"

/* The official name of this program (e.g., no `g' prefix).  */
#define PROGRAM_NAME "partprobe"

static struct option const long_options[] =
  {
    /* Note: the --no-update option is deprecated, and deliberately
     * not documented.  FIXME: remove --no-update in 2009. */
    {"no-update", no_argument, NULL, 'd'},
    {"dry-run", no_argument, NULL, 'd'},
    {"summary", no_argument, NULL, 's'},
    {"help", no_argument, NULL, 'h'},
    {"version", no_argument, NULL, 'v'},
    {NULL, 0, NULL, 0}
  };

/* initialized to 0 according to the language lawyers */
static int	opt_no_inform;
static int	opt_summary;

static void
summary (PedDisk* disk)
{
	PedPartition*	walk;

	printf ("%s: %s partitions", disk->dev->path, disk->type->name);
	for (walk = disk->part_list; walk; walk = walk->next) {
		if (!ped_partition_is_active (walk))
			continue;

		printf (" %d", walk->num);
		if (walk->type & PED_PARTITION_EXTENDED) {
			PedPartition*	log_walk;
			int		is_first = 1;

			printf (" <");
			for (log_walk = walk->part_list; log_walk;
			     log_walk = log_walk->next) {
				if (!ped_partition_is_active (log_walk))
					continue;
				if (!is_first)
					printf (" ");
				printf ("%d", log_walk->num);
				is_first = 0;
			}
			printf (">");
		}
	}
	printf ("\n");
}

static int
process_dev (PedDevice* dev)
{
	PedDiskType*	disk_type;
	PedDisk*	disk;

	disk_type = ped_disk_probe (dev);
	if (!disk_type || !strcmp (disk_type->name, "loop"))
		return 1;

	disk = ped_disk_new (dev);
	if (!disk)
		goto error;
	if (!opt_no_inform) {
		if (!ped_disk_commit_to_os (disk))
			goto error_destroy_disk;
	}
	if (opt_summary)
		summary (disk);
	ped_disk_destroy (disk);
	return 1;

error_destroy_disk:
	ped_disk_destroy (disk);
error:
	return 0;
}

static void
usage (int status)
{
  if (status != EXIT_SUCCESS)
    fprintf (stderr, _("Try `%s --help' for more information.\n"),
	     program_name);
  else
    {
      printf (_("Usage: %s [OPTION] [DEVICE]...\n"), PROGRAM_NAME);
      fputs (_("\
Inform the operating system about partition table changes.\n\
\n\
  -d, --dry-run    do not actually inform the operating system\n\
  -s, --summary    print a summary of contents\n\
  -h, --help       display this help and exit\n\
  -v, --version    output version information and exit\n\
"), stdout);
      fputs (_("\
\n\
When no DEVICE is given, probe all partitions.\n\
"), stdout);
      printf (_("\nReport bugs to <%s>.\n"), PACKAGE_BUGREPORT);
    }
  exit (status);
}

int
main (int argc, char* argv[])
{
	int		status = 0;

	set_program_name (argv[0]);

	setlocale (LC_ALL, "");
	bindtextdomain (PACKAGE, LOCALEDIR);
	textdomain (PACKAGE);

	atexit (close_stdout);

	int c;
	while ((c = getopt_long (argc, argv, "dhsv", long_options, NULL)) != -1)
		switch (c) {
			case 'd':
				opt_no_inform = 1;
				break;

			case 's':
				opt_summary = 1;
				break;

			case 'h':
				usage (EXIT_SUCCESS);
				break;

			case 'v':
				version_etc (stdout, PROGRAM_NAME, PACKAGE_NAME,
				             VERSION, AUTHORS, (char *) NULL);
				exit (EXIT_SUCCESS);
				break;

			default:
				usage (EXIT_FAILURE);
                }

        int n_dev = argc - optind;
	if (n_dev != 0) {
		int i;
		for (i = optind; i < argc; i++) {
			PedDevice *dev = ped_device_get (argv[i]);
			if (dev == NULL || process_dev (dev) == 0)
				status = 1;
		}
	} else {
		ped_device_probe_all ();
		PedDevice *dev;
		for (dev = ped_device_get_next (NULL); dev;
		     dev = ped_device_get_next (dev))
			if (process_dev (dev) == 0)
				status = 1;
	}

	return status;
}
