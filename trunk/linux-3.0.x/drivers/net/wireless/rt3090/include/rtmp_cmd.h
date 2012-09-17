#ifndef __RTMP_CMD_H__
#define __RTMP_CMD_H__

#include "rtmp_type.h"


typedef struct _CmdQElmt {
	UINT				command;
	PVOID				buffer;
	ULONG				bufferlength;
	BOOLEAN				CmdFromNdis;
	BOOLEAN				SetOperation;
	struct _CmdQElmt	*next;
} CmdQElmt, *PCmdQElmt;

typedef struct _CmdQ {
	UINT				size;
	CmdQElmt			*head;
	CmdQElmt			*tail;
	UINT32				CmdQState;
} CmdQ, *PCmdQ;


#define EnqueueCmd(cmdq, cmdqelmt)		\
{										\
	if (cmdq->size == 0)				\
		cmdq->head = cmdqelmt;			\
	else								\
		cmdq->tail->next = cmdqelmt;	\
	cmdq->tail = cmdqelmt;				\
	cmdqelmt->next = NULL;				\
	cmdq->size++;						\
}

#define NDIS_OID	UINT

#ifdef KTHREAD_SUPPORT
#define RTCMDUp(pAd) \
	do{	\
		RTMP_OS_TASK	*_pTask = &((pAd)->cmdQTask);	\
		{ \
			_pTask->kthread_running = TRUE; \
	        wake_up(&_pTask->kthread_q); \
		} \
	}while(0)
#else
#define RTCMDUp(pAd)	                \
	do{									    \
		RTMP_OS_TASK	*_pTask = &((pAd)->cmdQTask);	\
		CHECK_PID_LEGALITY(_pTask->taskPID)	    \
		{\
			RTMP_SEM_EVENT_UP(&(_pTask->taskSema)); \
		}\
	}while(0)
#endif


void  getRate(HTTRANSMIT_SETTING HTSetting, ULONG* fLastTxRxRate);

#endif // __RTMP_CMD_H__ //
