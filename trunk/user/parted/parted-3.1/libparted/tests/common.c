#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#include <check.h>

#include "common.h"

#define STREQ(a, b) (strcmp (a, b) == 0)

PedExceptionOption
_test_exception_handler (PedException* e)
{
        fail ("Exception of type %s has been raised: %s",
              ped_exception_get_type_string (e->type),
              e->message);

        return PED_EXCEPTION_UNHANDLED;
}

char*
_create_disk (const off_t n_bytes)
{
        char* filename = strdup ("parted-test-XXXXXX");

        if (filename == NULL)
                return NULL;

        int fd = mkstemp (filename);
        if (fd < 0) {
        free_filename:
                free (filename);
                return NULL;
        }

        FILE* disk = fdopen (fd, "w");
        if (disk == NULL)
                goto free_filename;

        int fail = (fseek (disk, n_bytes, SEEK_SET) != 0
                    || fwrite ("", sizeof (char), 1, disk) != 1);

        if (fclose (disk) != 0 || fail)
                goto free_filename;

        return filename;
}

PedDisk*
_create_disk_label (PedDevice *dev, PedDiskType *type)
{
        PedDisk* disk = NULL;

        /* Create the label */
        disk = ped_disk_new_fresh (dev, type);
        fail_if (!disk, "Failed to create a label of type: %s",
                 type->name);
        fail_if (!ped_disk_commit(disk),
		 "Failed to commit label to device");

        return disk;
}

int
_implemented_disk_label (const char *label)
{
        /* FIXME: these have minor problems, so skip them, temporarily.  */
        if (STREQ (label, "amiga")) return 0;

        /* Not implemented yet */
        if (STREQ (label, "aix")) return 0;
        if (STREQ (label, "pc98")) return 0;

        return 1;
}
