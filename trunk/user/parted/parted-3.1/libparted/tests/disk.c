#include <config.h>
#include <unistd.h>

#include <check.h>

#include <parted/parted.h>

#include "common.h"
#include "progname.h"
#include "xstrtol.h"

static char* temporary_disk;

static
size_t get_sector_size (void)
{
  char *p = getenv ("PARTED_SECTOR_SIZE");
  size_t ss = 512;
  unsigned long val;
  if (p
      && xstrtoul (p, NULL, 10, &val, NULL) == LONGINT_OK
      && val % 512 == 0)
    ss = val;

  return ss;
}

static void
create_disk (void)
{
        temporary_disk = _create_disk (get_sector_size () * 4 * 10 * 1024);
        fail_if (temporary_disk == NULL, "Failed to create temporary disk");
}

static void
destroy_disk (void)
{
        unlink (temporary_disk);
        free (temporary_disk);
}

/* TEST: Create a disklabel on a simple disk image */
START_TEST (test_duplicate)
{
        PedDevice* dev = ped_device_get (temporary_disk);
        if (dev == NULL)
                return;

        PedDisk* disk;
        PedDisk* disk_dup;
        PedPartition *part;
        PedPartition *part_dup;
        PedConstraint *constraint;

        int part_num[] = {1, 5, 6, 0};

        disk = _create_disk_label (dev, ped_disk_type_get ("msdos"));

        constraint = ped_constraint_any (dev);

        /* Primary partition from 16,4kB to 15MB */
        part = ped_partition_new (disk, PED_PARTITION_EXTENDED,
                                  NULL,
                                  32, 29311);
        ped_disk_add_partition (disk, part, constraint);

        /* Logical partition from 10MB to 15MB */
        part = ped_partition_new (disk, PED_PARTITION_LOGICAL,
                                  ped_file_system_type_get ("ext2"),
                                  19584, 29311);
        ped_disk_add_partition (disk, part, constraint);

        /* Logical partition from 16,4kB to 4981kB */
        part = ped_partition_new (disk, PED_PARTITION_LOGICAL,
                                  ped_file_system_type_get ("ext2"),
                                  32, 9727);
        ped_disk_add_partition (disk, part, constraint);

        ped_disk_commit (disk);

        ped_constraint_destroy (constraint);

        disk_dup = ped_disk_duplicate (disk);

        /* Checks if both partitions match */
        for (int *i = part_num; *i != 0; i++) {
                part = ped_disk_get_partition (disk, *i);
                part_dup = ped_disk_get_partition (disk_dup, *i);

                fail_if (part->geom.start != part_dup->geom.start ||
                         part->geom.end != part_dup->geom.end,
                         "Duplicated partition %d doesn't match. "
                         "Details are start: %d/%d end: %d/%d\n",
                         *i, part->geom.start, part_dup->geom.start,
                         part->geom.end, part_dup->geom.end);
        }

        ped_disk_destroy (disk);
        ped_device_destroy (dev);
}
END_TEST

int
main (int argc, char **argv)
{
        set_program_name (argv[0]);
        int number_failed;
        Suite* suite = suite_create ("Disk");
        TCase* tcase_duplicate = tcase_create ("Duplicate");

        /* Fail when an exception is raised */
        ped_exception_set_handler (_test_exception_handler);

        tcase_add_checked_fixture (tcase_duplicate, create_disk, destroy_disk);
        tcase_add_test (tcase_duplicate, test_duplicate);
        /* Disable timeout for this test */
        tcase_set_timeout (tcase_duplicate, 0);
        suite_add_tcase (suite, tcase_duplicate);

        SRunner* srunner = srunner_create (suite);
        srunner_run_all (srunner, CK_VERBOSE);

        number_failed = srunner_ntests_failed (srunner);
        srunner_free (srunner);

        return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
