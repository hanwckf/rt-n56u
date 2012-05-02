#include <errno.h>
#include <sys/msg.h>
#include "ipc.h"


#ifdef L_msgctl

#ifdef __NR_msgctl
#define __NR___libc_msgctl __NR_msgctl
_syscall3(int, __libc_msgctl, int, msqid, int, cmd, struct msqid_ds *, buf);
#endif
/* Message queue control operation.  */
int msgctl(int msqid, int cmd, struct msqid_ds *buf)
{
#ifdef __NR_msgctl
	return __libc_msgctl(msqid, cmd | __IPC_64, buf);
#else
    return __syscall_ipc(IPCOP_msgctl, msqid, cmd | __IPC_64, 0, buf);
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
    return __syscall_ipc(IPCOP_msgget ,key ,msgflg ,0 ,0);
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
_syscall5(int, msgrcv, int, msqid, void *, msgp, size_t, msgsz, long int, msgtyp, int, msgflg);
#else
int msgrcv (int msqid, void *msgp, size_t msgsz,
	long int msgtyp, int msgflg)
{
    struct new_msg_buf temp;

    temp.r_msgtyp = msgtyp;
    temp.oldmsg = msgp;
    return __syscall_ipc(IPCOP_msgrcv ,msqid ,msgsz ,msgflg ,&temp);
}
#endif
#endif



#ifdef L_msgsnd
#ifdef __NR_msgsnd
_syscall4(int, msgsnd, int, msqid, const void *, msgp, size_t, msgsz, int, msgflg);
#else
/* Send message to message queue.  */
int msgsnd (int msqid, const void *msgp, size_t msgsz, int msgflg)
{
    return __syscall_ipc(IPCOP_msgsnd, msqid, msgsz, msgflg, (void *)msgp);
}
#endif
#endif

