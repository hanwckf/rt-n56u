/* vi: set sw=4 ts=4: */
/*
 * Print UUIDs on all filesystems
 *
 * Copyright (C) 2008 Denys Vlasenko.
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */

//usage:#define blkid_trivial_usage
//usage:       ""
//usage:#define blkid_full_usage "\n\n"
//usage:       "Print UUIDs of all filesystems"

#include "libbb.h"
#include "volume_id.h"

//TODO: extend to take BLOCKDEV args, and show TYPE="fstype"

int blkid_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int blkid_main(int argc UNUSED_PARAM, char **argv)
{
	while (*++argv) {
		/* Note: bogus device names don't cause any error messages */
		add_to_uuid_cache(*argv);
	}

	display_uuid_cache();
	return 0;
}
