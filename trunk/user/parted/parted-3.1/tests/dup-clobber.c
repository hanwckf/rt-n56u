/* Demonstrate that setting disk->needs_clobber in ped_disk_duplicate
   is necessary.  With that, this test passes.  Without it, the last
   sectors of the disk are cleared, and this test fails.  */
#include <config.h>
#include <parted/parted.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "closeout.h"
#include "progname.h"

static void
seek_to_final_sector (int fd, PedSector ss)
{
  /* Seek to EOF.  */
  off_t off = lseek (fd, 0, SEEK_END);

  /* That had better succeed and determine that the size is > 2 sectors
     and an exact multiple of ss.  */
  assert (2 * ss < off);
  assert (off % ss == 0);

  /* Back up one sector.  */
  off = lseek (fd, -ss, SEEK_CUR);
  assert (0 < off);
}

static void
scribble_on_final_sector (char const *file_name, PedSector ss)
{
  assert (0 < ss);
  assert (ss % 512 == 0);
  int fd = open (file_name, O_WRONLY);
  assert (0 <= fd);

  seek_to_final_sector (fd, ss);

  /* Fill the final sector with ascii 'G's.  */
  char *buf = malloc (ss);
  assert (buf);
  memset (buf, 'G', ss);
  assert (write (fd, buf, ss) == ss);
  free (buf);
  assert (close (fd) == 0);
}

int
main (int argc, char **argv)
{
  atexit (close_stdout);
  set_program_name (argv[0]);

  if (argc != 1)
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

  PedDisk *disk = ped_disk_new_fresh (dev, ped_disk_type_get ("msdos"));
  assert (disk);

  assert (ped_disk_commit(disk));

  PedSector ss = dev->sector_size;
  scribble_on_final_sector (dev_name, ss);

  /* Before the fix, this ped_disk_duplicate call would always set
     copy->needs_clobber, thus causing the subsequent commit to
     mistakenly clobber 9KiB at each end of the disk.  */
  PedDisk *copy = ped_disk_duplicate (disk);
  assert (ped_disk_commit(copy));

  ped_disk_destroy (copy);
  ped_disk_destroy (disk);
  ped_device_destroy (dev);

  /* Read the final sector and ensure it's still all 'G's.  */
  fd = open (dev_name, O_RDONLY);
  assert (0 <= fd);
  seek_to_final_sector (fd, ss);
  char *buf = malloc (ss);
  assert (buf);
  assert (read (fd, buf, ss) == ss);
  unsigned int i;
  for (i = 0; i < ss; i++)
    assert (buf[i] == 'G');
  free (buf);

  return EXIT_SUCCESS;
}
