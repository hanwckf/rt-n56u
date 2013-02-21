#include <config.h>
#include <unistd.h>

#include <check.h>

#include <parted/parted.h>

#include "common.h"
#include "progname.h"

static const char* temporary_disk;

/* TEST: Probe a zero-length disk image without raising an exception */
START_TEST (test_probe)
{
        PedDevice* dev = ped_device_get (temporary_disk);
        if (dev)
                ped_device_destroy (dev);
}
END_TEST

int
main (int argc, char **argv)
{
        set_program_name (argv[0]);
        int number_failed;
        Suite* suite = suite_create ("ZeroLen");
        TCase* tcase_probe = tcase_create ("Probe");

        if (argc < 2) {
                fail ("Insufficient arguments");
                return EXIT_FAILURE;
        }
        temporary_disk = argv[1];
        setenv ("PARTED_TEST_DEVICE_LENGTH", "0", 1);

        /* Fail when an exception is raised */
        ped_exception_set_handler (_test_exception_handler);

        tcase_add_test (tcase_probe, test_probe);
        /* Disable timeout for this test */
        tcase_set_timeout (tcase_probe, 0);
        suite_add_tcase (suite, tcase_probe);

        SRunner* srunner = srunner_create (suite);
        srunner_run_all (srunner, CK_VERBOSE);

        number_failed = srunner_ntests_failed (srunner);
        srunner_free (srunner);

        return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
