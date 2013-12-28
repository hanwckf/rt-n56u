#ifndef __BIN_SEM_ASUS_H__
#define __BIN_SEM_ASUS_H__

/* our shared key (syslogd.c and logread.c must be in sync) */
enum { KEY_ID = 0x53555341 }; /* "ASUS" */

/* We must define union semun ourselves. */
union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
	struct seminfo *__buf;
};

extern int bin_sem_alloc(key_t key, int sem_flags);
extern int bin_sem_dealloc(int semid);
extern int bin_sem_init();
extern int bin_sem_wait();
extern int bin_sem_post();

extern int file_lock(char *tag);
extern void file_unlock(int fd_lock);

#endif
