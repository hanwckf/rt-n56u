#ifndef IPC_H
#define IPC_H
#include <syscall.h>
#include <bits/wordsize.h>

#if __WORDSIZE == 32 || defined __alpha__ || defined __mips__
# define __IPC_64	0x100
#else
# define __IPC_64	0x0
#endif

#ifdef __NR_ipc

/* The actual system call: all functions are multiplexed by this.  */
extern int __syscall_ipc (unsigned int __call, long __first, long __second,
					  long __third, void *__ptr, void *__fifth) attribute_hidden;


/* The codes for the functions to use the multiplexer `__syscall_ipc'.  */
#define IPCOP_semop	 1
#define IPCOP_semget	 2
#define IPCOP_semctl	 3
#define IPCOP_semtimedop 4
#define IPCOP_msgsnd	11
#define IPCOP_msgrcv	12
#define IPCOP_msgget	13
#define IPCOP_msgctl	14
#define IPCOP_shmat	21
#define IPCOP_shmdt	22
#define IPCOP_shmget	23
#define IPCOP_shmctl	24

#endif

#endif							/* IPC_H */
