#include <stdio.h>
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
    printf("%d\n", k);

    if (k < 0) {
	printf("semget failed: %m\n");
	return 1;
    }

    sd.buf = &sd_buf;
    r = semctl(k, 0, IPC_STAT, sd);
    printf("%d\n", r);

    if (r < 0) {
	printf("semctl IPC_STAT failed: %m\n");
	return 1;
    }

    printf("sem_nsems = %lu\n", sd_buf.sem_nsems);
    if (sd_buf.sem_nsems != 10) {
	printf("failed: incorrect sem_nsems!\n");
	return 1;
    }

    printf("succeeded\n");

    return 0;
}
