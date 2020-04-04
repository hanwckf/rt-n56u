/* Copyright (C) 2002, 2003, 2006, 2007, 2009 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <fcntl.h>
#include <mntent.h>
#include <paths.h>
#include <pthread.h>
#include <search.h>
#include <semaphore.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <linux_fsinfo.h>
#include "semaphoreP.h"
#include "../../libc/misc/internals/tempname.h"


/* Compatibility defines. */
#define __endmntent                    endmntent
#define __getmntent_r                  getmntent_r
#define __setmntent                    setmntent
#define __statfs                       statfs
#define __libc_close                   close
#define __libc_open                    open64
#define __fxstat64(vers, fd, buf)      fstat64(fd, buf)
#define __libc_write                   write


/* Information about the mount point.  */
struct mountpoint_info mountpoint attribute_hidden;

/* This is the default mount point.  */
static const char defaultmount[] = "/dev/shm";
/* This is the default directory.  */
static const char defaultdir[] = "/dev/shm/sem.";

/* Protect the `mountpoint' variable above.  */
pthread_once_t __namedsem_once attribute_hidden = PTHREAD_ONCE_INIT;


/* Determine where the shmfs is mounted (if at all).  */
void
attribute_hidden
__where_is_shmfs (void)
{
  char buf[512];
  struct statfs f;
  struct mntent resmem;
  struct mntent *mp;
  FILE *fp;

  /* The canonical place is /dev/shm.  This is at least what the
     documentation tells everybody to do.  */
  if (__statfs (defaultmount, &f) == 0
      && (f.f_type == SHMFS_SUPER_MAGIC_WITH_MMU
	  || f.f_type == SHMFS_SUPER_MAGIC_WITHOUT_MMU))
    {
      /* It is in the normal place.  */
      mountpoint.dir = (char *) defaultdir;
      mountpoint.dirlen = sizeof (defaultdir) - 1;

      return;
    }

  /* OK, do it the hard way.  Look through the /proc/mounts file and if
     this does not exist through /etc/fstab to find the mount point.  */
  fp = __setmntent ("/proc/mounts", "r");
  if (unlikely (fp == NULL))
    {
      fp = __setmntent (_PATH_MNTTAB, "r");
      if (unlikely (fp == NULL))
	/* There is nothing we can do.  Blind guesses are not helpful.  */
	return;
    }

  /* Now read the entries.  */
  while ((mp = __getmntent_r (fp, &resmem, buf, sizeof buf)) != NULL)
    /* The original name is "shm" but this got changed in early Linux
       2.4.x to "tmpfs".  */
    if (strcmp (mp->mnt_type, "tmpfs") == 0
	|| strcmp (mp->mnt_type, "shm") == 0)
      {
	/* Found it.  There might be more than one place where the
           filesystem is mounted but one is enough for us.  */
	size_t namelen;

	/* First make sure this really is the correct entry.  At least
	   some versions of the kernel give wrong information because
	   of the implicit mount of the shmfs for SysV IPC.  */
	if (__statfs (mp->mnt_dir, &f) != 0
	    || (f.f_type != SHMFS_SUPER_MAGIC_WITH_MMU
		&& f.f_type != SHMFS_SUPER_MAGIC_WITHOUT_MMU))
	  continue;

	namelen = strlen (mp->mnt_dir);

	if (namelen == 0)
	  /* Hum, maybe some crippled entry.  Keep on searching.  */
	  continue;

	mountpoint.dir = (char *) malloc (namelen + 4 + 2);
	if (mountpoint.dir != NULL)
	  {
	    char *cp = mempcpy (mountpoint.dir, mp->mnt_dir, namelen);
	    if (cp[-1] != '/')
	      *cp++ = '/';
	    cp = stpcpy (cp, "sem.");
	    mountpoint.dirlen = cp - mountpoint.dir;
	  }

	break;
      }

  /* Close the stream.  */
  __endmntent (fp);
}


/* Comparison function for search of existing mapping.  */
int
attribute_hidden
__sem_search (const void *a, const void *b)
{
  const struct inuse_sem *as = (const struct inuse_sem *) a;
  const struct inuse_sem *bs = (const struct inuse_sem *) b;

  if (as->ino != bs->ino)
    /* Cannot return the difference the type is larger than int.  */
    return as->ino < bs->ino ? -1 : (as->ino == bs->ino ? 0 : 1);

  if (as->dev != bs->dev)
    /* Cannot return the difference the type is larger than int.  */
    return as->dev < bs->dev ? -1 : (as->dev == bs->dev ? 0 : 1);

  return strcmp (as->name, bs->name);
}


/* The search tree for existing mappings.  */
void *__sem_mappings attribute_hidden;

/* Lock to protect the search tree.  */
int __sem_mappings_lock attribute_hidden = LLL_LOCK_INITIALIZER;


/* Search for existing mapping and if possible add the one provided.  */
static sem_t *
check_add_mapping (const char *name, size_t namelen, int fd, sem_t *existing)
{
  sem_t *result = SEM_FAILED;

  /* Get the information about the file.  */
  struct stat64 st;
  if (__fxstat64 (_STAT_VER, fd, &st) == 0)
    {
      /* Get the lock.  */
      lll_lock (__sem_mappings_lock, LLL_PRIVATE);

      /* Search for an existing mapping given the information we have.  */
      struct inuse_sem *fake;
      fake = (struct inuse_sem *) alloca (sizeof (*fake) + namelen);
      memcpy (fake->name, name, namelen);
      fake->dev = st.st_dev;
      fake->ino = st.st_ino;

      struct inuse_sem **foundp = tfind (fake, &__sem_mappings, __sem_search);
      if (foundp != NULL)
	{
	  /* There is already a mapping.  Use it.  */
	  result = (*foundp)->sem;
	  ++(*foundp)->refcnt;
	}
      else
	{
	  /* We haven't found a mapping.  Install ione.  */
	  struct inuse_sem *newp;

	  newp = (struct inuse_sem *) malloc (sizeof (*newp) + namelen);
	  if (newp != NULL)
	    {
	      /* If the caller hasn't provided any map it now.  */
	      if (existing == SEM_FAILED)
		existing = (sem_t *) mmap (NULL, sizeof (sem_t),
					   PROT_READ | PROT_WRITE, MAP_SHARED,
					   fd, 0);

	      newp->dev = st.st_dev;
	      newp->ino = st.st_ino;
	      newp->refcnt = 1;
	      newp->sem = existing;
	      memcpy (newp->name, name, namelen);

	      /* Insert the new value.  */
	      if (existing != MAP_FAILED
		  && tsearch (newp, &__sem_mappings, __sem_search) != NULL)
		/* Successful.  */
		result = existing;
	      else
		/* Something went wrong while inserting the new
		   value.  We fail completely.  */
		free (newp);
	    }
	}

      /* Release the lock.  */
      lll_unlock (__sem_mappings_lock, LLL_PRIVATE);
    }

  if (result != existing && existing != SEM_FAILED && existing != MAP_FAILED)
    {
      /* Do not disturb errno.  */
      INTERNAL_SYSCALL_DECL (err);
      INTERNAL_SYSCALL (munmap, err, 2, existing, sizeof (sem_t));
    }

  return result;
}


sem_t *
sem_open (const char *name, int oflag, ...)
{
  char *finalname;
  sem_t *result = SEM_FAILED;
  int fd;

  /* Determine where the shmfs is mounted.  */
  INTUSE(__pthread_once) (&__namedsem_once, __where_is_shmfs);

  /* If we don't know the mount points there is nothing we can do.  Ever.  */
  if (mountpoint.dir == NULL)
    {
      __set_errno (ENOSYS);
      return SEM_FAILED;
    }

  /* Construct the filename.  */
  while (name[0] == '/')
    ++name;

  if (name[0] == '\0')
    {
      /* The name "/" is not supported.  */
      __set_errno (EINVAL);
      return SEM_FAILED;
    }
  size_t namelen = strlen (name) + 1;

  /* Create the name of the final file.  */
  finalname = (char *) alloca (mountpoint.dirlen + namelen);
  mempcpy (mempcpy (finalname, mountpoint.dir, mountpoint.dirlen),
	     name, namelen);

  /* If the semaphore object has to exist simply open it.  */
  if ((oflag & O_CREAT) == 0 || (oflag & O_EXCL) == 0)
    {
    try_again:
      fd = __libc_open (finalname,
			(oflag & ~(O_CREAT|O_ACCMODE)) | O_NOFOLLOW | O_RDWR);

      if (fd == -1)
	{
	  /* If we are supposed to create the file try this next.  */
	  if ((oflag & O_CREAT) != 0 && errno == ENOENT)
	    goto try_create;

	  /* Return.  errno is already set.  */
	}
      else
	/* Check whether we already have this semaphore mapped and
	   create one if necessary.  */
	result = check_add_mapping (name, namelen, fd, SEM_FAILED);
    }
  else
    {
      /* We have to open a temporary file first since it must have the
	 correct form before we can start using it.  */
      char *tmpfname;
      mode_t mode;
      unsigned int value;
      va_list ap;

    try_create:
      va_start (ap, oflag);

      mode = va_arg (ap, mode_t);
      value = va_arg (ap, unsigned int);

      va_end (ap);

      if (value > SEM_VALUE_MAX)
	{
	  __set_errno (EINVAL);
	  return SEM_FAILED;
	}

      /* Create the initial file content.  */
      union
      {
	sem_t initsem;
	struct new_sem newsem;
      } sem;

      sem.newsem.value = value;
      sem.newsem.private = 0;
      sem.newsem.nwaiters = 0;

      /* Initialize the remaining bytes as well.  */
      memset ((char *) &sem.initsem + sizeof (struct new_sem), '\0',
	      sizeof (sem_t) - sizeof (struct new_sem));

      tmpfname = (char *) alloca (mountpoint.dirlen + 6 + 1);
      mempcpy (mempcpy (tmpfname, mountpoint.dir, mountpoint.dirlen),
	"XXXXXX", 7);

      fd = __gen_tempname (tmpfname, __GT_FILE, 0, 0, mode);
      if (fd == -1)
        return SEM_FAILED;

      if (TEMP_FAILURE_RETRY (__libc_write (fd, &sem.initsem, sizeof (sem_t)))
	  == sizeof (sem_t)
	  /* Map the sem_t structure from the file.  */
	  && (result = (sem_t *) mmap (NULL, sizeof (sem_t),
				       PROT_READ | PROT_WRITE, MAP_SHARED,
				       fd, 0)) != MAP_FAILED)
	{
	  /* Create the file.  Don't overwrite an existing file.  */
	  if (link (tmpfname, finalname) != 0)
	    {
	      /* Undo the mapping.  */
	      (void) munmap (result, sizeof (sem_t));

	      /* Reinitialize 'result'.  */
	      result = SEM_FAILED;

	      /* This failed.  If O_EXCL is not set and the problem was
		 that the file exists, try again.  */
	      if ((oflag & O_EXCL) == 0 && errno == EEXIST)
		{
		  /* Remove the file.  */
		  (void) unlink (tmpfname);

		  /* Close the file.  */
		  (void) __libc_close (fd);

		  goto try_again;
		}
	    }
	  else
	    /* Insert the mapping into the search tree.  This also
	       determines whether another thread sneaked by and already
	       added such a mapping despite the fact that we created it.  */
	    result = check_add_mapping (name, namelen, fd, result);
	}

      /* Now remove the temporary name.  This should never fail.  If
	 it fails we leak a file name.  Better fix the kernel.  */
      (void) unlink (tmpfname);
    }

  /* Map the mmap error to the error we need.  */
  if (MAP_FAILED != (void *) SEM_FAILED && result == MAP_FAILED)
    result = SEM_FAILED;

  /* We don't need the file descriptor anymore.  */
  if (fd != -1)
    {
      /* Do not disturb errno.  */
      INTERNAL_SYSCALL_DECL (err);
      INTERNAL_SYSCALL (close, err, 1, fd);
    }

  return result;
}
