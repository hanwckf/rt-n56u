/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2010
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
 
#ifndef _MSG_H_
#define _MSG_H_

#include "config.h"

/* Debug message event */
#define MSG_EVT_NONE        0x00000000	/* No event */
#define MSG_EVT_DMA         0x00000001	/* DMA related event */
#define MSG_EVT_CMD         0x00000002	/* MSDC CMD related event */
#define MSG_EVT_RSP         0x00000004	/* MSDC CMD RSP related event */
#define MSG_EVT_INT         0x00000008	/* MSDC INT event */
#define MSG_EVT_CFG         0x00000010	/* MSDC CFG event */
#define MSG_EVT_FUC         0x00000020	/* Function event */
#define MSG_EVT_OPS         0x00000040	/* Read/Write operation event */
#define MSG_EVT_FIO         0x00000080	/* FIFO operation event */
#define MSG_EVT_INF         0x01000000  /* information event */
#define MSG_EVT_WRN         0x02000000  /* Warning event */
#define MSG_EVT_ERR         0x04000000  /* Error event */

#define MSG_EVT_ALL         0xffffffff

//#define MSG_EVT_MASK       \
//    (MSG_EVT_ALL & ~MSG_EVT_DMA & ~MSG_EVT_WRN & ~MSG_EVT_RSP & ~MSG_EVT_INT & ~MSG_EVT_CMD & ~MSG_EVT_OPS)
//#define MSG_EVT_MASK       (MSG_EVT_ALL & ~MSG_EVT_FIO)
//#define MSG_EVT_MASK       (MSG_EVT_FIO)
#define MSG_EVT_MASK       (MSG_EVT_ALL)

#undef MSG

#ifdef MSG_DEBUG
#define MSG(evt, fmt, args...) \
do {	\
    if ((MSG_EVT_##evt) & MSG_EVT_MASK) { \
        printf(fmt, ##args); \
    } \
} while(0)

#define MSG_FUNC(f)	MSG(FUC, "<FUNC>: %s\n", __FUNCTION__)
#else
#define MSG(evt, fmt, args...)
#define MSG_FUNC(f)
#endif

// extern void dbg_print(char *fmt,...);

#endif /* end of _MSG_H_ */

