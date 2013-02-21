/* Demonstrate that ped_disk_duplicate is working correctly.
*/
#include <config.h>
#include <parted/parted.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "closeout.h"
#include "progname.h"

int
main (int argc, char **argv)
{
  atexit (close_stdout);
  set_program_name (argv[0]);

  if (argc != 2)
    return EXIT_FAILURE;

  char const *dev_name = "dev-file";

  /* Create a file.  */
  int fd = open (dev_name, O_CREAT|O_TRUNC|O_WRONLY, 0644);
  assert (0 <= fd);
  off_t size = 8 * 1024 * 1024;
  assert (ftruncate (fd, size) == 0);
  assert (close (fd) == 0);

  PedDevice *dev = ped_device_get (dev_name);
  assert (dev);

  PedDisk *disk = ped_disk_new_fresh (dev, ped_disk_type_get (argv[1]));
  assert (disk);
  assert (ped_disk_commit(disk));
  ped_disk_destroy (disk);

  /* re-open the disk */
  disk = ped_disk_new (dev);
  assert (disk);

  /* Create a partition */
  const PedFileSystemType *fs_type = ped_file_system_type_get ("ext2");
  assert (fs_type);
  PedPartitionType part_type = PED_PARTITION_NORMAL;
  const PedGeometry *geometry = ped_geometry_new (dev, 34, 1024);
  assert (geometry);
  PedPartition *part = ped_partition_new (disk, part_type, fs_type,
					  geometry->start, geometry->end);
  assert (part);
  PedConstraint *constraint = ped_constraint_exact (geometry);
  assert (constraint);

  if (ped_partition_is_flag_available (part, PED_PARTITION_BOOT))
    assert (ped_partition_set_flag (part, PED_PARTITION_BOOT, 1));

  assert (ped_disk_add_partition (disk, part, constraint));
  ped_constraint_destroy (constraint);

  assert (ped_partition_set_system (part, fs_type));
  if (ped_partition_is_flag_available (part, PED_PARTITION_LBA))
    ped_partition_set_flag (part, PED_PARTITION_LBA, 1);

  assert (ped_disk_commit(disk));

  /* Duplicate it */
  PedDisk *copy = ped_disk_duplicate (disk);
  assert (ped_disk_commit(copy));

  /* Compare the two copies */

  /* Check the device */
  assert (strcmp (disk->dev->model, copy->dev->model) == 0);
  assert (strcmp (disk->dev->path, copy->dev->path) == 0);
  assert (disk->dev->sector_size == copy->dev->sector_size);
  assert (disk->dev->phys_sector_size == copy->dev->phys_sector_size);
  assert (disk->dev->length == copy->dev->length);

  /* Check the type */
  assert (strcmp (disk->type->name, copy->type->name) == 0);
  assert (disk->type->features == copy->type->features);

  /* Check the flags */
  for (PedDiskFlag flag = PED_DISK_FIRST_FLAG; flag <= PED_DISK_LAST_FLAG;
       flag++) {
    if (!ped_disk_is_flag_available(disk, flag))
      continue;
    assert (ped_disk_get_flag (disk, flag) == ped_disk_get_flag (copy, flag));
  }

  /* Check the partitions */
  PedPartition *disk_part, *copy_part;
  for ( disk_part = disk->part_list, copy_part = copy->part_list;
        disk_part && copy_part;
        disk_part = disk_part->next, copy_part = copy_part->next)
  {
    /* Only active partitions are duplicated */
    if (!ped_partition_is_active (disk_part))
      continue;

    assert (disk_part->geom.start == copy_part->geom.start);
    assert (disk_part->geom.end == copy_part->geom.end);
    assert (disk_part->geom.length == copy_part->geom.length);
    assert (disk_part->num == copy_part->num);
    assert (disk_part->type == copy_part->type);

    if (disk_part->fs_type && disk_part->fs_type->name) {
      assert (strcmp (disk_part->fs_type->name, copy_part->fs_type->name) == 0);
    }

    /* Check the flags */
    for (PedPartitionFlag flag = PED_PARTITION_FIRST_FLAG;
	 flag <= PED_PARTITION_LAST_FLAG; flag++) {
      if (!ped_partition_is_flag_available(disk_part, flag))
        continue;
      fprintf (stderr, "Checking partition flag %d\n", flag);
      fprintf (stderr, "%d ? %d\n", ped_partition_get_flag (disk_part, flag),
	       ped_partition_get_flag (copy_part, flag));
      assert (ped_partition_get_flag (disk_part, flag)
	      == ped_partition_get_flag (copy_part, flag));
    }
  }

  /* Cleanup the mess */
  ped_disk_destroy (copy);
  ped_disk_destroy (disk);
  ped_device_destroy (dev);

  return EXIT_SUCCESS;
}
