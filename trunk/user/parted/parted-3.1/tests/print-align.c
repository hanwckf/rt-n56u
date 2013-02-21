#include <config.h>
#include <parted/parted.h>
#include <stdio.h>
#include <stdlib.h>

#include "closeout.h"
#include "progname.h"

int
main (int argc, char **argv)
{
  atexit (close_stdout);
  set_program_name (argv[0]);

  if (argc != 2)
    return EXIT_FAILURE;

  char const *dev_name = argv[1];
  PedDevice *dev = ped_device_get (dev_name);
  if (dev == NULL)
    return EXIT_FAILURE;

  PedAlignment *pa_min = ped_device_get_minimum_alignment (dev);
  if (pa_min)
    printf ("minimum: %lld %lld\n", pa_min->offset, pa_min->grain_size);
  else
    printf ("minimum: - -\n");
  free (pa_min);

  PedAlignment *pa_opt = ped_device_get_optimum_alignment (dev);
  if (pa_opt)
    printf ("optimal: %lld %lld\n", pa_opt->offset, pa_opt->grain_size);
  else
    printf ("optimal: - -\n");
  free (pa_opt);

  PedDisk *disk = ped_disk_new (dev);
  if (disk == NULL)
    return EXIT_FAILURE;

  PedAlignment *part_align = ped_disk_get_partition_alignment (disk);
  if (part_align == NULL)
    return EXIT_FAILURE;

  printf ("partition alignment: %lld %lld\n",
	  part_align->offset, part_align->grain_size);

  ped_device_destroy (dev);
  return EXIT_SUCCESS;
}
