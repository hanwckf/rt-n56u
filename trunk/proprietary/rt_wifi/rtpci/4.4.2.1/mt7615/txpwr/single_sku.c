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
	cmm_single_sku.c
*/
#ifdef COMPOS_WIN
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "Single_sku.tmh"
#endif
#elif defined (COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#endif

#include    "txpwr/single_sku.h"

#ifdef RF_LOCKDOWN
#include    "txpwr/SKUTable_1.h"
#include    "txpwr/SKUTable_2.h"
#include    "txpwr/SKUTable_3.h"
#include    "txpwr/SKUTable_4.h"
#include    "txpwr/SKUTable_5.h"
#include    "txpwr/SKUTable_6.h"
#include    "txpwr/SKUTable_7.h"
#include    "txpwr/SKUTable_8.h"
#include    "txpwr/SKUTable_9.h"
#include    "txpwr/SKUTable_10.h"
#include    "txpwr/SKUTable_11.h"
#include    "txpwr/SKUTable_12.h"
#include    "txpwr/SKUTable_13.h"
#include    "txpwr/SKUTable_14.h"
#include    "txpwr/SKUTable_15.h"
#include    "txpwr/SKUTable_16.h"
#include    "txpwr/SKUTable_17.h"
#include    "txpwr/SKUTable_18.h"
#include    "txpwr/SKUTable_19.h"
#include    "txpwr/SKUTable_20.h"
#include    "txpwr/BFBackoffTable_1.h"
#include    "txpwr/BFBackoffTable_2.h"
#include    "txpwr/BFBackoffTable_3.h"
#include    "txpwr/BFBackoffTable_4.h"
#include    "txpwr/BFBackoffTable_5.h"
#include    "txpwr/BFBackoffTable_6.h"
#include    "txpwr/BFBackoffTable_7.h"
#include    "txpwr/BFBackoffTable_8.h"
#include    "txpwr/BFBackoffTable_9.h"
#include    "txpwr/BFBackoffTable_10.h"
#include    "txpwr/BFBackoffTable_11.h"
#include    "txpwr/BFBackoffTable_12.h"
#include    "txpwr/BFBackoffTable_13.h"
#include    "txpwr/BFBackoffTable_14.h"
#include    "txpwr/BFBackoffTable_15.h"
#include    "txpwr/BFBackoffTable_16.h"
#include    "txpwr/BFBackoffTable_17.h"
#include    "txpwr/BFBackoffTable_18.h"
#include    "txpwr/BFBackoffTable_19.h"
#include    "txpwr/BFBackoffTable_20.h"
#endif /* RF_LOCKDOWN */

// TODO: shiang-usw, for MT76x0 series, currently cannot use this function!
#ifdef COMPOS_WIN

CHAR* os_str_pbrk(CHAR *str1,CHAR  *str2)
{
    const CHAR *x;
    for (; *str1; str1++)
        for (x = str2; *x; x++)
            if (*str1 == *x)
                return (CHAR *) str1;
    return NULL;
}

UINT32 os_str_spn(CHAR *str1,CHAR *str2)
{
	return strspn(str1,str2);
}

ULONG
simple_strtol(
    const RTMP_STRING *szProgID,
    INT EndPtr,
    INT Base
    )
{
    ULONG val=0;
    ANSI_STRING    AS;
    UNICODE_STRING    US;

    RtlInitAnsiString(&AS, szProgID);
    RtlAnsiStringToUnicodeString(&US, &AS, TRUE);

    RtlUnicodeStringToInteger(&US, 0, &val);
    RtlFreeUnicodeString(&US);

    return val;
}

LONG os_str_tol (const CHAR *str, CHAR **endptr, INT base)
{
	return simple_strtol(str,(INT)endptr,base);
}

CHAR* os_str_chr(CHAR *str, INT character)
{
	return strchr(str,character);
}

/**
 * rstrtok - Split a string into tokens
 * @s: The string to be searched
 * @ct: The characters to search for
 * * WARNING: strtok is deprecated, use strsep instead. However strsep is not compatible with old architecture.
 */
static RTMP_STRING *__rstrtok;
RTMP_STRING *rstrtok(RTMP_STRING *s,const RTMP_STRING *ct)
{
       
	RTMP_STRING *sbegin, *send;

	sbegin  = s ? s : __rstrtok;
	if (!sbegin)
	{
		return NULL;
	}

	sbegin += os_str_spn((CHAR *)sbegin,(CHAR *)ct);
	if (*sbegin == '\0')
	{
		__rstrtok = NULL;
		return( NULL );
	}

	send = os_str_pbrk( (CHAR *)sbegin, (CHAR *)ct);
	if (send && *send != '\0')
		*send++ = '\0';

	__rstrtok = send;

	return (sbegin);
}
#endif

#ifdef SINGLE_SKU_V2

INT	MtSingleSkuLoadParam(RTMP_ADAPTER *pAd)
{
    CHAR *buffer;
    CHAR *readline, *token;
#ifdef RF_LOCKDOWN
#else
    RTMP_OS_FD_EXT srcf;
    INT retval = 0;
#endif /* RF_LOCKDOWN */
    CHAR *ptr;
    INT index, i;
    CH_POWER *StartCh = NULL;
    UCHAR band = 0;
    UCHAR channel, *temp;
    CH_POWER *pwr = NULL;   
    CH_POWER *ch, *ch_temp;

    /* Link list Init */
    DlListInit(&pAd->SingleSkuPwrList);

    /* allocate memory for buffer SKU value */
    os_alloc_mem(pAd, (UCHAR **)&buffer, MAX_INI_BUFFER_SIZE);
    if (buffer == NULL)
        return FALSE;

#ifdef RF_LOCKDOWN
    pAd->CommonCfg.SKUTableIdx = pAd->EEPROMImage[SINGLE_SKU_TABLE_EFFUSE_ADDRESS] & BITS(0,6);

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: RF_LOCKDOWN Feature ON !!!\n", __FUNCTION__));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: SKU Table index = %d \n", __FUNCTION__, pAd->CommonCfg.SKUTableIdx));

    /* card information file exists so reading the card information */
    os_zero_mem(buffer, MAX_INI_BUFFER_SIZE);

    switch (pAd->CommonCfg.SKUTableIdx)
    {
        case SKUTABLE_1:
            os_move_mem(buffer, SKUvalue_1, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_1;
            break;
        case SKUTABLE_2:
            os_move_mem(buffer, SKUvalue_2, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_2;
            break;
        case SKUTABLE_3:
            os_move_mem(buffer, SKUvalue_3, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_3;
            break;
        case SKUTABLE_4:
            os_move_mem(buffer, SKUvalue_4, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_4;
            break;
        case SKUTABLE_5:
            os_move_mem(buffer, SKUvalue_5, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_5;
            break;
        case SKUTABLE_6:
            os_move_mem(buffer, SKUvalue_6, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_6;
            break;
        case SKUTABLE_7:
            os_move_mem(buffer, SKUvalue_7, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_7;
            break;
        case SKUTABLE_8:
            os_move_mem(buffer, SKUvalue_8, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_8;
            break;
        case SKUTABLE_9:
            os_move_mem(buffer, SKUvalue_9, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_9;
            break;
        case SKUTABLE_10:
            os_move_mem(buffer, SKUvalue_10, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_10;
            break;
        case SKUTABLE_11:
            os_move_mem(buffer, SKUvalue_11, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_11;
            break;
        case SKUTABLE_12:
            os_move_mem(buffer, SKUvalue_12, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_12;
            break;
        case SKUTABLE_13:
            os_move_mem(buffer, SKUvalue_13, MAX_INI_BUFFER_SIZE);
            break;
        case SKUTABLE_14:
            os_move_mem(buffer, SKUvalue_14, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_14;
            break;
        case SKUTABLE_15:
            os_move_mem(buffer, SKUvalue_15, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_15;
            break;
        case SKUTABLE_16:
            os_move_mem(buffer, SKUvalue_16, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_16;
            break;
        case SKUTABLE_17:
            os_move_mem(buffer, SKUvalue_17, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_17;
            break;
        case SKUTABLE_18:
            os_move_mem(buffer, SKUvalue_18, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_18;
            break;
        case SKUTABLE_19:
            os_move_mem(buffer, SKUvalue_19, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_19;
            break;
        case SKUTABLE_20:
            os_move_mem(buffer, SKUvalue_20, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_20;
            break;
        default:
            os_move_mem(buffer, SKUvalue_20, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_20;
            break; 
    }

#else
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: RF_LOCKDOWN Feature OFF !!!\n", __FUNCTION__));

    /* open card information file*/
    srcf = os_file_open(SINGLE_SKU_TABLE_FILE_NAME, O_RDONLY, 0);
    if (srcf.Status)
    {
        /* card information file does not exist */
        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
            ("--> Error opening %s\n", SINGLE_SKU_TABLE_FILE_NAME));
        goto  free_resource;
    }

    /* card information file exists so reading the card information */
    os_zero_mem(buffer, MAX_INI_BUFFER_SIZE);
    retval = os_file_read(srcf, buffer, MAX_INI_BUFFER_SIZE);

    if (retval < 0)
    {
        /* read fail */
        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("--> Read %s error %d\n", SINGLE_SKU_TABLE_FILE_NAME, -retval));
    }
    else
    {

#endif /* RF_LOCKDOWN */

#ifdef RF_LOCKDOWN
        for (readline = ptr = buffer, index = 0; (ptr = os_str_chr(readline, '\t')) != NULL; readline = ptr + 1, index++)
#else
        for (readline = ptr = buffer, index = 0; (ptr = os_str_chr(readline, '\n')) != NULL; readline = ptr + 1, index++)
#endif /* RF_LOCKDOWN */
        {
            *ptr = '\0';

#ifdef RF_LOCKDOWN
            if (readline[0] == '!')
                continue;
#else
            if (readline[0] == '#')
                continue;
#endif /* RF_LOCKDOWN */

            /* Band Info Parsing */
            if (!strncmp(readline, "Band: ", 6))
            {
                token = rstrtok(readline + 6 ," ");

                /* sanity check for non-Null pointer */
                if (!token)
                    continue;
                
                band = (UCHAR)os_str_tol(token, 0, 10);

                if (band == 2)
                {
                    band = 0;
                }
                else if (band == 5)
                {
                    band = 1;
                }
                    
                MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("band = %d\n", band));
            }

            /* Rate Info Parsing for each channel */
            if (!strncmp(readline, "Ch", 2) )
            {
                /* Dynamic allocate memory for parsing structure */
                os_alloc_mem(pAd, (UCHAR **)&pwr, sizeof(*pwr));

                /* set default value to 0 for parsing structure */
                os_zero_mem(pwr, sizeof(*pwr));

                token= rstrtok(readline + 2 ," ");

                /* sanity check for non-Null pointer */
                if (!token)
                {
                    /* free memory buffer before escape this loop */
                    os_free_mem(pwr);

                    /* escape this loop for Null pointer */
                    continue;
                }
                
                channel = (UCHAR)os_str_tol(token, 0, 10);
                pwr->StartChannel = channel;
                pwr->band = band;

                /* Rate Info Parsing (CCK) */
                if (band == 0) // if (pwr->StartChannel <= 14)
                {
                    for (i = 0; i < SINGLE_SKU_TABLE_CCK_LENGTH; i++)
                    {
                        token = rstrtok(NULL ," ");

                        /* sanity check for non-Null pointer */
                        if (!token)
                            break;

                        if (*token == ' ')
                            pwr->PwrCCK[i] = os_str_tol(token + 1, 0, 10) * 2;
                        else
                            pwr->PwrCCK[i] = os_str_tol(token, 0, 10) * 2;
                    }
                }

                /* Rate Info Parsing (OFDM) */
                for (i = 0; i < SINGLE_SKU_TABLE_OFDM_LENGTH; i++)
                {
                    token = rstrtok(NULL ," ");

                    /* sanity check for non-Null pointer */
                    if (!token)
                        break;

                    if (*token == ' ')
                        pwr->PwrOFDM[i] = os_str_tol(token + 1, 0, 10) * 2;
                    else
                        pwr->PwrOFDM[i] = os_str_tol(token, 0, 10) * 2;
                }

#ifdef DOT11_VHT_AC

                /* Rate Info Parsing (VHT20) */
                for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++)
                {
                    token = rstrtok(NULL ," ");

                    /* sanity check for non-Null pointer */
                    if (!token)
                        break;

                    if (*token == ' ')
                        pwr->PwrVHT20[i] = os_str_tol(token + 1, 0, 10) * 2;
                    else
                        pwr->PwrVHT20[i] = os_str_tol(token, 0, 10) * 2;
                }

                /* Rate Info Parsing (VHT40) */
                for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++)
                {
                    token = rstrtok(NULL ," ");

                    /* sanity check for non-Null pointer */
                    if (!token)
                        break;

                    if (*token == ' ')
                        pwr->PwrVHT40[i] = os_str_tol(token + 1, 0, 10) * 2;
                    else
                        pwr->PwrVHT40[i] = os_str_tol(token, 0, 10) * 2;
                }

                if (band == 1) // if (pwr->StartChannel > 14)
                {
                    /* Rate Info Parsing (VHT80) */
                    for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++)
                    {
                        token = rstrtok(NULL ," ");

                        /* sanity check for non-Null pointer */
                        if (!token)
                            break;

                        if (*token == ' ')
                            pwr->PwrVHT80[i] = os_str_tol(token + 1, 0, 10) * 2;
                        else
                            pwr->PwrVHT80[i] = os_str_tol(token, 0, 10) * 2;
                    }

                    /* Rate Info Parsing (VHT160) */
                    for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++)
                    {
                        token = rstrtok(NULL ," ");

                        /* sanity check for non-Null pointer */
                        if (!token)
                            break;

                        if (*token == ' ')
                            pwr->PwrVHT160[i] = os_str_tol(token + 1, 0, 10) * 2;
                        else
                            pwr->PwrVHT160[i] = os_str_tol(token, 0, 10) * 2;
                    }
                }
#endif /* DOT11_VHT_AC */

                /* Tx Stream offset Info Parsing */
                for (i = 0; i < SINGLE_SKU_TABLE_TX_OFFSET_NUM; i++)
                {
                    token = rstrtok(NULL ," ");

                    /* sanity check for non-Null pointer */
                    if (!token)
                        break;
                    
                    /* parsing order is 3T, 2T, 1T */
                    pwr->PwrTxStreamDelta[i] = os_str_tol(token, 0, 10) * 2;
                }

                /* Tx Spatial Stream offset Info Parsing */
                for (i = 0; i < SINGLE_SKU_TABLE_NSS_OFFSET_NUM; i++)
                {
                    token = rstrtok(NULL ," ");

                    /* sanity check for non-Null pointer */
                    if (!token)
                        break;

                    /* parsing order is 1SS, 2SS, 3SS, 4SS */
                    pwr->PwrTxNSSDelta[i] = os_str_tol(token, 0, 10) * 2;
                }

                /* Create New Data Structure to simpilify the SKU table (Represent together for channels with same rate Info, band Info, Tx Stream offset Info, Tx Spatial stream offset Info) */
                if (!StartCh)
                {
                    /* (Begining) assign new pointer head to SKU table contents for this channel */
                    StartCh = pwr;

                    /* add tail for Link list */
                    DlListAddTail(&pAd->SingleSkuPwrList, &pwr->List);
                }
                else
                {
                    BOOLEAN fgSameCont = TRUE;

                    if (band == 0) // if (pwr->StartChannel <= 14)
                    {
                        for (i = 0; i < SINGLE_SKU_TABLE_CCK_LENGTH; i++)
                        {
                            if (StartCh->PwrCCK[i] != pwr->PwrCCK[i])
                            {
                                fgSameCont = FALSE;
                                break;
                            }
                        }
                    }

                    if (fgSameCont)
                    {
                        for (i = 0; i < SINGLE_SKU_TABLE_OFDM_LENGTH; i++)
                        {
                            if (StartCh->PwrOFDM[i] != pwr->PwrOFDM[i])
                            {
                                fgSameCont = FALSE;
                                break;
                            }
                        }
                    }

                    if (fgSameCont)
                    {
                        for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++)
                        {
                            if (StartCh->PwrVHT20[i] != pwr->PwrVHT20[i])
                            {
                                fgSameCont = FALSE;
                                break;
                            }
                        }
                    }

                    if (fgSameCont)
                    {
                        for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++)
                        {
                            if (StartCh->PwrVHT40[i] != pwr->PwrVHT40[i])
                            {
                                fgSameCont = FALSE;
                                break;
                            }
                        }
                    }

                    if (fgSameCont)
                    {
                        for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++)
                        {
                            if (StartCh->PwrVHT80[i] != pwr->PwrVHT80[i])
                            {
                                fgSameCont = FALSE;
                                break;
                            }
                        }
                    }

                    if (fgSameCont)
                    {
                        for (i= 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++)
                        {
                            if (StartCh->PwrVHT160[i] != pwr->PwrVHT160[i])
                            {
                                fgSameCont = FALSE;
                                break;
                            }
                        }
                    }

                    if (fgSameCont)
                    {
                        for (i = 0; i < SINGLE_SKU_TABLE_TX_OFFSET_NUM; i++)
                        {
                            if (StartCh->PwrTxStreamDelta[i] != pwr->PwrTxStreamDelta[i])
                            {
                                fgSameCont = FALSE;
                                break;
                            }
                        }
                    }

                    if (fgSameCont)
                    {
                        for (i = 0; i < SINGLE_SKU_TABLE_NSS_OFFSET_NUM; i++)
                        {
                            if (StartCh->PwrTxNSSDelta[i] != pwr->PwrTxNSSDelta[i])
                            {
                                fgSameCont = FALSE;
                                break;
                            }
                        }
                    }

                    if (fgSameCont)
                    {
                        if (StartCh->band != pwr->band)
                        {
                            fgSameCont = FALSE;
                        }
                    }

                    /* check similarity of SKU table content for different channel */
                    if (fgSameCont)
                    {
                        os_free_mem(pwr);
                    }
                    else
                    {
                        /* Assign new pointer head to SKU table contents for this channel */
                        StartCh = pwr;

                        /* add tail for Link list */
                        DlListAddTail(&pAd->SingleSkuPwrList, &StartCh->List);
                    }
                }

                /* Increment total channel counts for channels with same SKU table contents */
                StartCh->num ++;

                /* allocate memory for channel list with same SKU table contents */
                os_alloc_mem(pAd, (PUCHAR *)&temp, StartCh->num);

                /* backup non-empty channel list to temp buffer */
                if (NULL != StartCh->Channel)
                {
                    /* copy channel list to temp buffer */
                    os_move_mem(temp, StartCh->Channel, StartCh->num - 1);

                    /* free memory for channel list used before assign pointer of temp memory buffer */
                    os_free_mem(StartCh->Channel);
                }

                /* assign pointer of temp memory buffer */
                StartCh->Channel = temp;

                /* update latest channel number to channel list */
                StartCh->Channel[StartCh->num - 1] = channel;
            }
        }
#ifdef RF_LOCKDOWN
#else
    }
#endif /* RF_LOCKDOWN */

    MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("SKU table index: %d \n", pAd->CommonCfg.SKUTableIdx));

    DlListForEachSafe(ch, ch_temp, &pAd->SingleSkuPwrList, CH_POWER, List)
    {
        int i;
        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("start ch = %d, ch->num = %d\n", ch->StartChannel, ch->num));

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Band: %d \n", ch->band));

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Channel: "));

        for ( i = 0 ; i < ch->num ; i++ )
        {
            MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%d ", ch->Channel[i]));
        }
        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("\n"));

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("CCK: "));

        for ( i= 0 ; i < SINGLE_SKU_TABLE_CCK_LENGTH ; i++ )
        {
            MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%d ", ch->PwrCCK[i]));
        }
        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("\n"));

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("OFDM: "));

        for ( i= 0 ; i < SINGLE_SKU_TABLE_OFDM_LENGTH ; i++ )
        {
            MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%d ", ch->PwrOFDM[i]));
        }
        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("\n"));

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("VHT20: "));

        for ( i= 0 ; i < SINGLE_SKU_TABLE_VHT_LENGTH ; i++ )
        {
            MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%d ", ch->PwrVHT20[i]));
        }
        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("\n"));

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("VHT40: "));

        for ( i= 0 ; i < SINGLE_SKU_TABLE_VHT_LENGTH ; i++ )
        {
            MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%d ", ch->PwrVHT40[i]));
        }
        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("\n"));

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("VHT80: "));

        for ( i= 0 ; i < SINGLE_SKU_TABLE_VHT_LENGTH ; i++ )
        {
            MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%d ", ch->PwrVHT80[i]));
        }
        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("\n"));

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("VHT160: "));

        for ( i= 0 ; i < SINGLE_SKU_TABLE_VHT_LENGTH ; i++ )
        {
            MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%d ", ch->PwrVHT160[i]));
        }
        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("\n"));

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("PwrTxStreamDelta: "));

        for ( i= 0 ; i < SINGLE_SKU_TABLE_TX_OFFSET_NUM ; i++ )
        {
            MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%d ", ch->PwrTxStreamDelta[i]));
        }      
        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("\n"));

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("PwrTxNSSDelta: "));
        for ( i= 0 ; i < SINGLE_SKU_TABLE_NSS_OFFSET_NUM ; i++ )
        {
            MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%d ", ch->PwrTxNSSDelta[i]));
        }
        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("\n"));

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("-----------------------------------------------------------------"));
    }
    
#ifdef RF_LOCKDOWN
#else   
    /* close file*/
    retval = os_file_close(srcf);

free_resource:
#endif /* RF_LOCKDOWN */

    os_free_mem(buffer);

    return TRUE;
}



VOID MtSingleSkuUnloadParam(RTMP_ADAPTER *pAd)
{
	CH_POWER *ch, *ch_temp;
	DlListForEachSafe(ch, ch_temp, &pAd->SingleSkuPwrList, CH_POWER, List)
	{
		DlListDel(&ch->List);

#if defined(MT7601) || defined(MT7603) || defined(MT7615)
                if (IS_MT7601(pAd) || IS_MT7603(pAd) || IS_MT7615(pAd))
                {
                    os_free_mem(ch->Channel);
                }
#endif /* MT7601 || MT7603 || MT7615 */

		os_free_mem(ch);
	}
}

CHAR
SKUTxPwrOffsetGet(RTMP_ADAPTER *pAd, UINT8 ucBandIdx, UINT8 ucBW, UINT8 ucPhymode ,UINT8 ucMCS, UINT_8 ucNss, BOOLEAN fgSE)
{
    CHAR        cPowerOffset = 0;
    UINT8       ucRateOffset = 0;
    UINT8       BW_OFFSET[4] = {VHT20_OFFSET, VHT40_OFFSET, VHT80_OFFSET, VHT160C_OFFSET};   
#ifdef CONFIG_ATE
    ATE_CTRL    *ATECtrl = &(pAd->ATECtrl);
#ifdef DBDC_MODE
    BAND_INFO   *Info = &(ATECtrl->band_ext[0]);
#endif /* DBDC_MODE */
#endif
    UINT8       ucNSS = 0;

    /* Compute MCS rate and Nss for HT mode */
    if ((ucPhymode == MODE_HTMIX) || (ucPhymode == MODE_HTGREENFIELD))
    {
        ucNss = (ucMCS >> 3) + 1;
        ucMCS &= 0x7;
    }

    switch (ucPhymode)
    {
        case MODE_CCK:
            ucRateOffset = SKU_CCK_OFFSET;
            switch (ucMCS)
            {
                case MCS_0:
                case MCS_1:
                    ucRateOffset = SKU_CCK_RATE_M01;
                    break;
                case MCS_2:
                case MCS_3:
                    ucRateOffset = SKU_CCK_RATE_M23;
                    break;
                default:
                    break;
            }
            break;   

        case MODE_OFDM:
            ucRateOffset = SKU_OFDM_OFFSET;
            switch (ucMCS)
            {
                case MCS_0:
                case MCS_1:
                    ucRateOffset = SKU_OFDM_RATE_M01;
                    break;
                case MCS_2:
                case MCS_3:
                    ucRateOffset = SKU_OFDM_RATE_M23;
                    break;
                case MCS_4:
                case MCS_5:
                    ucRateOffset = SKU_OFDM_RATE_M45;
                    break;
                case MCS_6:
                    ucRateOffset = SKU_OFDM_RATE_M6;
                    break;
                case MCS_7:
                    ucRateOffset = SKU_OFDM_RATE_M7;
                    break;
                default:
                    break;
            }
            break;     

        case MODE_HTMIX:
        case MODE_HTGREENFIELD:
            ucRateOffset = SKU_HT_OFFSET + BW_OFFSET[ucBW];
            switch (ucMCS)
            {
                case MCS_0:
                    ucRateOffset += SKU_HT_RATE_M0;
                    break;
                case MCS_1:
                case MCS_2:
                    ucRateOffset += SKU_HT_RATE_M12;
                    break;
                case MCS_3:
                case MCS_4:
                    ucRateOffset += SKU_HT_RATE_M34;
                    break;
                case MCS_5:
                    ucRateOffset += SKU_HT_RATE_M5;
                    break;
                case MCS_6:
                    ucRateOffset += SKU_HT_RATE_M6;
                    break;
                case MCS_7:
                    ucRateOffset += SKU_HT_RATE_M7;
                    break;
                default:
                    break;
            }   
            break;   

        case MODE_VHT:
            ucRateOffset = SKU_VHT_OFFSET + BW_OFFSET[ucBW];
            switch (ucMCS)
            {
                case MCS_0:
                    ucRateOffset += SKU_VHT_RATE_M0;
                    break;
                case MCS_1:
                case MCS_2:
                    ucRateOffset += SKU_VHT_RATE_M12;
                    break;
                case MCS_3:
                case MCS_4:
                    ucRateOffset += SKU_VHT_RATE_M34;
                    break;    
                case MCS_5:
                case MCS_6:
                    ucRateOffset += SKU_VHT_RATE_M56;
                    break; 
                case MCS_7:
                    ucRateOffset += SKU_VHT_RATE_M7;
                    break; 
                case MCS_8:
                    ucRateOffset += SKU_VHT_RATE_M8;
                    break;
                case MCS_9:
                    ucRateOffset += SKU_VHT_RATE_M9;
                    break;
                default:
                    break;
            }
            break;
    }

    /* Update Nss */
    switch (ucNss)
    {
        case 1:
            ucNSS = ATE_1_TX_STREAM;
            break;
        case 2:
            ucNSS = ATE_2_TX_STREAM;
            break;
        case 3:
            ucNSS = ATE_3_TX_STREAM;
            break;
        case 4:
            ucNSS = ATE_4_TX_STREAM;
            break;
        default:
            break;
    }

    /* Update Power offset by look up Tx Power Compensation Table */
    if (fgSE == TRUE)
        cPowerOffset = pAd->CommonCfg.cTxPowerCompBackup[ucBandIdx][ucRateOffset][ucNSS];
    else
        cPowerOffset = pAd->CommonCfg.cTxPowerCompBackup[ucBandIdx][ucRateOffset][ATE_4_TX_STREAM];

    /* Debug log for SKU Power offset to compensate */
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (KBLU "%s: ucBW: %d, ucPhymode: %d, ucMCS: %d, ucNss: %d, fgSPE: %d !!!\n" KNRM, __FUNCTION__, ucBW, ucPhymode, ucMCS, ucNss, fgSE));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (KBLU "%s: cPowerOffset: 0x%x (%d) !!!\n" KNRM, __FUNCTION__, cPowerOffset, cPowerOffset));

#ifdef CONFIG_ATE
    /* Check if Single SKU is disabled */
    if (BAND0 == ucBandIdx)
    {
        if (ATECtrl->fgTxPowerSKUEn == FALSE)
        {
            cPowerOffset = 0;
        }
    }
#ifdef DBDC_MODE    
    else if (BAND1 == ucBandIdx)
    {
        if(Info && (Info->fgTxPowerSKUEn == FALSE))
        {
            cPowerOffset = 0;
        }
    }
#endif /* DBDC_MODE */
#endif

    return cPowerOffset;
}

#ifdef TXBF_SUPPORT
#ifdef MT_MAC

INT	MtBfBackOffLoadTable(RTMP_ADAPTER *pAd)
{
    CHAR *buffer;
    CHAR *readline, *token;
#ifdef RF_LOCKDOWN
#else
        RTMP_OS_FD_EXT srcf;
        INT retval = 0;
#endif /* RF_LOCKDOWN */
    CHAR *ptr;
    INT index, i;
    BFback_POWER *StartCh = NULL;
    UCHAR band = 0;
    UCHAR channel, *temp;
    BFback_POWER *pwr = NULL;   
    BFback_POWER *ch, *ch_temp;

    DlListInit(&pAd->BFBackoffList);

    /* init*/
    os_alloc_mem(pAd, (UCHAR **)&buffer, MAX_INI_BUFFER_SIZE);
    if (buffer == NULL)
        return FALSE;

#ifdef RF_LOCKDOWN
    pAd->CommonCfg.SKUTableIdx = pAd->EEPROMImage[SINGLE_SKU_TABLE_EFFUSE_ADDRESS] & BITS(0,6);

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: RF_LOCKDOWN Feature ON !!!\n", __FUNCTION__));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: BFBackoff Table index = %d \n", __FUNCTION__, pAd->CommonCfg.SKUTableIdx));

    /* card information file exists so reading the card information */
    os_zero_mem(buffer, MAX_INI_BUFFER_SIZE);

    switch (pAd->CommonCfg.SKUTableIdx)
    {
        case SKUTABLE_1:
            os_move_mem(buffer, BFBackoffvalue_1, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_1;
            break;
        case SKUTABLE_2:
            os_move_mem(buffer, BFBackoffvalue_2, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_2;
            break;
        case SKUTABLE_3:
            os_move_mem(buffer, BFBackoffvalue_3, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_3;
            break;
        case SKUTABLE_4:
            os_move_mem(buffer, BFBackoffvalue_4, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_4;
            break;
        case SKUTABLE_5:
            os_move_mem(buffer, BFBackoffvalue_5, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_5;
            break;
        case SKUTABLE_6:
            os_move_mem(buffer, BFBackoffvalue_6, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_6;
            break;
        case SKUTABLE_7:
            os_move_mem(buffer, BFBackoffvalue_7, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_7;
            break;
        case SKUTABLE_8:
            os_move_mem(buffer, BFBackoffvalue_8, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_8;
            break;
        case SKUTABLE_9:
            os_move_mem(buffer, BFBackoffvalue_9, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_9;
            break;
        case SKUTABLE_10:
            os_move_mem(buffer, BFBackoffvalue_10, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_10;
            break;
        case SKUTABLE_11:
            os_move_mem(buffer, BFBackoffvalue_11, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_11;
            break;
        case SKUTABLE_12:
            os_move_mem(buffer, BFBackoffvalue_12, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_12;
            break;
        case SKUTABLE_13:
            os_move_mem(buffer, BFBackoffvalue_13, MAX_INI_BUFFER_SIZE);
            break;
        case SKUTABLE_14:
            os_move_mem(buffer, BFBackoffvalue_14, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_14;
            break;
        case SKUTABLE_15:
            os_move_mem(buffer, BFBackoffvalue_15, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_15;
            break;
        case SKUTABLE_16:
            os_move_mem(buffer, BFBackoffvalue_16, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_16;
            break;
        case SKUTABLE_17:
            os_move_mem(buffer, BFBackoffvalue_17, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_17;
            break;
        case SKUTABLE_18:
            os_move_mem(buffer, BFBackoffvalue_18, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_18;
            break;
        case SKUTABLE_19:
            os_move_mem(buffer, BFBackoffvalue_19, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_19;
            break;
        case SKUTABLE_20:
            os_move_mem(buffer, BFBackoffvalue_20, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_20;
            break;
        default:
            os_move_mem(buffer, SKUvalue_20, MAX_INI_BUFFER_SIZE);
            //buffer = SKUvalue_20;
            break; 
    }

#else
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: RF_LOCKDOWN Feature OFF !!!\n", __FUNCTION__));

    /* open card information file*/
    srcf = os_file_open(BF_SKU_TABLE_FILE_NAME, O_RDONLY, 0);
    if (srcf.Status)
    {
        /* card information file does not exist */
        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
            ("--> Error opening %s\n", BF_SKU_TABLE_FILE_NAME));
        goto  free_resource;
    }

    /* card information file exists so reading the card information */
    os_zero_mem(buffer, MAX_INI_BUFFER_SIZE);
    retval = os_file_read(srcf, buffer, MAX_INI_BUFFER_SIZE);
    if (retval < 0)
    {
        /* read fail */
        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("--> Read %s error %d\n", BF_SKU_TABLE_FILE_NAME, -retval));
    }
    else
    {
#endif /* RF_LOCKDOWN */

#ifdef RF_LOCKDOWN
        for (readline = ptr = buffer, index=0; (ptr = os_str_chr(readline, '\t')) != NULL; readline = ptr + 1, index++)
#else
        for (readline = ptr = buffer, index=0; (ptr = os_str_chr(readline, '\n')) != NULL; readline = ptr + 1, index++)
#endif /* RF_LOCKDOWN */
        {
            *ptr = '\0';

#ifdef RF_LOCKDOWN
            if (readline[0] == '!')
                continue;
#else
            if (readline[0] == '#')
                continue;
#endif /* RF_LOCKDOWN */

            /* Band Info Parsing */
            if (!strncmp(readline, "Band: ", 6))
            {
                token= rstrtok(readline +6 ," ");

                /* sanity check for non-Null pointer */
                if (!token)
                    continue;

                band = (UCHAR)os_str_tol(token, 0, 10);

                if (band == 2)
                {
                    band = 0;
                }
                else if (band == 5)
                {
                    band = 1;
                }
                    
                MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("band = %d\n", band));
            }

            /* BF Backoff Info Parsing for each channel */
            if (!strncmp(readline, "Ch", 2))
            {
                /* Dynamic allocate memory for parsing structure */
                os_alloc_mem(pAd, (UCHAR **)&pwr, sizeof(*pwr));

                /* set default value to 0 for parsing structure */
                os_zero_mem(pwr, sizeof(*pwr));

                token= rstrtok(readline + 2 ," ");

                /* sanity check for non-Null pointer */
                if (!token)
                {
                    /* free memory buffer before escape this loop */
                    os_free_mem(pwr);

                    /* escape this loop for Null pointer */
                    continue;
                }
                
                channel = (UCHAR)os_str_tol(token, 0, 10);
                pwr->StartChannel = channel;
                pwr->band = band;

                /* BF Backoff Info Parsing */
                for (i = 0; i < 3; i++)
                {
                    token = rstrtok(NULL ," ");

                    /* sanity check for non-Null pointer */
                    if (!token)
                        break;

                    if (*token == ' ')
                        pwr->PwrMax[i] = os_str_tol(token + 1, 0, 10) * 2;
                    else
                        pwr->PwrMax[i] = os_str_tol(token, 0, 10) * 2;
                }

                /* Create New Data Structure to simpilify the SKU table (Represent together for channels with same BF Backoff Info) */
                if (!StartCh)
                {
                    /* (Begining) assign new pointer head to SKU table contents for this channel */
                    StartCh = pwr;

                    /* add tail for Link list */
                    DlListAddTail(&pAd->BFBackoffList, &pwr->List);
                }
                else
                {
                    BOOLEAN fgSameCont = TRUE;

                    if (fgSameCont)
                    {
                        for (i = 0; i < 3; i++)
                        {
                            if (StartCh->PwrMax[i] != pwr->PwrMax[i])
                            {
                                fgSameCont = FALSE;
                                break;
                            }
                        }
                    }

                    if (fgSameCont)
                    {
                        if (StartCh->band != pwr->band)
                        {
                            fgSameCont = FALSE;
                        }
                    }

                    /* check similarity of SKU table content for different channel */
                    if (fgSameCont)
                    {
                        os_free_mem(pwr);
                    }
                    else
                    {
                        /* Assign new pointer head to SKU table contents for this channel */
                        StartCh = pwr;

                        /* add tail for Link list */
                        DlListAddTail(&pAd->BFBackoffList, &StartCh->List);
                    }

                }

                /* Increment total channel counts for channels with same SKU table contents */
                StartCh->num ++;

                /* allocate memory for channel list with same SKU table contents */
                os_alloc_mem(pAd, (PUCHAR *)&temp, StartCh->num);

                /* backup non-empty channel list to temp buffer */
                if (StartCh->Channel != NULL)
                {
                    /* copy channel list to temp buffer */
                    os_move_mem(temp, StartCh->Channel, StartCh->num - 1);

                    /* free memory for channel list used before assign pointer of temp memory buffer */
                    os_free_mem( StartCh->Channel);
                }

                /* assign pointer of temp memory buffer */
                StartCh->Channel = temp;

                /* update latest channel number to channel list */
                StartCh->Channel[StartCh->num - 1] = channel;
            }
        }
#ifdef RF_LOCKDOWN
#else
    }
#endif /* RF_LOCKDOWN */

    DlListForEachSafe(ch, ch_temp, &pAd->BFBackoffList, BFback_POWER, List)
    {
        int i;
        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("start ch = %d, ch->num = %d\n", ch->StartChannel, ch->num));

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Band: %d \n", ch->band));

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Channel: "));
        for ( i = 0 ; i < ch->num ; i++ )
            MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%d ", ch->Channel[i]));
        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("\n"));

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Max Power: "));
        for ( i= 0 ; i < 3 ; i++ )
        {
            MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%d ", ch->PwrMax[i]));
        }
        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("\n"));

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,("-----------------------------------------------------------------"));
    }
    
#ifdef RF_LOCKDOWN
#else   
    /* close file*/
    retval = os_file_close(srcf);

free_resource:
#endif /* RF_LOCKDOWN */

    os_free_mem( buffer);
    return TRUE;
}


VOID MtBfBackOffUnloadTable(RTMP_ADAPTER *pAd)
{
	BFback_POWER *ch, *ch_temp;
	DlListForEachSafe(ch, ch_temp, &pAd->BFBackoffList, BFback_POWER, List)
	{
		DlListDel(&ch->List);

#if defined(MT7601) || defined(MT7603) || defined(MT7615)
		if (IS_MT7601(pAd) || IS_MT7603(pAd) || IS_MT7615(pAd))
        {
			os_free_mem(ch->Channel);
		}
#endif /* MT7601 || MT7603 || MT7615 */

		os_free_mem(ch);
	}
}

#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */

#endif /* SINGLE_SKU_V2 */
