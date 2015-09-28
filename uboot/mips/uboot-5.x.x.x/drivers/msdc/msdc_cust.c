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

#include "msdc_cust.h"

struct msdc_cust msdc_cap = {
    MSDC_CLKSRC_48MHZ, /* host clock source             */
    MSDC_SMPL_FALLING,   /* command latch edge            */
//    MSDC_SMPL_RISING,   /* command latch edge            */
    MSDC_SMPL_FALLING,   /* data latch edge               */
//    MSDC_SMPL_RISING,   /* data latch edge               */
    4,                  /* clock pad driving             */
    4,                  /* command pad driving           */
    4,                  /* data pad driving              */
    4,                  /* clock pad driving on 1.8V     */
    4,                  /* command pad driving on 1.8V   */
    4,                  /* data pad driving on 1.8V      */
#if defined (EMMC_8BIT)
    8,                  /* data pins                     */
#else
    4,                  /* data pins                     */
#endif
    0,                  /* data address offset           */

    /* hardware capability flags     */
    MSDC_HIGHSPEED|MSDC_WP_PIN_EN|MSDC_SYS_SUSPEND|MSDC_REMOVABLE|MSDC_CD_PIN_EN,
    //MSDC_HIGHSPEED|MSDC_DDR|MSDC_UHS1|MSDC_SDIO_IRQ|MSDC_RST_PIN_EN|MSDC_REMOVABLE|MSDC_CD_PIN_EN,
#if 0
    MSDC_CD_PIN_EN|MSDC_WP_PIN_EN|MSDC_SDIO_IRQ|MSDC_REMOVABLE|
    MSDC_HIGHSPEED|MSDC_UHS1|MSDC_DDR, 
#endif    
};


