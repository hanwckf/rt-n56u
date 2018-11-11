/*
 ***************************************************************************
 * MediaTek Inc. 
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_txbf_cal_mt.c
*/

#ifdef COMPOS_WIN
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "Cmm_txbf_cal_mt.h"
#endif
#elif defined (COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#endif

#ifdef TXBF_SUPPORT

UINT_16 au2IBfCalEEPROMOffset[9] = {
    IBF_LNA_PHASE_G0_ADDR,
    IBF_LNA_PHASE_G1_ADDR,
    IBF_LNA_PHASE_G2_ADDR,
    IBF_LNA_PHASE_G3_ADDR,
    IBF_LNA_PHASE_G4_ADDR,
    IBF_LNA_PHASE_G5_ADDR,
    IBF_LNA_PHASE_G6_ADDR,
    IBF_LNA_PHASE_G7_ADDR,
    IBF_LNA_PHASE_G8_ADDR};


VOID E2pMemWrite(IN PRTMP_ADAPTER pAd,
                 IN UINT16 u2MemAddr,
                 IN UCHAR  ucInput_L,
                 IN UCHAR  ucInput_H)
{
    UINT16 u2Value;
    
    if ((u2MemAddr & 0x1) != 0)
    {
        u2Value = (ucInput_L << 8) | ucInput_H;
    }
    else
    {
        u2Value = (ucInput_H << 8) | ucInput_L;
    }

    RT28xx_EEPROM_WRITE16(pAd, u2MemAddr, u2Value);
}


VOID iBFPhaseCalE2PInit(IN PRTMP_ADAPTER pAd)
{
    UINT_16 u2BfE2pOccupiedSize;

    // Group 0
    u2BfE2pOccupiedSize = sizeof(IBF_PHASE_G0_T);
    RT28xx_EEPROM_READ_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[GROUP_0], u2BfE2pOccupiedSize, (PUCHAR)&pAd->iBfPhaseG0);

    // Group 1 ~ 7
    u2BfE2pOccupiedSize = (sizeof(IBF_PHASE_Gx_T) * 7) - 4;
    RT28xx_EEPROM_READ_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[GROUP_1], u2BfE2pOccupiedSize, (PUCHAR)&pAd->iBfPhaseGx[0]);
    
    // Group 8
    u2BfE2pOccupiedSize = sizeof(IBF_PHASE_Gx_T);
    RT28xx_EEPROM_READ_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[GROUP_8], u2BfE2pOccupiedSize, (PUCHAR)&pAd->iBfPhaseGx[7]);

    pAd->fgCalibrationFail = FALSE;
    NdisZeroMemory(&pAd->fgGroupIdPassFailStatus[0], 9);
}


VOID iBFPhaseCalE2PUpdate(IN PRTMP_ADAPTER pAd,
                          IN UCHAR   ucGroup,
                          IN BOOLEAN fgSX2,
                          IN UCHAR   ucUpdateAllTye)
{
    IBF_PHASE_Gx_T iBfPhaseGx;
    IBF_PHASE_G0_T iBfPhaseG0;
    UCHAR  ucGroupIdx, ucEndLoop, ucCounter;
    UCHAR  ucIBfGroupSize;
    UCHAR  ucBuf[64];
    //UINT16 u2Value;
   

    /* IF phase calibration is for BW20/40/80/160 */
    ucGroupIdx = 0;
    NdisZeroMemory(&iBfPhaseG0, sizeof(IBF_PHASE_G0_T));
    NdisZeroMemory(&iBfPhaseGx, sizeof(IBF_PHASE_Gx_T));

    switch (ucUpdateAllTye)
    {
        case IBF_PHASE_ONE_GROUP_UPDATE:
            ucGroupIdx = ucGroup - 1;
            ucEndLoop  = 0;
            
            if (ucGroup == GROUP_0)
            {
                RT28xx_EEPROM_WRITE_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[ucGroup], sizeof(IBF_PHASE_G0_T), (PUCHAR)&pAd->iBfPhaseG0);
                NdisCopyMemory(&iBfPhaseG0, &pAd->iBfPhaseG0, sizeof(IBF_PHASE_G0_T));
            }
            else if (ucGroup == GROUP_2)
            {
                RT28xx_EEPROM_WRITE_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[ucGroup], (sizeof(IBF_PHASE_Gx_T) - 4), (PUCHAR)&pAd->iBfPhaseGx[ucGroupIdx]);
                NdisCopyMemory(&iBfPhaseGx, &pAd->iBfPhaseGx[ucGroupIdx], (sizeof(IBF_PHASE_Gx_T) - 4));
            }
            else
            {
                RT28xx_EEPROM_WRITE_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[ucGroup], sizeof(IBF_PHASE_Gx_T), (PUCHAR)&pAd->iBfPhaseGx[ucGroupIdx]);
                NdisCopyMemory(&iBfPhaseGx, &pAd->iBfPhaseGx[ucGroupIdx], sizeof(IBF_PHASE_Gx_T));
            }

            if (ucGroup == GROUP_0)
            {
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d and Group_L\n G%d_L_T0_H = %d\n G%d_L_T1_H = %d\n G%d_L_T2_H = %d\n", 
                                                                     ucGroup,
                                                                     ucGroup, iBfPhaseG0.ucG0_L_T0_H,
                                                                     ucGroup, iBfPhaseG0.ucG0_L_T1_H,
                                                                     ucGroup, iBfPhaseG0.ucG0_L_T2_H));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d and Group_M\n G%d_M_T0_H = %d\n G%d_M_T1_H = %d\n G%d_M_T2_H = %d\n", 
                                                                     ucGroup,
                                                                     ucGroup, iBfPhaseG0.ucG0_M_T0_H,
                                                                     ucGroup, iBfPhaseG0.ucG0_M_T1_H,
                                                                     ucGroup, iBfPhaseG0.ucG0_M_T2_H));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d_R0_H = %d\n G%d_R0_M = %d\n G%d_R0_L = %d\n G%d_R1_H = %d\n G%d_R1_M = %d\n G%d_R1_L = %d\n", 
                                                                     ucGroup, iBfPhaseG0.ucG0_R0_H,
                                                                     ucGroup, iBfPhaseG0.ucG0_R0_M,
                                                                     ucGroup, iBfPhaseG0.ucG0_R0_L,
                                                                     ucGroup, iBfPhaseG0.ucG0_R1_H,
                                                                     ucGroup, iBfPhaseG0.ucG0_R1_M,
                                                                     ucGroup, iBfPhaseG0.ucG0_R1_L));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d_R2_H = %d\n G%d_R2_M = %d\n G%d_R2_L = %d\n G%d_R3_H = %d\n G%d_R3_M = %d\n G%d_R3_L = %d\n", 
                                                                     ucGroup, iBfPhaseG0.ucG0_R2_H,
                                                                     ucGroup, iBfPhaseG0.ucG0_R2_M,
                                                                     ucGroup, iBfPhaseG0.ucG0_R2_L,
                                                                     ucGroup, iBfPhaseG0.ucG0_R3_H,
                                                                     ucGroup, iBfPhaseG0.ucG0_R3_M,
                                                                     ucGroup, iBfPhaseG0.ucG0_R3_L));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d and Group_H\n G%d_H_T0_H = %d\n G%d_H_T1_H = %d\n G%d_H_T2_H = %d\n", 
                                                                     ucGroup,
                                                                     ucGroup, iBfPhaseG0.ucG0_H_T0_H,
                                                                     ucGroup, iBfPhaseG0.ucG0_H_T1_H,
                                                                     ucGroup, iBfPhaseG0.ucG0_H_T2_H));
            }
            else
            {
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d and Group_L\n G%d_L_T0_H = %d\n G%d_L_T1_H = %d\n G%d_L_T2_H = %d\n", 
                                                                     ucGroup,
                                                                     ucGroup, iBfPhaseGx.ucGx_L_T0_H,
                                                                     ucGroup, iBfPhaseGx.ucGx_L_T1_H,
                                                                     ucGroup, iBfPhaseGx.ucGx_L_T2_H));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d and Group_M\n G%d_M_T0_H = %d\n G%d_M_T1_H = %d\n G%d_M_T2_H = %d\n", 
                                                                     ucGroup,
                                                                     ucGroup, iBfPhaseGx.ucGx_M_T0_H,
                                                                     ucGroup, iBfPhaseGx.ucGx_M_T1_H,
                                                                     ucGroup, iBfPhaseGx.ucGx_M_T2_H));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d_R0_H = %d\n G%d_R0_M = %d\n G%d_R0_L = %d\n G%d_R1_H = %d\n G%d_R1_M = %d\n G%d_R1_L = %d\n", 
                                                                     ucGroup, iBfPhaseGx.ucGx_R0_H,
                                                                     ucGroup, iBfPhaseGx.ucGx_R0_M,
                                                                     ucGroup, iBfPhaseGx.ucGx_R0_L,
                                                                     ucGroup, iBfPhaseGx.ucGx_R1_H,
                                                                     ucGroup, iBfPhaseGx.ucGx_R1_M,
                                                                     ucGroup, iBfPhaseGx.ucGx_R1_L));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d_R2_H = %d\n G%d_R2_M = %d\n G%d_R2_L = %d\n G%d_R3_H = %d\n G%d_R3_M = %d\n G%d_R3_L = %d\n", 
                                                                     ucGroup, iBfPhaseGx.ucGx_R2_H,
                                                                     ucGroup, iBfPhaseGx.ucGx_R2_M,
                                                                     ucGroup, iBfPhaseGx.ucGx_R2_L,
                                                                     ucGroup, iBfPhaseGx.ucGx_R3_H,
                                                                     ucGroup, iBfPhaseGx.ucGx_R3_M,
                                                                     ucGroup, iBfPhaseGx.ucGx_R3_L));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d and Group_H\n G%d_H_T0_H = %d\n G%d_H_T1_H = %d\n G%d_H_T2_H = %d\n", 
                                                                     ucGroup,
                                                                     ucGroup, iBfPhaseGx.ucGx_H_T0_H,
                                                                     ucGroup, iBfPhaseGx.ucGx_H_T1_H,
                                                                     ucGroup, iBfPhaseGx.ucGx_H_T2_H));
            }

            break;
        case IBF_PHASE_ALL_GROUP_UPDATE:
            if (pAd->fgCalibrationFail == FALSE)
            {
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("All of groups can pass criterion and calibrated phases can be written into EEPROM\n"));
                NdisCopyMemory(ucBuf, &pAd->iBfPhaseG0, sizeof(IBF_PHASE_G0_T));
                NdisCopyMemory(&ucBuf[sizeof(IBF_PHASE_G0_T)], &pAd->iBfPhaseGx[0], sizeof(IBF_PHASE_Gx_T));
                // Write Group 0 and 1 into EEPROM
                RT28xx_EEPROM_WRITE_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[0], (sizeof(IBF_PHASE_G0_T) + sizeof(IBF_PHASE_Gx_T)),ucBuf);
                // Write Group 2 into EEPROM
                RT28xx_EEPROM_WRITE_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[2], (sizeof(IBF_PHASE_Gx_T) - 4), (PUCHAR)&pAd->iBfPhaseGx[1]);
                // Write Group 3 ~ 7 into EEPROM
                RT28xx_EEPROM_WRITE_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[3], (sizeof(IBF_PHASE_Gx_T) * 5), (PUCHAR)&pAd->iBfPhaseGx[2]);
                // Write Group 8 into EEPROM
                RT28xx_EEPROM_WRITE_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[8], sizeof(IBF_PHASE_Gx_T), (PUCHAR)&pAd->iBfPhaseGx[7]);
            }
            else
            {
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibrated phases can't be written into EEPROM because some groups can't pass criterion!!!\n"));
                for (ucCounter = GROUP_0; ucCounter <= GROUP_8; ucCounter++)
                {
                    MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group%d = %s\n", 
                                                                       ucCounter, 
                                                                      (pAd->fgGroupIdPassFailStatus[ucCounter] == TRUE) ? "FAIL" : "PASS"));
                }
            }
            break;
        case IBF_PHASE_ALL_GROUP_ERASE:
            NdisZeroMemory(&pAd->iBfPhaseG0, sizeof(IBF_PHASE_G0_T));
            NdisZeroMemory(pAd->iBfPhaseGx, sizeof(IBF_PHASE_Gx_T) * 8);
            RT28xx_EEPROM_WRITE_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[GROUP_0], sizeof(IBF_PHASE_G0_T), (PUCHAR)&pAd->iBfPhaseG0);
            RT28xx_EEPROM_WRITE_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[GROUP_1], (sizeof(IBF_PHASE_Gx_T) * 7 - 4), (PUCHAR)&pAd->iBfPhaseGx[0]);
            RT28xx_EEPROM_WRITE_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[GROUP_8], sizeof(IBF_PHASE_Gx_T), (PUCHAR)&pAd->iBfPhaseGx[7]);
            break;
        case IBF_PHASE_ALL_GROUP_READ_FROM_E2P:
            iBFPhaseCalE2PInit(pAd);
            break;
        default:
            ucGroupIdx = ucGroup - 1;
            ucEndLoop  = 0;
            ucIBfGroupSize = sizeof(IBF_PHASE_Gx_T);
            ucIBfGroupSize = (ucGroup == GROUP_2)? (ucIBfGroupSize - 4) : ucIBfGroupSize;
                
            NdisCopyMemory(&iBfPhaseGx, &pAd->iBfPhaseGx[ucGroupIdx], ucIBfGroupSize);
            NdisCopyMemory(&iBfPhaseG0, &pAd->iBfPhaseG0, sizeof(IBF_PHASE_G0_T));
            break;
    }
}


VOID iBFPhaseCalReport(IN PRTMP_ADAPTER pAd,
                       IN UCHAR   ucGroupL_M_H,
                       IN UCHAR   ucGroup,
                       IN BOOLEAN fgSX2,
                       IN UCHAR   ucStatus,
                       IN UCHAR   ucPhaseCalType,
                       IN PUCHAR  pBuf)
{
    IBF_PHASE_Gx_T iBfPhaseGx;
    IBF_PHASE_G0_T iBfPhaseG0;
    IBF_PHASE_OUT  iBfPhaseOut;
    PUCHAR pucIBfPhaseG;
    UCHAR  ucGroupIdx;
    
    MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s :: Calibrated iBF phases\n", __FUNCTION__));

    pucIBfPhaseG = pBuf + sizeof(IBF_PHASE_OUT);
    NdisCopyMemory(&iBfPhaseOut, pBuf, sizeof(IBF_PHASE_OUT));

    switch (ucPhaseCalType)
    {
    case IBF_PHASE_CAL_NOTHING: /* Do nothing */
        break;
    case IBF_PHASE_CAL_NORMAL_INSTRUMENT:
    case IBF_PHASE_CAL_NORMAL: /* Store calibrated phases with buffer mode */
        /* IF phase calibration is for DBDC or 80+80 */
        if ((fgSX2 == TRUE) && (ucGroup > 0))
        {
            ucGroupIdx = ucGroup - 1;
            NdisCopyMemory(&iBfPhaseGx, pucIBfPhaseG, sizeof(IBF_PHASE_Gx_T));

            switch (ucGroupL_M_H)
            {
            case GROUP_L:
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_L_T2_H_SX2 = iBfPhaseGx.ucGx_L_T2_H_SX2;
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("160 iBF phase calibration\n Group_L\n G%d_L_T2_H_SX2 = %d\n ", 
                                                                     ucGroup, iBfPhaseGx.ucGx_L_T2_H_SX2));
                
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group : %d\n", ucGroup));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibration == 1? or Verification == 2? : %d\n", ucPhaseCalType));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibrated result = %d\n", ucStatus));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0 : Means failed\n 1: means pass\n 2: means on-going\n"));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("C0_H : %d, C1_H : %d, C2_H : %d\n C0_M : %d, C1_M : %d, C2_M : %d\n C0_L : %d, C1_L : %d, C2_L : %d\n C3_M : %d, C3_L : %d\n", 
                                                                     iBfPhaseOut.ucC0_H, iBfPhaseOut.ucC1_H, iBfPhaseOut.ucC2_H, 
                                                                     iBfPhaseOut.ucC0_M, iBfPhaseOut.ucC1_M, iBfPhaseOut.ucC2_M,
                                                                     iBfPhaseOut.ucC0_L, iBfPhaseOut.ucC1_L, iBfPhaseOut.ucC2_L, 
                                                                     iBfPhaseOut.ucC3_M, iBfPhaseOut.ucC3_L));

                break;
            case GROUP_M:
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_M_T2_H_SX2 = iBfPhaseGx.ucGx_M_T2_H_SX2;
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_R2_H_SX2   = iBfPhaseGx.ucGx_R2_H_SX2;
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_R2_M_SX2   = iBfPhaseGx.ucGx_R2_M_SX2;
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_R2_L_SX2   = iBfPhaseGx.ucGx_R2_L_SX2;
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_R3_H_SX2   = iBfPhaseGx.ucGx_R3_H_SX2;
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_R3_M_SX2   = iBfPhaseGx.ucGx_R3_M_SX2;
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_R3_L_SX2   = iBfPhaseGx.ucGx_R3_L_SX2;
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group_M\n G%d_M_T2_H_SX2 = %d\n G%d_R2_H_SX2 = %d\n G%d_R2_M_SX2 = %d\n", 
                                                                     ucGroup, iBfPhaseGx.ucGx_H_T2_H_SX2,
                                                                     ucGroup, iBfPhaseGx.ucGx_R2_H_SX2,
                                                                     ucGroup, iBfPhaseGx.ucGx_R2_M_SX2));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d_R2_L_SX2 = %d\n G%d_R3_H_SX2 = %d\n G%d_R3_M_SX2 = %d\n G%d_R3_L_SX2 = %d\n", 
                                                                     ucGroup, iBfPhaseGx.ucGx_R2_L_SX2,
                                                                     ucGroup, iBfPhaseGx.ucGx_R3_H_SX2,
                                                                     ucGroup, iBfPhaseGx.ucGx_R3_M_SX2,
                                                                     ucGroup, iBfPhaseGx.ucGx_R3_L_SX2));

                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group : %d\n", ucGroup));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibration == 1? or Verification == 2? : %d\n", ucPhaseCalType));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibrated result = %d\n", ucStatus));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0 : Means failed\n 1: means pass\n 2: means on-going\n"));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("C0_H : %d, C1_H : %d, C2_H : %d\n C0_M : %d, C1_M : %d, C2_M : %d\n C0_L : %d, C1_L : %d, C2_L : %d\n C3_M : %d, C3_L : %d\n", 
                                                                     iBfPhaseOut.ucC0_H, iBfPhaseOut.ucC1_H, iBfPhaseOut.ucC2_H, 
                                                                     iBfPhaseOut.ucC0_M, iBfPhaseOut.ucC1_M, iBfPhaseOut.ucC2_M,
                                                                     iBfPhaseOut.ucC0_L, iBfPhaseOut.ucC1_L, iBfPhaseOut.ucC2_L, 
                                                                     iBfPhaseOut.ucC3_M, iBfPhaseOut.ucC3_L));

                break;
            case GROUP_H:
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_H_T2_H_SX2 = iBfPhaseGx.ucGx_H_T2_H_SX2;
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group_H\n G%d_H_T2_H_SX2 = %d\n", 
                                                                     ucGroup, iBfPhaseGx.ucGx_H_T2_H_SX2));

                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group : %d\n", ucGroup));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibration == 1? or Verification == 2? : %d\n", ucPhaseCalType));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibrated result = %d\n", ucStatus));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0 : Means failed\n 1: means pass\n 2: means on-going\n"));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("C0_H : %d, C1_H : %d, C2_H : %d\n C0_M : %d, C1_M : %d, C2_M : %d\n C0_L : %d, C1_L : %d, C2_L : %d\n C3_M : %d, C3_L : %d\n", 
                                                                     iBfPhaseOut.ucC0_H, iBfPhaseOut.ucC1_H, iBfPhaseOut.ucC2_H, 
                                                                     iBfPhaseOut.ucC0_M, iBfPhaseOut.ucC1_M, iBfPhaseOut.ucC2_M,
                                                                     iBfPhaseOut.ucC0_L, iBfPhaseOut.ucC1_L, iBfPhaseOut.ucC2_L, 
                                                                     iBfPhaseOut.ucC3_M, iBfPhaseOut.ucC3_L));

                break;
            }  
        
            return;
        }

        /* IF phase calibration is for BW20/40/80 */
        if (ucGroup == GROUP_0)
        {
            NdisCopyMemory(&iBfPhaseGx, pucIBfPhaseG, sizeof(IBF_PHASE_Gx_T));
            NdisCopyMemory(&iBfPhaseG0, pucIBfPhaseG, sizeof(IBF_PHASE_G0_T));
            
            switch (ucGroupL_M_H)
            {
            case GROUP_L:
                pAd->iBfPhaseG0.ucG0_L_T0_H = iBfPhaseGx.ucGx_L_T0_H;
                pAd->iBfPhaseG0.ucG0_L_T1_H = iBfPhaseGx.ucGx_L_T1_H;
                pAd->iBfPhaseG0.ucG0_L_T2_H = iBfPhaseGx.ucGx_L_T2_H;
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G0 and Group_L\n G0_L_T0_H = %d\n G0_L_T1_H = %d\n G0_L_T2_H = %d\n", 
                                                                     iBfPhaseGx.ucGx_L_T0_H,
                                                                     iBfPhaseGx.ucGx_L_T1_H,
                                                                     iBfPhaseGx.ucGx_L_T2_H));
                break;
            case GROUP_M:
                pAd->iBfPhaseG0.ucG0_M_T0_H = iBfPhaseGx.ucGx_M_T0_H;
                pAd->iBfPhaseG0.ucG0_M_T1_H = iBfPhaseGx.ucGx_M_T1_H;
                pAd->iBfPhaseG0.ucG0_M_T2_H = iBfPhaseGx.ucGx_M_T2_H;
                pAd->iBfPhaseG0.ucG0_R0_H   = iBfPhaseG0.ucG0_R0_H;
                pAd->iBfPhaseG0.ucG0_R0_M   = iBfPhaseG0.ucG0_R0_M;
                pAd->iBfPhaseG0.ucG0_R0_L   = iBfPhaseG0.ucG0_R0_L;
                pAd->iBfPhaseG0.ucG0_R1_H   = iBfPhaseG0.ucG0_R1_H;
                pAd->iBfPhaseG0.ucG0_R1_M   = iBfPhaseG0.ucG0_R1_M;
                pAd->iBfPhaseG0.ucG0_R1_L   = iBfPhaseG0.ucG0_R1_L;
                pAd->iBfPhaseG0.ucG0_R2_H   = iBfPhaseG0.ucG0_R2_H;
                pAd->iBfPhaseG0.ucG0_R2_M   = iBfPhaseG0.ucG0_R2_M;
                pAd->iBfPhaseG0.ucG0_R2_L   = iBfPhaseG0.ucG0_R2_L;
                pAd->iBfPhaseG0.ucG0_R3_H   = iBfPhaseG0.ucG0_R3_H;
                pAd->iBfPhaseG0.ucG0_R3_M   = iBfPhaseG0.ucG0_R3_M;
                pAd->iBfPhaseG0.ucG0_R3_L   = iBfPhaseG0.ucG0_R3_L;
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G0 and Group_M\n G0_M_T0_H = %d\n G0_M_T1_H = %d\n G0_M_T2_H = %d\n", 
                                                                     iBfPhaseGx.ucGx_M_T0_H,
                                                                     iBfPhaseGx.ucGx_M_T1_H,
                                                                     iBfPhaseGx.ucGx_M_T2_H));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G0_R0_H = %d\n G0_R0_M = %d\n G0_R0_L = %d\n G0_R1_H = %d\n G0_R1_M = %d\n G0_R1_L = %d\n", 
                                                                     iBfPhaseG0.ucG0_R0_H,
                                                                     iBfPhaseG0.ucG0_R0_M,
                                                                     iBfPhaseG0.ucG0_R0_L,
                                                                     iBfPhaseG0.ucG0_R1_H,
                                                                     iBfPhaseG0.ucG0_R1_M,
                                                                     iBfPhaseG0.ucG0_R1_L));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G0_R2_H = %d\n G0_R2_M = %d\n G0_R2_L = %d\n G0_R3_H = %d\n G0_R3_M = %d\n G0_R3_L = %d\n", 
                                                                     iBfPhaseG0.ucG0_R2_H,
                                                                     iBfPhaseG0.ucG0_R2_M,
                                                                     iBfPhaseG0.ucG0_R2_L,
                                                                     iBfPhaseG0.ucG0_R3_H,
                                                                     iBfPhaseG0.ucG0_R3_M,
                                                                     iBfPhaseG0.ucG0_R3_L));
                
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group : %d\n", 0));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibration == 1? or Verification == 2? : %d\n", ucPhaseCalType));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibrated result = %d\n", ucStatus));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0 : Means failed\n 1: means pass\n 2: means on-going\n"));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("C0_H : %d, C1_H : %d, C2_H : %d\n C0_M : %d, C1_M : %d, C2_M : %d\n C0_L : %d, C1_L : %d, C2_L : %d\n C3_M : %d, C3_L : %d\n", 
                                                                     iBfPhaseOut.ucC0_H, iBfPhaseOut.ucC1_H, iBfPhaseOut.ucC2_H, 
                                                                     iBfPhaseOut.ucC0_M, iBfPhaseOut.ucC1_M, iBfPhaseOut.ucC2_M,
                                                                     iBfPhaseOut.ucC0_L, iBfPhaseOut.ucC1_L, iBfPhaseOut.ucC2_L, 
                                                                     iBfPhaseOut.ucC3_M, iBfPhaseOut.ucC3_L));
              
                break;
            case GROUP_H:
                pAd->iBfPhaseG0.ucG0_H_T0_H = iBfPhaseGx.ucGx_H_T0_H;
                pAd->iBfPhaseG0.ucG0_H_T1_H = iBfPhaseGx.ucGx_H_T1_H;
                pAd->iBfPhaseG0.ucG0_H_T2_H = iBfPhaseGx.ucGx_H_T2_H;
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G1 and Group_H\n G0_H_T0_H = %d\n G0_H_T1_H = %d\n G0_H_T2_H = %d\n", 
                                                                     iBfPhaseGx.ucGx_H_T0_H,
                                                                     iBfPhaseGx.ucGx_H_T1_H,
                                                                     iBfPhaseGx.ucGx_H_T2_H));

                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group : %d\n", 0));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibration == 1? or Verification == 2? : %d\n", ucPhaseCalType));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibrated result = %d\n", ucStatus));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0 : Means failed\n 1: means pass\n 2: means on-going\n"));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("C0_H : %d, C1_H : %d, C2_H : %d\n C0_M : %d, C1_M : %d, C2_M : %d\n C0_L : %d, C1_L : %d, C2_L : %d\n C3_M : %d, C3_L : %d\n", 
                                                                     iBfPhaseOut.ucC0_H, iBfPhaseOut.ucC1_H, iBfPhaseOut.ucC2_H, 
                                                                     iBfPhaseOut.ucC0_M, iBfPhaseOut.ucC1_M, iBfPhaseOut.ucC2_M,
                                                                     iBfPhaseOut.ucC0_L, iBfPhaseOut.ucC1_L, iBfPhaseOut.ucC2_L, 
                                                                     iBfPhaseOut.ucC3_M, iBfPhaseOut.ucC3_L));
                break;
            }
        }
        else
        {
            ucGroupIdx = ucGroup - 1;
            NdisCopyMemory(&iBfPhaseGx, pucIBfPhaseG, sizeof(IBF_PHASE_Gx_T));
            switch (ucGroupL_M_H)
            {
            case GROUP_L:
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_L_T0_H = iBfPhaseGx.ucGx_L_T0_H;
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_L_T1_H = iBfPhaseGx.ucGx_L_T1_H;
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_L_T2_H = iBfPhaseGx.ucGx_L_T2_H;
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d and Group_L\n G%d_L_T0_H = %d\n G%d_L_T1_H = %d\n G%d_L_T2_H = %d\n", 
                                                                     ucGroup,
                                                                     ucGroup, iBfPhaseGx.ucGx_L_T0_H,
                                                                     ucGroup, iBfPhaseGx.ucGx_L_T1_H,
                                                                     ucGroup, iBfPhaseGx.ucGx_L_T2_H));
                
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group : %d\n", ucGroup));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibration == 1? or Verification == 2? : %d\n", ucPhaseCalType));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibrated result = %d\n", ucStatus));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0 : Means failed\n 1: means pass\n 2: means on-going\n"));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("C0_H : %d, C1_H : %d, C2_H : %d\n C0_M : %d, C1_M : %d, C2_M : %d\n C0_L : %d, C1_L : %d, C2_L : %d\n C3_M : %d, C3_L : %d\n", 
                                                                     iBfPhaseOut.ucC0_H, iBfPhaseOut.ucC1_H, iBfPhaseOut.ucC2_H, 
                                                                     iBfPhaseOut.ucC0_M, iBfPhaseOut.ucC1_M, iBfPhaseOut.ucC2_M,
                                                                     iBfPhaseOut.ucC0_L, iBfPhaseOut.ucC1_L, iBfPhaseOut.ucC2_L, 
                                                                     iBfPhaseOut.ucC3_M, iBfPhaseOut.ucC3_L));                
                break;
            case GROUP_M:
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_M_T0_H = iBfPhaseGx.ucGx_M_T0_H;
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_M_T1_H = iBfPhaseGx.ucGx_M_T1_H;
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_M_T2_H = iBfPhaseGx.ucGx_M_T2_H;
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_R0_H   = iBfPhaseGx.ucGx_R0_H;
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_R0_M   = iBfPhaseGx.ucGx_R0_M;
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_R0_L   = iBfPhaseGx.ucGx_R0_L;
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_R1_H   = iBfPhaseGx.ucGx_R1_H;
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_R1_M   = iBfPhaseGx.ucGx_R1_M;
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_R1_L   = iBfPhaseGx.ucGx_R1_L;
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_R2_H   = iBfPhaseGx.ucGx_R2_H;
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_R2_M   = iBfPhaseGx.ucGx_R2_M;
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_R2_L   = iBfPhaseGx.ucGx_R2_L;
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_R3_H   = iBfPhaseGx.ucGx_R3_H;
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_R3_M   = iBfPhaseGx.ucGx_R3_M;
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_R3_L   = iBfPhaseGx.ucGx_R3_L;
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d and Group_M\n G%d_M_T0_H = %d\n G%d_M_T1_H = %d\n G%d_M_T2_H = %d\n", 
                                                                     ucGroup,
                                                                     ucGroup, iBfPhaseGx.ucGx_M_T0_H,
                                                                     ucGroup, iBfPhaseGx.ucGx_M_T1_H,
                                                                     ucGroup, iBfPhaseGx.ucGx_M_T2_H));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d_R0_H = %d\n G%d_R0_M = %d\n G%d_R0_L = %d\n G%d_R1_H = %d\n G%d_R1_M = %d\n G%d_R1_L = %d\n", 
                                                                     ucGroup, iBfPhaseGx.ucGx_R0_H,
                                                                     ucGroup, iBfPhaseGx.ucGx_R0_M,
                                                                     ucGroup, iBfPhaseGx.ucGx_R0_L,
                                                                     ucGroup, iBfPhaseGx.ucGx_R1_H,
                                                                     ucGroup, iBfPhaseGx.ucGx_R1_M,
                                                                     ucGroup, iBfPhaseGx.ucGx_R1_L));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d_R2_H = %d\n G%d_R2_M = %d\n G%d_R2_L = %d\n G%d_R3_H = %d\n G%d_R3_M = %d\n G%d_R3_L = %d\n", 
                                                                     ucGroup, iBfPhaseGx.ucGx_R2_H,
                                                                     ucGroup, iBfPhaseGx.ucGx_R2_M,
                                                                     ucGroup, iBfPhaseGx.ucGx_R2_L,
                                                                     ucGroup, iBfPhaseGx.ucGx_R3_H,
                                                                     ucGroup, iBfPhaseGx.ucGx_R3_M,
                                                                     ucGroup, iBfPhaseGx.ucGx_R3_L));
                
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group : %d\n", ucGroup));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibration == 1? or Verification == 2? : %d\n", ucPhaseCalType));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibrated result = %d\n", ucStatus));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0 : Means failed\n 1: means pass\n 2: means on-going\n"));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("C0_H : %d, C1_H : %d, C2_H : %d\n C0_M : %d, C1_M : %d, C2_M : %d\n C0_L : %d, C1_L : %d, C2_L : %d\n C3_M : %d, C3_L : %d\n", 
                                                                     iBfPhaseOut.ucC0_H, iBfPhaseOut.ucC1_H, iBfPhaseOut.ucC2_H, 
                                                                     iBfPhaseOut.ucC0_M, iBfPhaseOut.ucC1_M, iBfPhaseOut.ucC2_M,
                                                                     iBfPhaseOut.ucC0_L, iBfPhaseOut.ucC1_L, iBfPhaseOut.ucC2_L, 
                                                                     iBfPhaseOut.ucC3_M, iBfPhaseOut.ucC3_L));
                break;
            case GROUP_H:
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_H_T0_H = iBfPhaseGx.ucGx_H_T0_H;
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_H_T1_H = iBfPhaseGx.ucGx_H_T1_H;
                pAd->iBfPhaseGx[ucGroupIdx].ucGx_H_T2_H = iBfPhaseGx.ucGx_H_T2_H;
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d and Group_H\n G%d_H_T0_H = %d\n G%d_H_T1_H = %d\n G%d_H_T2_H = %d\n", 
                                                                     ucGroup,
                                                                     ucGroup, iBfPhaseGx.ucGx_H_T0_H,
                                                                     ucGroup, iBfPhaseGx.ucGx_H_T1_H,
                                                                     ucGroup, iBfPhaseGx.ucGx_H_T2_H));
                
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group : %d\n", ucGroup));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibration == 1? or Verification == 2? : %d\n", ucPhaseCalType));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibrated result = %d\n", ucStatus));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0 : Means failed\n 1: means pass\n 2: means on-going\n"));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("C0_H : %d, C1_H : %d, C2_H : %d\n C0_M : %d, C1_M : %d, C2_M : %d\n C0_L : %d, C1_L : %d, C2_L : %d\n C3_M : %d, C3_L : %d\n", 
                                                                     iBfPhaseOut.ucC0_H, iBfPhaseOut.ucC1_H, iBfPhaseOut.ucC2_H, 
                                                                     iBfPhaseOut.ucC0_M, iBfPhaseOut.ucC1_M, iBfPhaseOut.ucC2_M,
                                                                     iBfPhaseOut.ucC0_L, iBfPhaseOut.ucC1_L, iBfPhaseOut.ucC2_L, 
                                                                     iBfPhaseOut.ucC3_M, iBfPhaseOut.ucC3_L));
                break;
            }        
        
        }
        
        break;

    case IBF_PHASE_CAL_VERIFY: /* Show calibrated result only */
    case IBF_PHASE_CAL_VERIFY_INSTRUMENT:
        NdisCopyMemory(&iBfPhaseOut, pBuf, sizeof(IBF_PHASE_OUT));
        // Update calibrated status
        pAd->fgCalibrationFail |= ((ucStatus == 1) ? FALSE : TRUE);
        pAd->fgGroupIdPassFailStatus[ucGroup] = ((ucStatus == 1) ? FALSE : TRUE);

        MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group : %d\n", ucGroup));
        MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibration == 1? or Verification == 2? : %d\n", ucPhaseCalType));
        MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibrated result = %d\n", ucStatus));
        MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0 : Means failed\n 1: means pass\n 2: means on-going\n"));
        MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("C0_H : %d, C1_H : %d, C2_H : %d\n C0_M : %d, C1_M : %d, C2_M : %d\n C0_L : %d, C1_L : %d, C2_L : %d\n C3_M : %d, C3_L : %d\n", 
                                                          iBfPhaseOut.ucC0_H, iBfPhaseOut.ucC1_H, iBfPhaseOut.ucC2_H, 
                                                          iBfPhaseOut.ucC0_M, iBfPhaseOut.ucC1_M, iBfPhaseOut.ucC2_M,
                                                          iBfPhaseOut.ucC0_L, iBfPhaseOut.ucC1_L, iBfPhaseOut.ucC2_L, 
                                                          iBfPhaseOut.ucC3_M, iBfPhaseOut.ucC3_L));
        break;

    default:
        break;
    }

    return;
}

#endif /* TXBF_SUPPORT */
