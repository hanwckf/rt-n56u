#include <config.h>

#include <parted/parted.h>
#include <parted/debug.h>

#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include "progname.h"

int
main (int argc, char **argv)
{
	set_program_name (argv[0]);

	PedSector start = 0, len = 0;
	PedGeometry geom, new_geom;
	PedDevice *dev;
	PedFileSystem *fs;
	PedTimer *g_timer = NULL;

	if (argc != 2 && argc != 4) {
		fprintf(stderr, "usage: %s <device>\n"
				"       %s <device> <start> <length>\n",
				argv[0], argv[0]);
		return 1;
	}

	dev = ped_device_get(argv[1]);
	if (!dev) {
		fprintf(stderr, "cannot create device %s\n", argv[1]);
		return 1;
	}

	if (!ped_device_open(dev)) {
		fprintf(stderr, "cannot open device %s\n", argv[1]);
		return 1;
	}

	if (!ped_geometry_init(&geom, dev, 0, dev->length)) {
		fprintf(stderr, "cannot initialize geometry\n");
		return 1;
	}

	if (argc > 2) {
		start = strtoll(argv[2], NULL, 0);
		len = strtoll(argv[3], NULL, 0);
	} else {
		start = 0;
		len = dev->length;
	}

	if (!ped_geometry_init(&new_geom, dev, start, len)) {
		fprintf(stderr, "cannot initialize new geometry\n");
		return 1;
	}

	fs = ped_file_system_open(&geom);
	if (!fs) {
		fprintf(stderr, "cannot read fs\n");
		return 1;
	}

	if (!ped_file_system_resize(fs, &new_geom, g_timer)) {
		fprintf(stderr, "cannot resize filesystem\n");
		return 1;
	}

	ped_file_system_close(fs);
	return 0;
}
