/*
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#ifndef _SYS_SHM_H
# error "Never include <bits/shm.h> directly; use <sys/shm.h> instead."
#endif

#include <bits/types.h>

/* Permission flag for shmget.  */
#define SHM_R		0400		/* or S_IRUGO from <linux/stat.h> */
#define SHM_W		0200		/* or S_IWUGO from <linux/stat.h> */

/* Flags for `shmat'.  */
#define SHM_RDONLY	010000		/* attach read-only else read-write */
#define SHM_RND		020000		/* round attach address to SHMLBA */
#define SHM_REMAP	040000		/* take-over region on attach */

/* Commands for `shmctl'.  */
#define SHM_LOCK	11		/* lock segment (root only) */
#define SHM_UNLOCK	12		/* unlock segment (root only) */

__BEGIN_DECLS

/* Segment low boundary address multiple.  */
#define SHMLBA		(__getpagesize () << 2)
extern int __getpagesize (void) __THROW __attribute__ ((__const__));


/* Type to count number of attaches.  */
typedef unsigned long int shmatt_t;

/* Data structure describing a set of semaphores.  */
struct shmid_ds
  {
    struct ipc_perm shm_perm;		/* operation permission struct */
    size_t shm_segsz;			/* size of segment in bytes */
    __time_t shm_atime;			/* time of last shmat() */
    unsigned long int __unused1;
    __time_t shm_dtime;			/* time of last shmdt() */
    unsigned long int __unused2;
    __time_t shm_ctime;			/* time of last change by shmctl() */
    unsigned long int __unused3;
    __pid_t shm_cpid;			/* pid of creator */
    __pid_t shm_lpid;			/* pid of last shmop */
    shmatt_t shm_nattch;		/* number of current attaches */
    unsigned long int __unused4;
    unsigned long int __unused5;
  };

#ifdef __USE_MISC

/* ipcs ctl commands */
# define SHM_STAT 	13
# define SHM_INFO 	14

/* shm_mode upper byte flags */
# define SHM_DEST	01000	/* segment will be destroyed on last detach */
# define SHM_LOCKED	02000   /* segment will not be swapped */
# define SHM_HUGETLB	04000	/* segment is mapped via hugetlb */
# define SHM_NORESERVE	010000	/* don't check for reservations */

struct	shminfo
  {
    unsigned long int shmmax;
    unsigned long int shmmin;
    unsigned long int shmmni;
    unsigned long int shmseg;
    unsigned long int shmall;
    unsigned long int __unused1;
    unsigned long int __unused2;
    unsigned long int __unused3;
    unsigned long int __unused4;
  };

struct shm_info
  {
    int used_ids;
    unsigned long int shm_tot;	/* total allocated shm */
    unsigned long int shm_rss;	/* total resident shm */
    unsigned long int shm_swp;	/* total swapped shm */
    unsigned long int swap_attempts;
    unsigned long int swap_successes;
  };

#endif /* __USE_MISC */

__END_DECLS
