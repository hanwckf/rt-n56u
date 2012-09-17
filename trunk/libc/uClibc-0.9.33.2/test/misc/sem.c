#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int main(void)
{
	int k, r;
	union semun {
		int val;
		struct semid_ds *buf;
		unsigned short int *array;
		struct seminfo *__buf;
	} sd;
	struct semid_ds sd_buf;

	k = semget(IPC_PRIVATE, 10, IPC_CREAT | 0666 );
	printf("semget(IPC_CREAT) = %d\n", k);

	if (k < 0) {
		fprintf(stderr, "semget failed: %s\n", strerror(errno));
		return 1;
	}

	sd.buf = &sd_buf;
	r = semctl(k, 0, IPC_STAT, sd);
	printf("semctl(k) = %d\n", r);

	if (r < 0) {
		perror("semctl IPC_STAT failed");
		return 1;
	}

	printf("sem_nsems = %lu\n", sd_buf.sem_nsems);
	if (sd_buf.sem_nsems != 10) {
		fprintf(stderr, "failed: incorrect sem_nsems!\n");
		return 1;
	}

	printf("succeeded\n");

	return 0;
}
