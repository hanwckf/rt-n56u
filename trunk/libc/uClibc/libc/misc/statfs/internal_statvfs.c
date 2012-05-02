/* Copyright (C) 1998, 1999, 2000, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

  /* Now fill in the fields we have information for.  */
  buf->f_bsize = fsbuf.f_bsize;
  /* Linux does not support f_frsize, so set it to the full block size.  */
  buf->f_frsize = fsbuf.f_bsize;
  buf->f_blocks = fsbuf.f_blocks;
  buf->f_bfree = fsbuf.f_bfree;
  buf->f_bavail = fsbuf.f_bavail;
  buf->f_files = fsbuf.f_files;
  buf->f_ffree = fsbuf.f_ffree;
  if (sizeof (buf->f_fsid) == sizeof (fsbuf.f_fsid))
    buf->f_fsid = (fsbuf.f_fsid.__val[0]
		   | ((unsigned long int) fsbuf.f_fsid.__val[1]
		      << (8 * (sizeof (buf->f_fsid)
			       - sizeof (fsbuf.f_fsid.__val[0])))));
  else
    /* We cannot help here.  The statvfs element is not large enough to
       contain both words of the statfs f_fsid field.  */
    buf->f_fsid = fsbuf.f_fsid.__val[0];
#ifdef _STATVFSBUF_F_UNUSED
  buf->__f_unused = 0;
#endif
  buf->f_namemax = fsbuf.f_namelen;
  memset (buf->__f_spare, '\0', 6 * sizeof (int));

  /* What remains to do is to fill the fields f_favail and f_flag.  */

  /* XXX I have no idea how to compute f_favail.  Any idea???  */
  buf->f_favail = buf->f_ffree;

  /* Determining the flags is tricky.  We have to read /proc/mounts or
     the /etc/mtab file and search for the entry which matches the given
     file.  The way we can test for matching filesystem is using the
     device number.  */
  buf->f_flag = 0;
  if (STAT (&st) >= 0)
    {
      int save_errno = errno;
      struct mntent mntbuf;
      FILE *mtab;

      mtab = setmntent ("/proc/mounts", "r");
      if (mtab == NULL)
	mtab = setmntent (_PATH_MOUNTED, "r");

      if (mtab != NULL)
	{
	  char tmpbuf[1024];

	  while (getmntent_r (mtab, &mntbuf, tmpbuf, sizeof (tmpbuf)))
	    {
	      struct stat fsst;

	      /* Find out about the device the current entry is for.  */
	      if (stat (mntbuf.mnt_dir, &fsst) >= 0
		  && st.st_dev == fsst.st_dev)
		{
		  /* Bingo, we found the entry for the device FD is on.
		     Now interpret the option string.  */
		  char *cp = mntbuf.mnt_opts;
		  char *opt;

		  while ((opt = strsep (&cp, ",")) != NULL)
		    if (strcmp (opt, "ro") == 0)
		      buf->f_flag |= ST_RDONLY;
		    else if (strcmp (opt, "nosuid") == 0)
		      buf->f_flag |= ST_NOSUID;
		    else if (strcmp (opt, "noexec") == 0)
		      buf->f_flag |= ST_NOEXEC;
		    else if (strcmp (opt, "nodev") == 0)
		      buf->f_flag |= ST_NODEV;
		    else if (strcmp (opt, "sync") == 0)
		      buf->f_flag |= ST_SYNCHRONOUS;
		    else if (strcmp (opt, "mand") == 0)
		      buf->f_flag |= ST_MANDLOCK;
		    else if (strcmp (opt, "noatime") == 0)
		      buf->f_flag |= ST_NOATIME;
		    else if (strcmp (opt, "nodiratime") == 0)
		      buf->f_flag |= ST_NODIRATIME;

		  /* We can stop looking for more entries.  */
		  break;
		}
	    }

	  /* Close the file.  */
	  endmntent (mtab);
	}

      __set_errno (save_errno);
    }
