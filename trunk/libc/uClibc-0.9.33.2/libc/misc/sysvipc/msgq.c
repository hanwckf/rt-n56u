#include <errno.h>
#include <sys/msg.h>
#include "ipc.h"
#ifdef __UCLIBC_HAS_THREADS_NATIVE__
#include "sysdep-cancel.h"
#else
#define SINGLE_THREAD_P 1
#endif


#ifdef L_msgctl

#ifdef __NR_msgctl
#define __NR___libc_msgctl __NR_msgctl
static __inline__ _syscall3(int, __libc_msgctl, int, msqid, int, cmd, struct msqid_ds *, buf)
#endif
/* Message queue control operation.  */
int msgctl(int msqid, int cmd, struct msqid_ds *buf)
{
#ifdef __NR_msgctl
	return __libc_msgctl(msqid, cmd | __IPC_64, buf);
#else
    return __syscall_ipc(IPCOP_msgctl, msqid, cmd | __IPC_64, 0, buf, 0);
#endif
}
#endif


#ifdef L_msgget
#ifdef __NR_msgget
_syscall2(int, msgget, key_t, key, int, msgflg)
#else
/* Get messages queue.  */
int msgget (key_t key, int msgflg)
{
    return __syscall_ipc(IPCOP_msgget ,key ,msgflg ,0 ,0, 0);
}
#endif
#endif


struct new_msg_buf{
    struct msgbuf * oldmsg;
    long int r_msgtyp;       /* the fifth arg of __syscall_ipc */
};
/* Receive message from message queue.  */


#ifdef L_msgrcv
#ifdef __NR_msgrcv
#define __NR___syscall_msgrcv __NR_msgrcv
static inline _syscall5(ssize_t, __syscall_msgrcv, int, msqid, void *, msgp,
			size_t, msgsz, long int, msgtyp, int, msgflg)
#endif
static inline ssize_t do_msgrcv (int msqid, void *msgp, size_t msgsz,
			    long int msgtyp, int msgflg)
{
#ifdef __NR_msgrcv
    return __syscall_msgrcv(msqid, msgp, msgsz, msgtyp, msgflg);
#else
    struct new_msg_buf temp;

    temp.r_msgtyp = msgtyp;
    temp.oldmsg = msgp;
    return __syscall_ipc(IPCOP_msgrcv ,msqid ,msgsz ,msgflg ,&temp, 0);
#endif
}
ssize_t msgrcv (int msqid, void *msgp, size_t msgsz,
	    long int msgtyp, int msgflg)
{
    if (SINGLE_THREAD_P)
	return do_msgrcv(msqid, msgp, msgsz, msgtyp, msgflg);
#ifdef __UCLIBC_HAS_THREADS_NATIVE__
    int oldtype = LIBC_CANCEL_ASYNC ();
    int result = do_msgrcv(msqid, msgp, msgsz, msgtyp, msgflg);
    LIBC_CANCEL_RESET (oldtype);
    return result;
#endif
}
#endif



#ifdef L_msgsnd
#ifdef __NR_msgsnd
#define __NR___syscall_msgsnd __NR_msgsnd
static inline _syscall4(int, __syscall_msgsnd, int, msqid, const void *, msgp,
			size_t, msgsz, int, msgflg)
#endif
/* Send message to message queue.  */
static inline int do_msgsnd (int msqid, const void *msgp, size_t msgsz,
			    int msgflg)
{
#ifdef __NR_msgsnd
    return __syscall_msgsnd(msqid, msgp, msgsz, msgflg);
#else
    return __syscall_ipc(IPCOP_msgsnd, msqid, msgsz, msgflg, (void *)msgp, 0);
#endif
}
int msgsnd (int msqid, const void *msgp, size_t msgsz, int msgflg)
{
    if (SINGLE_THREAD_P)
	return do_msgsnd(msqid, msgp, msgsz, msgflg);
#ifdef __UCLIBC_HAS_THREADS_NATIVE__
    int oldtype = LIBC_CANCEL_ASYNC ();
    int result = do_msgsnd(msqid, msgp, msgsz, msgflg);
    LIBC_CANCEL_RESET (oldtype);
    return result;
#endif
}
#endif

