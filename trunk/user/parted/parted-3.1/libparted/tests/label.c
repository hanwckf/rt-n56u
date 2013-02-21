#include <config.h>
#include <unistd.h>

#include <check.h>

#include <parted/parted.h>

#include "common.h"
#include "progname.h"

#define STREQ(a, b) (strcmp (a, b) == 0)

static char* temporary_disk;

static void
create_disk (void)
{
        temporary_disk = _create_disk (80 * 1024 * 1024);
        fail_if (temporary_disk == NULL, "Failed to create temporary disk");
}

static void
destroy_disk (void)
{
        unlink (temporary_disk);
        free (temporary_disk);
}

/* TEST: Create a disklabel on a simple disk image */
START_TEST (test_create_label)
{
        PedDevice* dev = ped_device_get (temporary_disk);
        if (dev == NULL)
                return;

        PedDiskType* type;
        PedDisk* disk;

        for (type = ped_disk_type_get_next (NULL); type;
             type = ped_disk_type_get_next (type)) {
          fprintf (stderr, "create label: %s\n", type->name); fflush (stderr);

                if (!_implemented_disk_label (type->name))
                        continue;

                disk = _create_disk_label (dev, type);
                ped_disk_destroy (disk);
        }
        ped_device_destroy (dev);
}
END_TEST

/* TEST: Probe the disk label of a loop device. */
START_TEST (test_probe_label)
{
        PedDevice* dev = ped_device_get (temporary_disk);
        if (dev == NULL)
                return;

        PedDiskType* probed;
        PedDiskType* type;
        PedDisk* disk;

        for (type = ped_disk_type_get_next (NULL); type;
             type = ped_disk_type_get_next (type)) {
          fprintf (stderr, "PROBE label: %s\n", type->name); fflush (stderr);
                if (!_implemented_disk_label (type->name))
                        continue;

                disk = _create_disk_label (dev, type);
                ped_disk_destroy (disk);

                /* Try to probe the disk label. */
                probed = ped_disk_probe (dev);
                fail_if (!probed,
                         "Failed to probe the just created label of type: %s",
                         type->name);
                if (probed && !STREQ (probed->name, type->name))
                        fail_if (1,
                                 "Probe returned label of type: %s as type: %s",
                                 type->name, probed->name);
        }
        ped_device_destroy (dev);
}
END_TEST

/* TEST: Read the disk label of a loop device. */
START_TEST (test_read_label)
{
        PedDevice* dev = ped_device_get (temporary_disk);
        if (dev == NULL)
                return;

        PedDiskType* type;
        PedDisk* disk;

        for (type = ped_disk_type_get_next (NULL); type;
             type = ped_disk_type_get_next (type)) {
          fprintf (stderr, "read label: %s\n", type->name); fflush (stderr);
                if (!_implemented_disk_label (type->name))
                        continue;

                disk = _create_disk_label (dev, type);
                ped_disk_destroy (disk);

                /* Try to read the disk label. */
                disk = ped_disk_new (dev);
                fail_if (!disk,
                         "Failed to read the just created label of type: %s",
                         type->name);
                if (disk && !STREQ (disk->type->name, type->name))
                        fail_if (1,
                                 "Read returned label of type: %s as type: %s",
                                 type->name, disk->type->name);

                ped_disk_destroy (disk);
        }
        ped_device_destroy (dev);
}
END_TEST

/* TEST: Clone the disk label of a loop device. */
START_TEST (test_clone_label)
{
        PedDevice* dev = ped_device_get (temporary_disk);
        if (dev == NULL)
                return;

        PedDiskType* type;

        for (type = ped_disk_type_get_next (NULL); type;
             type = ped_disk_type_get_next (type)) {
          fprintf (stderr, "clone label: %s\n", type->name); fflush (stderr);
                if (!_implemented_disk_label (type->name))
                        continue;

                PedDisk* disk = _create_disk_label (dev, type);

                /* Try to clone the disk label. */
                PedDisk* clone = ped_disk_duplicate (disk);
                fail_if (!clone,
                         "Failed to clone the just created label of type: %s",
                         type->name);

                ped_disk_destroy (clone);
                ped_disk_destroy (disk);
        }
        ped_device_destroy (dev);
}
END_TEST

int
main (int argc, char **argv)
{
        set_program_name (argv[0]);
        int number_failed;
        Suite* suite = suite_create ("Disk Label");
        TCase* tcase_basic = tcase_create ("Create");
        TCase* tcase_probe = tcase_create ("Probe");
        TCase* tcase_read = tcase_create ("Read");
        TCase* tcase_clone = tcase_create ("Clone");

        /* Fail when an exception is raised */
        ped_exception_set_handler (_test_exception_handler);

        tcase_add_checked_fixture (tcase_basic, create_disk, destroy_disk);
        tcase_add_test (tcase_basic, test_create_label);
        /* Disable timeout for this test */
        tcase_set_timeout (tcase_basic, 0);
        suite_add_tcase (suite, tcase_basic);

        tcase_add_checked_fixture (tcase_probe, create_disk, destroy_disk);
        tcase_add_test (tcase_probe, test_probe_label);
        /* Disable timeout for this test. */
        tcase_set_timeout (tcase_probe, 0);
        suite_add_tcase (suite, tcase_probe);

        tcase_add_checked_fixture (tcase_read, create_disk, destroy_disk);
        tcase_add_test (tcase_read, test_read_label);
        /* Disable timeout for this test. */
        tcase_set_timeout (tcase_read, 0);
        suite_add_tcase (suite, tcase_read);

        tcase_add_checked_fixture (tcase_clone, create_disk, destroy_disk);
        tcase_add_test (tcase_clone, test_clone_label);
        /* Disable timeout for this test. */
        tcase_set_timeout (tcase_clone, 0);
        suite_add_tcase (suite, tcase_clone);

        SRunner* srunner = srunner_create (suite);
        srunner_run_all (srunner, CK_VERBOSE);

        number_failed = srunner_ntests_failed (srunner);
        srunner_free (srunner);

        return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
