/* Sometimes libparted operates on device mapper files, with a path of
   /dev/mapper/foo. With newer lvm versions /dev/mapper/foo is a symlink to
   /dev/dm-#. However some storage administration programs (anaconda for
   example) may do the following:
   1) Create a ped_device for /dev/mapper/foo
   2) ped_get_device resolves the symlink to /dev/dm-#, and the path
      in the PedDevice struct points to /dev/dm-#
   3) The program does some things to lvm, causing the symlink to
      point to a different /dev/dm-# node
   4) The program does something with the PedDevice, which results
      in an operation on the wrong device

   Newer libparted versions do not suffer from this problem, as they
   do not canonicalize device names under /dev/mapper. This test checks
   for this bug. */

#include <config.h>
#include <unistd.h>

#include <check.h>

#include <parted/parted.h>

#include "common.h"
#include "progname.h"

static char *temporary_disk;

static void
create_disk (void)
{
        temporary_disk = _create_disk (4096 * 1024);
        fail_if (temporary_disk == NULL, "Failed to create temporary disk");
}

static void
destroy_disk (void)
{
        unlink (temporary_disk);
        free (temporary_disk);
}

START_TEST (test_symlink)
{
        char cwd[256], ln[256] = "/dev/mapper/parted-test-XXXXXX";

        if (!getcwd (cwd, sizeof cwd)) {
                fail ("Could not get cwd");
                return;
        }

        /* Create a symlink under /dev/mapper to our
           temporary disk */
        int tmp_fd = mkstemp (ln);
        if (tmp_fd == -1) {
                fail ("Could not create tempfile");
                return;
        }

        /* There is a temp file vulnerability symlink attack possibility
           here, but as /dev/mapper is root owned this is a non issue */
        close (tmp_fd);
        unlink (ln);
        char temp_disk_path[256];
        snprintf (temp_disk_path, sizeof temp_disk_path, "%s/%s", cwd,
                  temporary_disk);
        int res = symlink (temp_disk_path, ln);
        if (res) {
                fail ("could not create symlink");
                return;
        }

        PedDevice *dev = ped_device_get (ln);
        if (dev == NULL)
                goto exit_unlink_ln;

        /* Create a second temporary_disk */
        char *temporary_disk2 = _create_disk (4096 * 1024);
        if (temporary_disk2 == NULL) {
                fail ("Failed to create 2nd temporary disk");
                goto exit_destroy_dev;
        }

        /* Remove first temporary disk, and make temporary_disk point to
           temporary_disk2 (for destroy_disk()). */
        unlink (temporary_disk);
        free (temporary_disk);
        temporary_disk = temporary_disk2;

        /* Update symlink to point to our new / second temporary disk */
        unlink (ln);
        snprintf (temp_disk_path, sizeof temp_disk_path, "%s/%s", cwd,
                  temporary_disk);
        res = symlink (temp_disk_path, ln);
        if (res) {
                fail ("could not create 2nd symlink");
                goto exit_destroy_dev;
        }

        /* Do something to our PedDevice, if the symlink was resolved,
           instead of remembering the /dev/mapper/foo name, this will fail */
        ped_disk_clobber (dev);

exit_destroy_dev:
        ped_device_destroy (dev);
exit_unlink_ln:
        unlink (ln);
}
END_TEST

int
main (int argc, char **argv)
{
        set_program_name (argv[0]);
        int number_failed;
        Suite* suite = suite_create ("Symlink");
        TCase* tcase_symlink = tcase_create ("/dev/mapper symlink");

        /* Fail when an exception is raised */
        ped_exception_set_handler (_test_exception_handler);

        tcase_add_checked_fixture (tcase_symlink, create_disk, destroy_disk);
        tcase_add_test (tcase_symlink, test_symlink);
        /* Disable timeout for this test */
        tcase_set_timeout (tcase_symlink, 0);
        suite_add_tcase (suite, tcase_symlink);

        SRunner* srunner = srunner_create (suite);
        srunner_run_all (srunner, CK_VERBOSE);

        number_failed = srunner_ntests_failed (srunner);
        srunner_free (srunner);

        return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
