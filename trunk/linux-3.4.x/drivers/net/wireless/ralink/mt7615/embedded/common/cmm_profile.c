/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	cmm_profile.c

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#include "rt_config.h"

#ifdef MULTI_PROFILE
#ifdef DSCP_QOS_MAP_SUPPORT
INT multi_profile_get_bss_num(struct _RTMP_ADAPTER *ad, UINT8 profile_num);
#endif
#endif

#define ETH_MAC_ADDR_STR_LEN 17  /* in format of xx:xx:xx:xx:xx:xx*/

/* We assume the s1 is a sting, s2 is a memory space with 6 bytes. and content of s1 will be changed.*/
BOOLEAN rtstrmactohex(RTMP_STRING *s1, RTMP_STRING *s2)
{
	int i = 0;
	RTMP_STRING *ptokS = s1, *ptokE = s1;

	if (strlen(s1) != ETH_MAC_ADDR_STR_LEN)
		return FALSE;

	while((*ptokS) != '\0')
	{
		if((ptokE = strchr(ptokS, ':')) != NULL)
			*ptokE++ = '\0';
		if ((strlen(ptokS) != 2) || (!isxdigit(*ptokS)) || (!isxdigit(*(ptokS+1))))
			break; /* fail*/
		AtoH(ptokS, (PUCHAR)&s2[i++], 1);
		ptokS = ptokE;
		if (ptokS == NULL)
			break;
		if (i == 6)
			break; /* parsing finished*/
	}

	return ( i == 6 ? TRUE : FALSE);

}


#define ASC_LOWER(_x)	((((_x) >= 0x41) && ((_x) <= 0x5a)) ? (_x) + 0x20 : (_x))
/* we assume the s1 and s2 both are strings.*/
BOOLEAN rtstrcasecmp(RTMP_STRING *s1, RTMP_STRING *s2)
{
	RTMP_STRING *p1 = s1, *p2 = s2;
	CHAR c1, c2;

	if (strlen(s1) != strlen(s2))
		return FALSE;

	while(*p1 != '\0')
	{
		c1 = ASC_LOWER(*p1);
		c2 = ASC_LOWER(*p2);
		if(c1 != c2)
			return FALSE;
		p1++;
		p2++;
	}

	return TRUE;
}


/* we assume the s1 (buffer) and s2 (key) both are strings.*/
RTMP_STRING *rtstrstruncasecmp(RTMP_STRING *s1, RTMP_STRING *s2)
{
	INT l1, l2, i;
	char temp1, temp2;

	l2 = strlen(s2);
	if (!l2)
		return (char *) s1;

	l1 = strlen(s1);

	while (l1 >= l2)
	{
		l1--;

		for(i=0; i<l2; i++)
		{
			temp1 = *(s1+i);
			temp2 = *(s2+i);

			if (('a' <= temp1) && (temp1 <= 'z'))
				temp1 = 'A'+(temp1-'a');
			if (('a' <= temp2) && (temp2 <= 'z'))
				temp2 = 'A'+(temp2-'a');

			if (temp1 != temp2)
				break;
		}

		if (i == l2)
			return (char *) s1;

		s1++;
	}

	return NULL; /* not found*/
}


 /**
  * strstr - Find the first substring in a %NUL terminated string
  * @s1: The string to be searched
  * @s2: The string to search for
  */
RTMP_STRING *rtstrstr(const RTMP_STRING *s1,const RTMP_STRING *s2)
{
	INT l1, l2;

	l2 = strlen(s2);
	if (!l2)
		return (RTMP_STRING *)s1;

	l1 = strlen(s1);

	while (l1 >= l2)
	{
		l1--;
		if (!memcmp(s1,s2,l2))
			return (RTMP_STRING *)s1;
		s1++;
	}

	return NULL;
}

/**
 * rstrtok - Split a string into tokens
 * @s: The string to be searched
 * @ct: The characters to search for
 * * WARNING: strtok is deprecated, use strsep instead. However strsep is not compatible with old architecture.
 */
RTMP_STRING *__rstrtok;
RTMP_STRING *rstrtok(RTMP_STRING *s,const RTMP_STRING *ct)
{
	RTMP_STRING *sbegin, *send;

	sbegin  = s ? s : __rstrtok;
	if (!sbegin)
	{
		return NULL;
	}

	sbegin += strspn(sbegin,ct);
	if (*sbegin == '\0')
	{
		__rstrtok = NULL;
		return( NULL );
	}

	send = strpbrk( sbegin, ct);
	if (send && *send != '\0')
		*send++ = '\0';

	__rstrtok = send;

	return (sbegin);
}

/**
 * delimitcnt - return the count of a given delimiter in a given string.
 * @s: The string to be searched.
 * @ct: The delimiter to search for.
 * Notice : We suppose the delimiter is a single-char string(for example : ";").
 */
INT delimitcnt(RTMP_STRING *s,RTMP_STRING *ct)
{
	INT count = 0;
	/* point to the beginning of the line */
	RTMP_STRING *token = s;

	for ( ;; )
	{
		token = strpbrk(token, ct); /* search for delimiters */

        if ( token == NULL )
		{
			/* advanced to the terminating null character */
			break;
		}
		/* skip the delimiter */
	    ++token;

		/*
		 * Print the found text: use len with %.*s to specify field width.
		 */

		/* accumulate delimiter count */
	    ++count;
	}
    return count;
}

/*
  * converts the Internet host address from the standard numbers-and-dots notation
  * into binary data.
  * returns nonzero if the address is valid, zero if not.
  */
int rtinet_aton(const RTMP_STRING *cp, unsigned int *addr)
{
	unsigned int 	val;
	int         	base, n;
	RTMP_STRING c;
	unsigned int    parts[4] = {0};
	unsigned int    *pp = parts;

	for (;;)
    {
         /*
          * Collect number up to ``.''.
          * Values are specified as for C:
          *	0x=hex, 0=octal, other=decimal.
          */
         val = 0;
         base = 10;
         if (*cp == '0')
         {
             if (*++cp == 'x' || *cp == 'X')
                 base = 16, cp++;
             else
                 base = 8;
         }
         while ((c = *cp) != '\0')
         {
             if (isdigit((unsigned char) c))
             {
                 val = (val * base) + (c - '0');
                 cp++;
                 continue;
             }
             if (base == 16 && isxdigit((unsigned char) c))
             {
                 val = (val << 4) +
                     (c + 10 - (islower((unsigned char) c) ? 'a' : 'A'));
                 cp++;
                 continue;
             }
             break;
         }
         if (*cp == '.')
         {
             /*
              * Internet format: a.b.c.d a.b.c   (with c treated as 16-bits)
              * a.b     (with b treated as 24 bits)
              */
             if (pp >= parts + 3 || val > 0xff)
                 return 0;
             *pp++ = val, cp++;
         }
         else
             break;
     }

     /*
      * Check for trailing junk.
      */
     while (*cp)
         if (!isspace((unsigned char) *cp++))
             return 0;

     /*
      * Concoct the address according to the number of parts specified.
      */
     n = pp - parts + 1;
     switch (n)
     {

         case 1:         /* a -- 32 bits */
             break;

         case 2:         /* a.b -- 8.24 bits */
             if (val > 0xffffff)
                 return 0;
             val |= parts[0] << 24;
             break;

         case 3:         /* a.b.c -- 8.8.16 bits */
             if (val > 0xffff)
                 return 0;
             val |= (parts[0] << 24) | (parts[1] << 16);
             break;

         case 4:         /* a.b.c.d -- 8.8.8.8 bits */
             if (val > 0xff)
                 return 0;
             val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
             break;
     }

     *addr = OS_HTONL(val);
     return 1;

}



static VOID RTMPChannelCfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{
	UINT32 i;
	CHAR *macptr;
	struct wifi_dev *wdev;
	UCHAR Channel;

#ifdef CONFIG_AP_SUPPORT
        pAd->AutoChSelCtrl.AutoChannelFlag = 0;
#endif /* CONFIG_AP_SUPPORT */

	for (i = 0, macptr = rstrtok(Buffer,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	{
		Channel = simple_strtol(macptr, 0, 10);
	
#ifdef CONFIG_AP_SUPPORT	
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			if (i >= pAd->ApCfg.BssidNum)
				break;

			wdev = &pAd->ApCfg.MBSSID[i].wdev;
			wdev->channel = Channel;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS%d Channel=%d\n", i, wdev->channel));

			if (Channel == 0)
				pAd->AutoChSelCtrl.AutoChannelFlag |= (1<< i);
		}
#endif /* CONFIG_AP_SUPPORT */
	}

/*Can not assign default channel to wdev-> channel when channel = 0 */    
/*Just by reason of channel = 0 is one of the indicators of auto-channel selection */

}


static VOID RTMPWirelessModeCfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{
	UCHAR i;
	UCHAR cfg_mode, *macptr;
	struct wifi_dev *wdev = NULL;
#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
    UCHAR IdBss = 0;
#endif
#endif

	for (i = 0, macptr = rstrtok(Buffer,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	{
		cfg_mode = simple_strtol(macptr, 0, 10);
#ifdef CONFIG_AP_SUPPORT
		if (i >= pAd->ApCfg.BssidNum)
			break;

		wdev = &pAd->ApCfg.MBSSID[i].wdev;
		wdev->PhyMode = cfgmode_2_wmode(cfg_mode);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS%d PhyMode=%d\n", i, wdev->PhyMode));
#ifdef MBSS_SUPPORT
		if (i == 0)
		{
			/* for first time, update all phy mode is same as ra0 */
				for(IdBss=1; IdBss<pAd->ApCfg.BssidNum; IdBss++)
			{
					pAd->ApCfg.MBSSID[IdBss].wdev.PhyMode = pAd->ApCfg.MBSSID[0].wdev.PhyMode;
			}
		}else
		{
			RT_CfgSetMbssWirelessMode(pAd, macptr);
		}
#endif /* MBSS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

		if(i==0)
		{
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
			UCHAR idx;
			/* for first time, update all phy mode is same as ra0 */
			for(idx=0; idx<MAX_APCLI_NUM; idx++) {
				pAd->ApCfg.ApCliTab[idx].wdev.PhyMode = pAd->ApCfg.MBSSID[0].wdev.PhyMode;
			}
#endif /*APCLI_SUPPORT*/
#endif /* CONFIG_AP_SUPPORT */
			RT_CfgSetWirelessMode(pAd, macptr,wdev);
		}

	}
#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
	/*Check if any wdev not configure a wireless mode, apply MSSID value to it.*/
	for(i=0;i < pAd->ApCfg.BssidNum;i++)
	{
		wdev = &pAd->ApCfg.MBSSID[i].wdev;
		if(wdev->PhyMode==WMODE_INVALID)
		{
			wdev->PhyMode = pAd->ApCfg.MBSSID[0].wdev.PhyMode;
		}
	}
#endif/*MBSS_SUPPORT*/
#endif /*CONFIG_AP_SUPPORT*/
	if (wdev)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PhyMode=%d\n", wdev->PhyMode));
}


/*
    ========================================================================

    Routine Description:
        Find key section for Get key parameter.

    Arguments:
        buffer                      Pointer to the buffer to start find the key section
        section                     the key of the secion to be find

    Return Value:
        NULL                        Fail
        Others                      Success
    ========================================================================
*/
RTMP_STRING *RTMPFindSection(RTMP_STRING *buffer)
{
    RTMP_STRING temp_buf[32];
    RTMP_STRING *ptr;

    strcpy(temp_buf, "Default");

    if((ptr = rtstrstr(buffer, temp_buf)) != NULL)
            return (ptr+strlen("\n"));
        else
            return NULL;
}

/*
    ========================================================================

    Routine Description:
        Get key parameter.

    Arguments:
	key			Pointer to key string
	dest			Pointer to destination
	destsize		The datasize of the destination
	buffer		Pointer to the buffer to start find the key
	bTrimSpace	Set true if you want to strip the space character of the result pattern

    Return Value:
        TRUE                        Success
        FALSE                       Fail

    Note:
	This routine get the value with the matched key (case case-sensitive)
	For SSID and security key related parameters, we SHALL NOT trim the space(' ') character.
    ========================================================================
*/
INT RTMPGetKeyParameter(
    IN RTMP_STRING *key,
    OUT RTMP_STRING *dest,
    IN INT destsize,
    IN RTMP_STRING *buffer,
    IN BOOLEAN bTrimSpace)
{
	RTMP_STRING *pMemBuf, *temp_buf1 = NULL, *temp_buf2 = NULL;
	RTMP_STRING *start_ptr, *end_ptr;
	RTMP_STRING *ptr;
	RTMP_STRING *offset = NULL;
	INT  len, keyLen;


	keyLen = strlen(key);
	os_alloc_mem(NULL, (PUCHAR *)&pMemBuf, MAX_PARAM_BUFFER_SIZE  * 2);
	if (pMemBuf == NULL)
		return (FALSE);

	memset(pMemBuf, 0, MAX_PARAM_BUFFER_SIZE * 2);
	temp_buf1 = pMemBuf;
	temp_buf2 = (RTMP_STRING *)(pMemBuf + MAX_PARAM_BUFFER_SIZE);


	/*find section*/
	if((offset = RTMPFindSection(buffer)) == NULL)
	{
		os_free_mem( (PUCHAR)pMemBuf);
		return (FALSE);
	}

	strcpy(temp_buf1, "\n");
	strcat(temp_buf1, key);
	strcat(temp_buf1, "=");

	/*search key*/
	if((start_ptr=rtstrstr(offset, temp_buf1)) == NULL)
	{
		os_free_mem( (PUCHAR)pMemBuf);
		return (FALSE);
	}

	start_ptr += strlen("\n");
	if((end_ptr = rtstrstr(start_ptr, "\n"))==NULL)
		end_ptr = start_ptr+strlen(start_ptr);

	if (end_ptr<start_ptr)
	{
		os_free_mem( (PUCHAR)pMemBuf);
		return (FALSE);
	}

	NdisMoveMemory(temp_buf2, start_ptr, end_ptr-start_ptr);
	temp_buf2[end_ptr-start_ptr]='\0';

	if((start_ptr=rtstrstr(temp_buf2, "=")) == NULL)
	{
		os_free_mem( (PUCHAR)pMemBuf);
		return (FALSE);
	}
	ptr = (start_ptr +1);
	/*trim special characters, i.e.,  TAB or space*/
	while(*start_ptr != 0x00)
	{
		if( ((*ptr == ' ') && bTrimSpace) || (*ptr == '\t') )
			ptr++;
		else
			break;
	}
	len = strlen(start_ptr);

	memset(dest, 0x00, destsize);
	strncpy(dest, ptr, ((len >= destsize) ? destsize: len));

	os_free_mem( (PUCHAR)pMemBuf);

	return TRUE;
}

/*
    ========================================================================

    Routine Description:
        Add key parameter.

    Arguments:
	key			Pointer to key string
	value			Pointer to destination
	destsize		The datasize of the destination
	bTrimSpace	Set true if you want to strip the space character of the result pattern

    Return Value:
        TRUE                        Success
        FALSE                       Fail

    Note:
	This routine get the value with the matched key (case case-sensitive)
	For SSID and security key related parameters, we SHALL NOT trim the space(' ') character.
    ========================================================================
*/
INT RTMPAddKeyParameter(
    IN RTMP_STRING *key,
    IN CHAR *value,
    IN INT destsize,
    IN RTMP_STRING *buffer)
{
	UINT len = strlen(buffer);
	CHAR *ptr = buffer + len;
	sprintf(ptr,"%s=%s\n",key,value);
	return TRUE;
}

/*
    ========================================================================

    Routine Description:
        Set key parameter.

    Arguments:
	key			Pointer to key string
	value			Pointer to destination
	destsize		The datasize of the destination
	bTrimSpace	Set true if you want to strip the space character of the result pattern

    Return Value:
        TRUE                        Success
        FALSE                       Fail

    Note:
	This routine get the value with the matched key (case case-sensitive)
	For SSID and security key related parameters, we SHALL NOT trim the space(' ') character.
    ========================================================================
*/
INT RTMPSetKeyParameter(
    IN RTMP_STRING *key,
    OUT CHAR *value,
    IN INT destsize,
    IN RTMP_STRING *buffer,
    IN BOOLEAN bTrimSpace)
{
	RTMP_STRING buf[512] = "", *temp_buf1 = NULL;
	RTMP_STRING *start_ptr;
	RTMP_STRING *end_ptr;
	RTMP_STRING *offset = NULL;
	INT keyLen;
	INT start_len;
	INT end_len;
	INT len;

	keyLen = strlen(key);
	temp_buf1 = buf;

	/*find section*/
	if((offset = RTMPFindSection(buffer)) == NULL)
	{
		return (FALSE);
	}

	strcpy(temp_buf1, "\n");
	strcat(temp_buf1, key);
	strcat(temp_buf1, "=");

	/*search key*/
	if((start_ptr=rtstrstr(offset, temp_buf1)) == NULL)
	{
		/*can't searched, add directly*/
		RTMPAddKeyParameter(key,value,destsize,buffer);
		return (TRUE);
	}
	/*remove original*/
	start_ptr += strlen("\n");
	start_len = strlen(start_ptr);
	if((end_ptr = rtstrstr(start_ptr, "\n"))==NULL)
		end_ptr = start_ptr+start_len;

	if (end_ptr<start_ptr)
	{
		return (FALSE);
	}
	/*clear original setting*/
	end_ptr += strlen("\n");
	end_len = strlen(end_ptr);
	os_move_mem(start_ptr,end_ptr,end_len);
	start_ptr += end_len;
	len = start_len - end_len;
	os_zero_mem(start_ptr,len);
	/*fill new field & value*/
	RTMPAddKeyParameter(key,value,destsize,buffer);
	return TRUE;
}

/*
    ========================================================================

    Routine Description:
        Get multiple key parameter.

    Arguments:
        key                         Pointer to key string
        dest                        Pointer to destination
        destsize                    The datasize of the destination
        buffer                      Pointer to the buffer to start find the key

    Return Value:
        TRUE                        Success
        FALSE                       Fail

    Note:
        This routine get the value with the matched key (case case-sensitive)
    ========================================================================
*/
INT RTMPGetKeyParameterWithOffset(
    IN  RTMP_STRING *key,
    OUT RTMP_STRING *dest,
    OUT	USHORT	*end_offset,
    IN  INT     destsize,
    IN  RTMP_STRING *buffer,
    IN	BOOLEAN	bTrimSpace)
{
    RTMP_STRING *temp_buf1 = NULL;
    RTMP_STRING *temp_buf2 = NULL;
    RTMP_STRING *start_ptr;
    RTMP_STRING *end_ptr;
    RTMP_STRING *ptr;
    RTMP_STRING *offset = 0;
    INT  len;

	if (*end_offset >= MAX_INI_BUFFER_SIZE)
		return (FALSE);

	os_alloc_mem(NULL, (PUCHAR *)&temp_buf1, MAX_PARAM_BUFFER_SIZE);

	if(temp_buf1 == NULL)
        return (FALSE);

	os_alloc_mem(NULL, (PUCHAR *)&temp_buf2, MAX_PARAM_BUFFER_SIZE);
	if(temp_buf2 == NULL)
	{
		os_free_mem( (PUCHAR)temp_buf1);
        return (FALSE);
	}

    /*find section		*/
	if(*end_offset == 0)
    {
		if ((offset = RTMPFindSection(buffer)) == NULL)
		{
			os_free_mem( (PUCHAR)temp_buf1);
	    	os_free_mem( (PUCHAR)temp_buf2);
    	    return (FALSE);
		}
    }
	else
		offset = buffer + (*end_offset);

    strcpy(temp_buf1, "\n");
    strcat(temp_buf1, key);
    strcat(temp_buf1, "=");

    /*search key*/
    if((start_ptr=rtstrstr(offset, temp_buf1))==NULL)
    {
		os_free_mem( (PUCHAR)temp_buf1);
    	os_free_mem( (PUCHAR)temp_buf2);
        return (FALSE);
    }

    start_ptr+=strlen("\n");
    if((end_ptr=rtstrstr(start_ptr, "\n"))==NULL)
       end_ptr=start_ptr+strlen(start_ptr);

    if (end_ptr<start_ptr)
    {
		os_free_mem( (PUCHAR)temp_buf1);
    	os_free_mem( (PUCHAR)temp_buf2);
        return (FALSE);
    }

	*end_offset = end_ptr - buffer;

    NdisMoveMemory(temp_buf2, start_ptr, end_ptr-start_ptr);
    temp_buf2[end_ptr-start_ptr]='\0';
    len = strlen(temp_buf2);
    strcpy(temp_buf1, temp_buf2);
    if((start_ptr=rtstrstr(temp_buf1, "=")) == NULL)
    {
		os_free_mem( (PUCHAR)temp_buf1);
    	os_free_mem( (PUCHAR)temp_buf2);
        return (FALSE);
    }

    strcpy(temp_buf2, start_ptr+1);
    ptr = temp_buf2;
    /*trim space or tab*/
    while(*ptr != 0x00)
    {
        if((bTrimSpace && (*ptr == ' ')) || (*ptr == '\t') )
            ptr++;
        else
           break;
    }

    len = strlen(ptr);
    memset(dest, 0x00, destsize);
    strncpy(dest, ptr, len >= destsize ?  destsize: len);

	os_free_mem( (PUCHAR)temp_buf1);
    os_free_mem( (PUCHAR)temp_buf2);
    return TRUE;
}


#ifdef CONFIG_AP_SUPPORT

#ifdef APCLI_SUPPORT
static void rtmp_read_ap_client_from_file(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *tmpbuf,
	IN RTMP_STRING *buffer)
{
	RTMP_STRING *macptr = NULL;
	INT			i=0, j=0 ;
	UCHAR		macAddress[MAC_ADDR_LEN];
	PAPCLI_STRUCT   pApCliEntry = NULL;
   	struct wifi_dev *wdev;

#ifdef DOT11W_PMF_SUPPORT
	for (i = 0; i < MAX_APCLI_NUM; i++)
	{
		pAd->ApCfg.ApCliTab[i].wdev.SecConfig.PmfCfg.Desired_MFPC = FALSE;
		pAd->ApCfg.ApCliTab[i].wdev.SecConfig.PmfCfg.Desired_MFPR = FALSE;
		pAd->ApCfg.ApCliTab[i].wdev.SecConfig.PmfCfg.Desired_PMFSHA256 = FALSE;
	}
#endif /* DOT11W_PMF_SUPPORT */

 
	/*ApCliEnable*/
	if(RTMPGetKeyParameter("ApCliEnable", tmpbuf, 128, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL,";"), i++)
		{
			pApCliEntry = &pAd->ApCfg.ApCliTab[i];
			if ((strncmp(macptr, "0", 1) == 0))
				pApCliEntry->Enable = FALSE;
			else if ((strncmp(macptr, "1", 1) == 0))
				pApCliEntry->Enable = TRUE;
	        else
				pApCliEntry->Enable = FALSE;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCliEntry[%d].Enable=%d\n", i, pApCliEntry->Enable));
	    }
	}
	
	/*ApCliSsid*/
	if(RTMPGetKeyParameter("ApCliSsid", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, FALSE))
	{
		RTMP_STRING *orig_tmpbuf;
		orig_tmpbuf = tmpbuf;
		for (i=0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL,";"), i++)
		{
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;
			pApCliEntry = &pAd->ApCfg.ApCliTab[i];

			/*Ssid acceptable strlen must be less than 32 and bigger than 0.*/
			pApCliEntry->CfgSsidLen = (UCHAR)strlen(macptr);
			if (pApCliEntry->CfgSsidLen > 32)
			{
				pApCliEntry->CfgSsidLen = 0;
				continue;
			}
			if(pApCliEntry->CfgSsidLen > 0)
			{
				memcpy(&pApCliEntry->CfgSsid, macptr, pApCliEntry->CfgSsidLen);
				pApCliEntry->Valid = FALSE;/* it should be set when successfuley association*/
			} else
			{
				NdisZeroMemory(&(pApCliEntry->CfgSsid), MAX_LEN_OF_SSID);
				continue;
			}
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCliEntry[%d].CfgSsidLen=%d, CfgSsid=%s\n", i, pApCliEntry->CfgSsidLen, pApCliEntry->CfgSsid));
		}
	}

#ifdef WH_EZ_SETUP
/*ApCliHideSSID*/
	if(RTMPGetKeyParameter("ApCliHideSSID", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, FALSE))
	{
		for (i=0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL,";"), i++)
		{
			pApCliEntry = &pAd->ApCfg.ApCliTab[i];

			/*Ssid acceptable strlen must be less than 32 and bigger than 0.*/
			pApCliEntry->CfgHideSsidLen = (UCHAR)strlen(macptr);
			if (pApCliEntry->CfgHideSsidLen > 32)
			{
				pApCliEntry->CfgHideSsidLen = 0;
				continue;
			}
			if(pApCliEntry->CfgHideSsidLen > 0)
			{
				memcpy(&pApCliEntry->CfgHideSsid, macptr, pApCliEntry->CfgHideSsidLen);
				pApCliEntry->Valid = FALSE;/* it should be set when successfuley association*/
			} else
			{
				NdisZeroMemory(&(pApCliEntry->CfgHideSsid), MAX_LEN_OF_SSID);
				continue;
			}
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCliEntry[%d].CfgHideSsidLen=%d, CfgSsid=%s\n", i, pApCliEntry->CfgHideSsidLen, pApCliEntry->CfgHideSsid));
		}
	}
#endif

#ifdef DBDC_MODE
	/*ApCliWirelessMode*/
	if(RTMPGetKeyParameter("ApCliWirelessMode", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, FALSE))
	{
		for (i=0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL,";"), i++)
		{
			UCHAR cfg_mode;
			cfg_mode = simple_strtol(macptr, 0, 10);
			pApCliEntry = &pAd->ApCfg.ApCliTab[i];
			pApCliEntry->wdev.PhyMode = cfgmode_2_wmode(cfg_mode);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCliEntry[%d].wdev.PhyMode=%d\n", i, pApCliEntry->wdev.PhyMode));
		}
	}
#ifdef MULTI_PROFILE
	//sanity check apcli wireless mode setting
	{
		{
			UCHAR max_5G_PhyMode = 0;
			UCHAR max_2G_PhyMode = 0;
			INT defualt_5g_rule = 0;

			if (WMODE_5G_ONLY(pAd->ApCfg.MBSSID[0].wdev.PhyMode))
				defualt_5g_rule = 1;
			for (i=0; i < MAX_APCLI_NUM; i++)
			{
				UCHAR mbss_idx;
				UCHAR apcli_phy_mode_correct = 0;
				pApCliEntry = &pAd->ApCfg.ApCliTab[i];
				//check if apcli phy mode setting is in one of all mbss phy mode
				for(mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++)
				{
					BSS_STRUCT *pMbss = NULL;
					struct wifi_dev	*mbss_wdev=NULL;
					pMbss = &pAd->ApCfg.MBSSID[mbss_idx];
					mbss_wdev = &pMbss->wdev;

					if (WMODE_5G_ONLY(mbss_wdev->PhyMode))
						max_5G_PhyMode = (max_5G_PhyMode < mbss_wdev->PhyMode) ? 
										mbss_wdev->PhyMode:max_5G_PhyMode;
					else
						max_2G_PhyMode = (max_2G_PhyMode < mbss_wdev->PhyMode) ? 
										mbss_wdev->PhyMode:max_2G_PhyMode;
					if (defualt_5g_rule == 1)
					{
						if ((i == 0) &&
							WMODE_5G_ONLY(mbss_wdev->PhyMode) &&
							(pApCliEntry->wdev.PhyMode == pAd->ApCfg.MBSSID[mbss_idx].wdev.PhyMode))
						{
							apcli_phy_mode_correct = 1;
						}
						else if ((i == 1) &&
								!WMODE_5G_ONLY(mbss_wdev->PhyMode) &&
								(pApCliEntry->wdev.PhyMode == pAd->ApCfg.MBSSID[mbss_idx].wdev.PhyMode))
						{
							apcli_phy_mode_correct = 1;
						}
					} 
					else
					{
						if ((i == 0) &&
								!WMODE_5G_ONLY(mbss_wdev->PhyMode) &&
								(pApCliEntry->wdev.PhyMode == pAd->ApCfg.MBSSID[mbss_idx].wdev.PhyMode))
						{
							apcli_phy_mode_correct = 1;
						}
						else if ((i == 1) &&
								 WMODE_5G_ONLY(mbss_wdev->PhyMode) &&
								 (pApCliEntry->wdev.PhyMode == pAd->ApCfg.MBSSID[mbss_idx].wdev.PhyMode))
						{
							apcli_phy_mode_correct = 1;
						}
					}
				}
				if (apcli_phy_mode_correct != 1) 
				{
					if (defualt_5g_rule == 1)
					{
						if (i == 0)
							pApCliEntry->wdev.PhyMode = max_5G_PhyMode;
						else if (i == 1)
							pApCliEntry->wdev.PhyMode = max_2G_PhyMode;
					}
					else
					{
						if (i == 0)
							pApCliEntry->wdev.PhyMode = max_2G_PhyMode;
						else if (i == 1)
							pApCliEntry->wdev.PhyMode = max_5G_PhyMode;
					}
				}
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Sanity check in DBDC :ApCliEntry[%d].wdev.PhyMode=%d\n", i, pApCliEntry->wdev.PhyMode));
			}
		}
	}
#endif /*MULTI_PROFILE*/
#endif

	/*ApCliBssid*/
	if(RTMPGetKeyParameter("ApCliBssid", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE))
	{
		RTMP_STRING *orig_tmpbuf;
		orig_tmpbuf = tmpbuf;
		for (i=0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL,";"), i++)
		{
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;
			pApCliEntry = &pAd->ApCfg.ApCliTab[i];

			if(strlen(macptr) != 17)  /*Mac address acceptable format 01:02:03:04:05:06 length 17*/
				continue;
			if(strcmp(macptr,"00:00:00:00:00:00") == 0)
				continue;
			for (j=0; j<MAC_ADDR_LEN; j++)
			{
				AtoH(macptr, &macAddress[j], 1);
				macptr=macptr+3;
			}
			memcpy(pApCliEntry->CfgApCliBssid, &macAddress, MAC_ADDR_LEN);
			pApCliEntry->Valid = FALSE;/* it should be set when successfuley association*/
		}
	}
 
	/* ApCliTxMode*/
	if (RTMPGetKeyParameter("ApCliTxMode", tmpbuf, 25, buffer, TRUE))
	{
		RTMP_STRING *orig_tmpbuf;
		orig_tmpbuf = tmpbuf;
		for (i = 0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL,";"), i++)
		{
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;
			wdev = &pAd->ApCfg.ApCliTab[i].wdev;

			wdev->DesiredTransmitSetting.field.FixedTxMode =
										RT_CfgSetFixedTxPhyMode(macptr);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(apcli%d) Tx Mode = %d\n", i,
											wdev->DesiredTransmitSetting.field.FixedTxMode));
		}
	}

	/* ApCliTxMcs*/
	if (RTMPGetKeyParameter("ApCliTxMcs", tmpbuf, 50, buffer, TRUE))
	{
		RTMP_STRING *orig_tmpbuf;
		orig_tmpbuf = tmpbuf;
		for (i = 0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL,";"), i++)
		{
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;
			wdev = &pAd->ApCfg.ApCliTab[i].wdev;

			wdev->DesiredTransmitSetting.field.MCS =
					RT_CfgSetTxMCSProc(macptr, &wdev->bAutoTxRateSwitch);

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(apcli%d) Tx MCS = %s(%d)\n", i,
						(wdev->DesiredTransmitSetting.field.MCS == MCS_AUTO ? "AUTO" : ""),
						wdev->DesiredTransmitSetting.field.MCS));
		}
	}


#ifdef WSC_AP_SUPPORT
	/* Wsc4digitPinCode = TRUE use 4-digit Pin code, otherwise 8-digit Pin code */
	if (RTMPGetKeyParameter("ApCli_Wsc4digitPinCode", tmpbuf, 32, buffer, TRUE))
	{
		RTMP_STRING *orig_tmpbuf;
		orig_tmpbuf = tmpbuf;
	    for (i = 0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL,";"), i++)
	    {   
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;
    		if (simple_strtol(macptr, 0, 10) != 0)
    			pAd->ApCfg.ApCliTab[i].WscControl.WscEnrollee4digitPinCode = TRUE;
    		else //Disable
    			pAd->ApCfg.ApCliTab[i].WscControl.WscEnrollee4digitPinCode = FALSE;
		    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(apcli%d) ApCli_Wsc4digitPinCode=%d\n", i,
                    pAd->ApCfg.ApCliTab[i].WscControl.WscEnrollee4digitPinCode));
	    }
	}
    
#ifdef APCLI_SUPPORT
    /* ApCliWscScanMode */
    if(RTMPGetKeyParameter("ApCliWscScanMode", tmpbuf, 32, buffer, TRUE))
    {
        UCHAR Mode;
		RTMP_STRING *orig_tmpbuf;
		orig_tmpbuf = tmpbuf;
        for (i = 0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL,";"), i++)
        {
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;
            Mode = simple_strtol(macptr, 0, 10);
            if(Mode != TRIGGER_PARTIAL_SCAN)
                Mode = TRIGGER_FULL_SCAN;
         
            pAd->ApCfg.ApCliTab[i].WscControl.WscApCliScanMode = Mode;
            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(apcli%d) WscApCliScanMode=%d\n",i, Mode));
        }
    }
#endif /* APCLI_SUPPORT */	
#endif /* WSC_AP_SUPPORT */


#ifdef UAPSD_SUPPORT
	/*APSDCapable*/
	if(RTMPGetKeyParameter("ApCliAPSDCapable", tmpbuf, 10, buffer, TRUE))
	{
		RTMP_STRING *orig_tmpbuf;
		orig_tmpbuf = tmpbuf;
		pAd->ApCfg.FlgApCliIsUapsdInfoUpdated = TRUE;

		for (i = 0, macptr = rstrtok(tmpbuf,";");(macptr && i < MAX_APCLI_NUM);	macptr = rstrtok(NULL,";"), i++)
		{
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;
			pApCliEntry = &pAd->ApCfg.ApCliTab[i];

			pApCliEntry->wdev.UapsdInfo.bAPSDCapable = \
									(UCHAR) simple_strtol(macptr, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ApCliAPSDCapable[%d]=%d\n", i,
					pApCliEntry->wdev.UapsdInfo.bAPSDCapable));
	    }
	}
#endif /* UAPSD_SUPPORT */

	/* ApCliNum */
	if(RTMPGetKeyParameter("ApCliNum", tmpbuf, 10, buffer, TRUE))
	{
		if (simple_strtol(tmpbuf, 0, 10) <= MAX_APCLI_NUM)
		{
			pAd->ApCfg.ApCliNum = simple_strtol(tmpbuf, 0, 10);
		}
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(apcli) ApCliNum=%d\n", pAd->ApCfg.ApCliNum));
	}
#if defined(DBDC_MODE) && defined(MT7615)
	if (pAd->CommonCfg.dbdc_mode == TRUE)
		pAd->ApCfg.ApCliNum = 2;
	else
		pAd->ApCfg.ApCliNum = 1;
#else	
		pAd->ApCfg.ApCliNum = MAX_APCLI_NUM_DEFAULT;
#endif
#ifdef APCLI_CONNECTION_TRIAL
	pAd->ApCfg.ApCliNum++;
	/* ApCliTrialCh */
	if(RTMPGetKeyParameter("ApCliTrialCh", tmpbuf, 128, buffer, TRUE))
	{
		// last IF is for apcli connection trial
		pApCliEntry = &pAd->ApCfg.ApCliTab[pAd->ApCfg.ApCliNum-1];
		pApCliEntry->TrialCh = (UCHAR) simple_strtol(tmpbuf, 0, 10);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("TrialChannel=%d\n", pApCliEntry->TrialCh));
	}
#endif /* APCLI_CONNECTION_TRIAL */

#ifdef DOT11W_PMF_SUPPORT
	/* Protection Management Frame Capable */
	if (RTMPGetKeyParameter("ApCliPMFMFPC", tmpbuf, 32, buffer, TRUE))
	{
		RTMP_STRING *orig_tmpbuf;
		orig_tmpbuf = tmpbuf;
		for (i = 0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL,";"), i++)
		{
			POS_COOKIE pObj;
			INT backup_ioctl_if;
			INT backup_ioctl_if_type;

			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;
			pObj = (POS_COOKIE) pAd->OS_Cookie;
			RtmpOSNetDevProtect(1);
			backup_ioctl_if = pObj->ioctl_if ;
			backup_ioctl_if_type = pObj->ioctl_if_type;
			pObj->ioctl_if = i;
			pObj->ioctl_if_type = INT_APCLI;
			Set_ApCliPMFMFPC_Proc(pAd, macptr);
			pObj->ioctl_if = backup_ioctl_if;
			pObj->ioctl_if_type = backup_ioctl_if_type;
			RtmpOSNetDevProtect(0);
		}
	}

	/* Protection Management Frame Required */
	if (RTMPGetKeyParameter("ApCliPMFMFPR", tmpbuf, 32, buffer, TRUE))
	{
		RTMP_STRING *orig_tmpbuf;
		orig_tmpbuf = tmpbuf;
		for (i = 0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL,";"), i++)
		{
			POS_COOKIE pObj;
			INT backup_ioctl_if;
			INT backup_ioctl_if_type;

			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;
			pObj = (POS_COOKIE) pAd->OS_Cookie;
			RtmpOSNetDevProtect(1);
			backup_ioctl_if = pObj->ioctl_if ;
			backup_ioctl_if_type = pObj->ioctl_if_type;
			pObj->ioctl_if = i;
			pObj->ioctl_if_type = INT_APCLI;
			Set_ApCliPMFMFPR_Proc(pAd, macptr);
			pObj->ioctl_if = backup_ioctl_if;
			pObj->ioctl_if_type = backup_ioctl_if_type;
			RtmpOSNetDevProtect(0);
		}	        
	}

	if (RTMPGetKeyParameter("ApCliPMFSHA256", tmpbuf, 32, buffer, TRUE))
	{
		RTMP_STRING *orig_tmpbuf;
		orig_tmpbuf = tmpbuf;
		for (i = 0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL,";"), i++)
		{
			POS_COOKIE pObj;
			INT backup_ioctl_if;
			INT backup_ioctl_if_type;

			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;
			pObj = (POS_COOKIE) pAd->OS_Cookie;
			RtmpOSNetDevProtect(1);
			backup_ioctl_if = pObj->ioctl_if ;
			backup_ioctl_if_type = pObj->ioctl_if_type;
			pObj->ioctl_if = i;
			pObj->ioctl_if_type = INT_APCLI;
			Set_ApCliPMFSHA256_Proc(pAd, macptr);
			pObj->ioctl_if = backup_ioctl_if;
			pObj->ioctl_if_type = backup_ioctl_if_type;
			RtmpOSNetDevProtect(0);
		}
	}
#endif /* DOT11W_PMF_SUPPORT */

}
#endif /* APCLI_SUPPORT */


static void rtmp_read_acl_parms_from_file(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buffer)
{
	RTMP_STRING tok_str[32], *macptr;
	INT			i=0, j=0, idx;
	UCHAR		macAddress[MAC_ADDR_LEN];


	memset(macAddress, 0, MAC_ADDR_LEN);
	for (idx=0; idx<MAX_MBSSID_NUM(pAd); idx++)
	{
		memset(&pAd->ApCfg.MBSSID[idx].AccessControlList, 0, sizeof(RT_802_11_ACL));
		/* AccessPolicyX*/
		snprintf(tok_str, sizeof(tok_str), "AccessPolicy%d", idx);
		if (RTMPGetKeyParameter(tok_str, tmpbuf, 10, buffer, TRUE))
		{
			switch (simple_strtol(tmpbuf, 0, 10))
			{
				case 1: /* Allow All, and the AccessControlList is positive now.*/
					pAd->ApCfg.MBSSID[idx].AccessControlList.Policy = 1;
					break;
				case 2: /* Reject All, and the AccessControlList is negative now.*/
					pAd->ApCfg.MBSSID[idx].AccessControlList.Policy = 2;
					break;
				case 0: /* Disable, don't care the AccessControlList.*/
				default:
					pAd->ApCfg.MBSSID[idx].AccessControlList.Policy = 0;
					break;
			}
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s=%ld\n", tok_str, pAd->ApCfg.MBSSID[idx].AccessControlList.Policy));
		}
		/* AccessControlListX*/
		snprintf(tok_str, sizeof(tok_str), "AccessControlList%d", idx);
		if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE))
		{
			for (i=0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
			{
				if (strlen(macptr) != 17)  /* Mac address acceptable format 01:02:03:04:05:06 length 17*/
					continue;

				ASSERT(pAd->ApCfg.MBSSID[idx].AccessControlList.Num <= MAX_NUM_OF_ACL_LIST);

				for (j=0; j<MAC_ADDR_LEN; j++)
				{
					AtoH(macptr, &macAddress[j], 1);
					macptr=macptr+3;
				}

				if (pAd->ApCfg.MBSSID[idx].AccessControlList.Num == MAX_NUM_OF_ACL_LIST)
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("The AccessControlList is full, and no more entry can join the list!\n"));
        			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("The last entry of ACL is %02x:%02x:%02x:%02x:%02x:%02x\n",
        				macAddress[0],macAddress[1],macAddress[2],macAddress[3],macAddress[4],macAddress[5]));

				    break;
				}

				pAd->ApCfg.MBSSID[idx].AccessControlList.Num++;
				NdisMoveMemory(pAd->ApCfg.MBSSID[idx].AccessControlList.Entry[(pAd->ApCfg.MBSSID[idx].AccessControlList.Num - 1)].Addr, macAddress, MAC_ADDR_LEN);
			}
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s=Get %ld Mac Address\n", tok_str, pAd->ApCfg.MBSSID[idx].AccessControlList.Num));
 		}
	}
}

/*
    ========================================================================

    Routine Description:
        In kernel mode read parameters from file

    Arguments:
        src                     the location of the file.
        dest                        put the parameters to the destination.
        Length                  size to read.

    Return Value:
        None

    Note:

    ========================================================================
*/

static void rtmp_read_ap_edca_from_file(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buffer)
{
	RTMP_STRING *macptr,*edcaptr,tok_str[16];
	INT	i=0,j=0;
	EDCA_PARM *pEdca;
	RTMP_STRING *ptmpStr[6];
	struct wifi_dev *wdev = NULL;
	UCHAR ack_policy[WMM_NUM_OF_AC];

	for(j=0;j<WMM_NUM;j++)
	{
		snprintf(tok_str, sizeof(tok_str), "APEdca%d", j);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n",tok_str));

		if(RTMPGetKeyParameter(tok_str, tmpbuf, 128, buffer, TRUE))
		{
			pEdca = &pAd->CommonCfg.APEdcaParm[j];

			for (i = 0, edcaptr = rstrtok(tmpbuf,";"); edcaptr; edcaptr = rstrtok(NULL,";"), i++)
			{
				ptmpStr[i] = edcaptr;
			}
			
			if (i != 6) 
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Input parameter incorrect\n"));
				return;
			}

			/*APValid*/
			edcaptr = ptmpStr[0];
			if(edcaptr)
			{
				pEdca->bValid = (UCHAR) simple_strtol(edcaptr, 0, 10);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Valid=%d\n",pEdca->bValid));
			}
		    /*APAifsn*/
			edcaptr = ptmpStr[1];
			if(edcaptr)
			{
			    for (i = 0, macptr = rstrtok(edcaptr,","); macptr; macptr = rstrtok(NULL,","), i++)
			    {
					pEdca->Aifsn[i] = (UCHAR) simple_strtol(macptr, 0, 10);

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("APAifsn[%d]=%d\n", i, pEdca->Aifsn[i]));
			    }
			}
			/*APCwmin*/

			edcaptr = ptmpStr[2];
			if(edcaptr)
			{
			    for (i = 0, macptr = rstrtok(edcaptr,","); macptr; macptr = rstrtok(NULL,","), i++)
			    {
					pEdca->Cwmin[i] = (UCHAR) simple_strtol(macptr, 0, 10);

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("APCwmin[%d]=%d\n", i, pEdca->Cwmin[i]));
			    }
			}
			/*APCwmax*/
			edcaptr = ptmpStr[3];
			if(edcaptr)

			{
			    for (i = 0, macptr = rstrtok(edcaptr,","); macptr; macptr = rstrtok(NULL,","), i++)
			    {
					pEdca->Cwmax[i] = (UCHAR) simple_strtol(macptr, 0, 10);

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("APCwmax[%d]=%d\n", i, pEdca->Cwmax[i]));
			    }
			}
			/*APTxop*/
			edcaptr = ptmpStr[4];
			if(edcaptr)
			{
			    for (i = 0, macptr = rstrtok(edcaptr,","); macptr; macptr = rstrtok(NULL,","), i++)
			    {
					pEdca->Txop[i] = (USHORT) simple_strtol(macptr, 0, 10);

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("APTxop[%d]=%d\n", i, pEdca->Txop[i]));
			    }
			}
			/*APACM*/
			edcaptr = ptmpStr[5];
			if(edcaptr)
			{
			    for (i = 0, macptr = rstrtok(edcaptr,","); macptr; macptr = rstrtok(NULL,","), i++)
			    {
					pEdca->bACM[i] = (BOOLEAN) simple_strtol(macptr, 0, 10);

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("APACM[%d]=%d\n", i, pEdca->bACM[i]));
			    }
			}
		}
	}
	/*AckPolicy*/
	for(i = 0 ; i < pAd->ApCfg.BssidNum; i++){
		snprintf(tok_str, sizeof(tok_str), "APAckPolicy%d", i);
		if(RTMPGetKeyParameter(tok_str, tmpbuf, 128, buffer, TRUE)){
			wdev = &pAd->ApCfg.MBSSID[i].wdev;
			for (j = 0, edcaptr = rstrtok(tmpbuf,";"); edcaptr; edcaptr = rstrtok(NULL,";"), j++){
				ack_policy[j] = (USHORT) simple_strtol(edcaptr, 0, 10);
			}
			wlan_config_set_ack_policy(wdev,ack_policy);
		}
	}
}

static void rtmp_read_bss_edca_from_file(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buffer)
{
	RTMP_STRING *macptr,*edcaptr,tok_str[16];
	INT	i=0,j=0;
	RTMP_STRING *ptmpStr[6];
	struct _EDCA_PARM *pBssEdca = NULL;

	for(j=0;j<pAd->ApCfg.BssidNum;j++)
	{
		snprintf(tok_str, sizeof(tok_str), "BSSEdca%d", j);

		if(RTMPGetKeyParameter(tok_str, tmpbuf, 128, buffer, TRUE))
		{
			pBssEdca = wlan_config_get_ht_edca(&pAd->ApCfg.MBSSID[j].wdev);

			if (!pBssEdca)
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("BSS[%d]: Invalid pBssEdca\n", j));
				return;
			}

			for (i = 0, edcaptr = rstrtok(tmpbuf,";"); edcaptr; edcaptr = rstrtok(NULL,";"), i++)
			{
				ptmpStr[i] = edcaptr;
			}
			
			if (i != 5) 
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Input parameter incorrect\n"));
				return;
			}

		  	/*BSSAifsn*/
			edcaptr = ptmpStr[0];
			if(edcaptr)
			{
			    for (i = 0, macptr = rstrtok(edcaptr,","); macptr; macptr = rstrtok(NULL,","), i++)
			    {
					pBssEdca->Aifsn[i] = (UCHAR) simple_strtol(macptr, 0, 10);

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BSSAifsn[%d]=%d\n", i, pBssEdca->Aifsn[i]));
			    }
			}
			/*BSSCwmin*/

			edcaptr = ptmpStr[1];
			if(edcaptr)
			{
			    for (i = 0, macptr = rstrtok(edcaptr,","); macptr; macptr = rstrtok(NULL,","), i++)
			    {
					pBssEdca->Cwmin[i] = (UCHAR) simple_strtol(macptr, 0, 10);

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSSCwmin[%d]=%d\n", i, pBssEdca->Cwmin[i]));
			    }
			}
			/*BSSCwmax*/
			edcaptr = ptmpStr[2];
			if(edcaptr)

			{
			    for (i = 0, macptr = rstrtok(edcaptr,","); macptr; macptr = rstrtok(NULL,","), i++)
			    {
					pBssEdca->Cwmax[i] = (UCHAR) simple_strtol(macptr, 0, 10);

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSSCwmax[%d]=%d\n", i, pBssEdca->Cwmax[i]));
			    }
			}
			/*BSSTxop*/
			edcaptr = ptmpStr[3];
			if(edcaptr)
			{
			    for (i = 0, macptr = rstrtok(edcaptr,","); macptr; macptr = rstrtok(NULL,","), i++)
			    {
					pBssEdca->Txop[i] = (USHORT) simple_strtol(macptr, 0, 10);

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSSTxop[%d]=%d\n", i, pBssEdca->Txop[i]));
			    }
			}
			/*BSSACM*/
			edcaptr = ptmpStr[4];
			if(edcaptr)
			{
			    for (i = 0, macptr = rstrtok(edcaptr,","); macptr; macptr = rstrtok(NULL,","), i++)
			    {
					pBssEdca->bACM[i] = (BOOLEAN) simple_strtol(macptr, 0, 10);

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSSACM[%d]=%d\n", i, pBssEdca->bACM[i]));
			    }
			}
		}
	}
}

static void rtmp_read_ap_wmm_parms_from_file(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buffer)
{
	RTMP_STRING *macptr;
	INT	i=0, j=0;
	struct _EDCA_PARM *pBssEdca = NULL;

	/*WmmCapable*/
	if(RTMPGetKeyParameter("WmmCapable", tmpbuf, 32, buffer, TRUE))
	{
	    BOOLEAN bEnableWmm = FALSE;
	    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	    {
			if (i >= pAd->ApCfg.BssidNum)
			{
				break;
			}

			if(simple_strtol(macptr, 0, 10) != 0)
			{
				pAd->ApCfg.MBSSID[i].wdev.bWmmCapable = TRUE;
				bEnableWmm = TRUE;
			}
			else
			{
				pAd->ApCfg.MBSSID[i].wdev.bWmmCapable = FALSE;
			}

			if (bEnableWmm)
			{
				pAd->CommonCfg.APEdcaParm[0].bValid = TRUE;

				/* Apply BSS[0] setting to all as default */
				if (i == 0)
					wlan_config_set_edca_valid_all(&pAd->wpf,TRUE);
				else
					wlan_config_set_edca_valid(&pAd->ApCfg.MBSSID[i].wdev,TRUE);
			}
			else
			{
				pAd->CommonCfg.APEdcaParm[0].bValid = FALSE;

				/* Apply BSS[0] setting to all as default */
				if (i == 0)
					wlan_config_set_edca_valid_all(&pAd->wpf,FALSE);
				else
					wlan_config_set_edca_valid(&pAd->ApCfg.MBSSID[i].wdev,FALSE);
			}

			pAd->ApCfg.MBSSID[i].bWmmCapableOrg = \
											pAd->ApCfg.MBSSID[i].wdev.bWmmCapable;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(ra%d) WmmCapable=%d\n", i, pAd->ApCfg.MBSSID[i].wdev.bWmmCapable));
	    }
	}

	/*New WMM Parameter*/
	rtmp_read_ap_edca_from_file(pAd,tmpbuf,buffer);
	/*DLSCapable*/
	if(RTMPGetKeyParameter("DLSCapable", tmpbuf, 32, buffer, TRUE))
	{
	    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	    {
			if (i >= pAd->ApCfg.BssidNum)
			{
				break;
			}

			if(simple_strtol(macptr, 0, 10) != 0)  /*Enable*/
			{
				pAd->ApCfg.MBSSID[i].bDLSCapable = TRUE;
			}
			else /*Disable*/
			{
				pAd->ApCfg.MBSSID[i].bDLSCapable = FALSE;
			}

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(ra%d) DLSCapable=%d\n", i, pAd->ApCfg.MBSSID[i].bDLSCapable));
	    }
	}
	/*APAifsn*/
	if(RTMPGetKeyParameter("APAifsn", tmpbuf, 32, buffer, TRUE))
	{
	    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	    {
			pAd->CommonCfg.APEdcaParm[0].Aifsn[i] = (UCHAR) simple_strtol(macptr, 0, 10);;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("APAifsn[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[0].Aifsn[i]));
	    }
	}
	/*APCwmin*/
	if(RTMPGetKeyParameter("APCwmin", tmpbuf, 32, buffer, TRUE))
	{
	    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	    {
			pAd->CommonCfg.APEdcaParm[0].Cwmin[i] = (UCHAR) simple_strtol(macptr, 0, 10);;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("APCwmin[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[0].Cwmin[i]));
	    }
	}
	/*APCwmax*/
	if(RTMPGetKeyParameter("APCwmax", tmpbuf, 32, buffer, TRUE))
	{
	    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	    {
			pAd->CommonCfg.APEdcaParm[0].Cwmax[i] = (UCHAR) simple_strtol(macptr, 0, 10);;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("APCwmax[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[0].Cwmax[i]));
	    }
	}
	/*APTxop*/
	if(RTMPGetKeyParameter("APTxop", tmpbuf, 32, buffer, TRUE))
	{
	    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	    {
			pAd->CommonCfg.APEdcaParm[0].Txop[i] = (USHORT) simple_strtol(macptr, 0, 10);;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("APTxop[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[0].Txop[i]));
	    }
	}
	/*APACM*/
	if(RTMPGetKeyParameter("APACM", tmpbuf, 32, buffer, TRUE))
	{
	    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	    {
			pAd->CommonCfg.APEdcaParm[0].bACM[i] = (BOOLEAN) simple_strtol(macptr, 0, 10);;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("APACM[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[0].bACM[i]));
	    }
	}

	/* Apply default (BSS) WMM Parameter */
	for(j=0;j<pAd->ApCfg.BssidNum;j++)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS[%d]:\n", j));

		pBssEdca = wlan_config_get_ht_edca(&pAd->ApCfg.MBSSID[j].wdev);

		if (!pBssEdca)
			continue;

		/*BSSAifsn*/
		if(RTMPGetKeyParameter("BSSAifsn", tmpbuf, 32, buffer, TRUE))
		{
		    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		    {
				pBssEdca->Aifsn[i] = (UCHAR) simple_strtol(macptr, 0, 10);;

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSSAifsn[%d]=%d\n", i, pBssEdca->Aifsn[i]));
		    }
		}
		/*BSSCwmin*/
		if(RTMPGetKeyParameter("BSSCwmin", tmpbuf, 32, buffer, TRUE))
		{
		    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		    {
				pBssEdca->Cwmin[i] = (UCHAR) simple_strtol(macptr, 0, 10);;

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSSCwmin[%d]=%d\n", i, pBssEdca->Cwmin[i]));
		    }
		}
		/*BSSCwmax*/
		if(RTMPGetKeyParameter("BSSCwmax", tmpbuf, 32, buffer, TRUE))
		{
		    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		    {
				pBssEdca->Cwmax[i] = (UCHAR) simple_strtol(macptr, 0, 10);;

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSSCwmax[%d]=%d\n", i, pBssEdca->Cwmax[i]));
		    }
		}
		/*BSSTxop*/
		if(RTMPGetKeyParameter("BSSTxop", tmpbuf, 32, buffer, TRUE))
		{
		    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		    {
				pBssEdca->Txop[i] = (USHORT) simple_strtol(macptr, 0, 10);;

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSSTxop[%d]=%d\n", i, pBssEdca->Txop[i]));
		    }
		}
		/*BSSACM*/
		if(RTMPGetKeyParameter("BSSACM", tmpbuf, 32, buffer, TRUE))
		{
		    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		    {
				pBssEdca->bACM[i] = (BOOLEAN) simple_strtol(macptr, 0, 10);;

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSSACM[%d]=%d\n", i, pBssEdca->bACM[i]));
		    }
		}
	}

	/*Apply new (BSS) WMM Parameter*/
	rtmp_read_bss_edca_from_file(pAd,tmpbuf,buffer);

	/*AckPolicy*/
	if(RTMPGetKeyParameter("AckPolicy", tmpbuf, 32, buffer, TRUE))
	{
	    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	    {
			pAd->CommonCfg.AckPolicy[i] = (UCHAR) simple_strtol(macptr, 0, 10);;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("AckPolicy[%d]=%d\n", i, pAd->CommonCfg.AckPolicy[i]));
	    }
		wlan_config_set_ack_policy_all(&pAd->wpf,pAd->CommonCfg.AckPolicy);
	}
#ifdef UAPSD_SUPPORT
	/*APSDCapable*/
	if(RTMPGetKeyParameter("APSDCapable", tmpbuf, 10, buffer, TRUE))
	{

	    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	    {
			if (i < HW_BEACON_MAX_NUM)
			{
				pAd->ApCfg.MBSSID[i].wdev.UapsdInfo.bAPSDCapable = \
										(UCHAR) simple_strtol(macptr, 0, 10);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("APSDCapable[%d]=%d\n", i,
						pAd->ApCfg.MBSSID[i].wdev.UapsdInfo.bAPSDCapable));
			}
	    }

		if (i == 1)
		{
			/*
				Old format in UAPSD settings: only 1 parameter
				i.e. UAPSD for all BSS is enabled or disabled.
			*/
			for(i=1; i<HW_BEACON_MAX_NUM; i++)
			{
				pAd->ApCfg.MBSSID[i].wdev.UapsdInfo.bAPSDCapable =
							pAd->ApCfg.MBSSID[0].wdev.UapsdInfo.bAPSDCapable;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("APSDCapable[%d]=%d\n", i,
						pAd->ApCfg.MBSSID[i].wdev.UapsdInfo.bAPSDCapable));
			}
		}

#ifdef APCLI_SUPPORT
		if (pAd->ApCfg.FlgApCliIsUapsdInfoUpdated == FALSE)
		{
			/*
				Backward:
				All UAPSD for AP Client interface is same as MBSS0
				when we can not find "ApCliAPSDCapable".
				When we find "ApCliAPSDCapable" hereafter, we will over-write.
			*/
			for(i=0; i<MAX_APCLI_NUM; i++)
			{
				pAd->ApCfg.ApCliTab[i].wdev.UapsdInfo.bAPSDCapable = \
								pAd->ApCfg.MBSSID[0].wdev.UapsdInfo.bAPSDCapable;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("default ApCliAPSDCapable[%d]=%d\n",
						i, pAd->ApCfg.ApCliTab[i].wdev.UapsdInfo.bAPSDCapable));
			}
		}
#endif /* APCLI_SUPPORT */
	}
#endif /* UAPSD_SUPPORT */
}
  
#endif /* CONFIG_AP_SUPPORT */

INT rtmp_band_index_get_by_order(struct _RTMP_ADAPTER *pAd, UCHAR order)
{
	INT ret = DBDC_BAND0;

#ifdef MULTI_PROFILE 
	if (is_multi_profile_enable(pAd)) {
#ifdef DEFAULT_5G_PROFILE
		if (order == 0)
			ret = DBDC_BAND1;
		if (order == 1)
			ret = DBDC_BAND0;
#else /*DEFAULT_5G_PROFILE*/
		if (order == 0)
			ret = DBDC_BAND0;
		if (order == 1)
			ret = DBDC_BAND1;
#endif /*DEFAULT_5G_PROFILE*/
	} else {
		if (order == 0)
			ret = DBDC_BAND0;
		if (order == 1)
			ret = DBDC_BAND1;
	}
#else /*MULTI_PROFILE*/
	if (order == 0)
		ret = DBDC_BAND0;
	if (order == 1)
		ret = DBDC_BAND1;
#endif /*MULTI_PROFILE*/

	return ret;
}

static void read_frag_thld_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i = 0, bss_idx = 0;
	UINT32 frag_thld = 0;
	RTMP_STRING *macptr = NULL;
	struct wifi_dev *wdev = NULL;

	if(RTMPGetKeyParameter("FragThreshold", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i > DBDC_BAND_NUM)
				break;
			frag_thld = simple_strtol(macptr, 0, 10);
			if (frag_thld > MAX_FRAG_THRESHOLD || frag_thld < MIN_FRAG_THRESHOLD)
				frag_thld = MAX_FRAG_THRESHOLD;
			else if (frag_thld % 2 == 1)
				frag_thld -= 1;
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
				for (bss_idx = 0; bss_idx < MAX_MBSSID_NUM(pAd); bss_idx++) {
					if (bss_idx >= HW_BEACON_MAX_NUM)
						break;
					wdev = &pAd->ApCfg.MBSSID[bss_idx].wdev;
					wlan_config_set_frag_thld(wdev, frag_thld);
				}
			}
#endif /*CONFIG_AP_SUPPORT*/
		}
	}
}

static VOID read_rts_pkt_thld_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i, bss_idx;
	UCHAR rts_pkt_thld = 0;
	RTMP_STRING *macptr = NULL;
	struct wifi_dev *wdev = NULL;

	if (RTMPGetKeyParameter("RTSPktThreshold", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i > DBDC_BAND_NUM)
				break;
			rts_pkt_thld = (UCHAR)os_str_tol(macptr, 0, 10);
			if ((rts_pkt_thld < 1) || (rts_pkt_thld > MAX_RTS_PKT_THRESHOLD))
				rts_pkt_thld = MAX_RTS_PKT_THRESHOLD;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("profile: RTSPktThreshold[%d]=%d\n", i, rts_pkt_thld));
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
				for (bss_idx = 0; bss_idx < MAX_MBSSID_NUM(pAd); bss_idx++) {
					if (bss_idx >= HW_BEACON_MAX_NUM)
						break;
					wdev = &pAd->ApCfg.MBSSID[bss_idx].wdev;
					wlan_config_set_rts_pkt_thld(wdev, rts_pkt_thld);
				}
			}
#endif /*CONFIG_AP_SUPPORT*/
		}
	}
}
		
static VOID read_rts_len_thld_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i, bss_idx;
	UINT32 rts_thld = 0;
	RTMP_STRING *macptr = NULL;
	struct wifi_dev *wdev = NULL;

	if(RTMPGetKeyParameter("RTSThreshold", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i > DBDC_BAND_NUM)
				break;
			rts_thld = (UINT32)simple_strtol(macptr, 0, 10);
			if ((rts_thld > MAX_RTS_THRESHOLD) || (rts_thld < 1))
				rts_thld = MAX_RTS_THRESHOLD;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("profile: RTSThreshold[%d]=%d\n", i, rts_thld));
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
				for (bss_idx = 0; bss_idx < MAX_MBSSID_NUM(pAd); bss_idx++) {
					if (bss_idx >= HW_BEACON_MAX_NUM)
						break;
					wdev = &pAd->ApCfg.MBSSID[bss_idx].wdev;
					wlan_config_set_rts_len_thld(wdev, rts_thld);
				}
			}
#endif
		}
	}
}



#ifdef DOT11_VHT_AC
static void VHTParametersHook(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *pValueStr,
	IN RTMP_STRING *pInput)
{
#ifdef CONFIG_AP_SUPPORT
	INT i=0;
	RTMP_STRING *Bufptr;
#endif /* CONFIG_AP_SUPPORT */

	long Value;
	UCHAR vht_bw;

	/* Channel Width */
	if (RTMPGetKeyParameter("VHT_BW", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value <= VHT_BW_8080){
			vht_bw = Value;
		}
		else{
			vht_bw = VHT_BW_2040;
		}

		if(pAd->CommonCfg.dbdc_mode && (vht_bw > VHT_BW_80))
			vht_bw = VHT_BW_80;

		pAd->CommonCfg.vht_bw = vht_bw;
		pAd->CommonCfg.cfg_vht_bw = vht_bw;

		wlan_config_set_vht_bw_all(&pAd->wpf,vht_bw);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
                ("VHT: Channel Width = %s MHz\n",
                 VhtBw2Str(vht_bw)));
	}

	/* VHT GI setting */
	if (RTMPGetKeyParameter("VHT_SGI", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value == GI_800)
			pAd->CommonCfg.vht_sgi = GI_800;
		else
			pAd->CommonCfg.vht_sgi = GI_400;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VHT: Short GI for 80Mhz/160Mhz  = %s\n",
					(pAd->CommonCfg.vht_sgi==GI_800) ? "Disabled" : "Enable" ));
	}

	/* VHT STBC */
	if (RTMPGetKeyParameter("VHT_STBC", pValueStr, 25, pInput, TRUE))
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			for (i = 0, Bufptr = rstrtok(pValueStr,";"); (Bufptr && i < MAX_MBSSID_NUM(pAd)); Bufptr = rstrtok(NULL,";"), i++)
			{
				struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[i].wdev;

				Value = simple_strtol(Bufptr, 0, 10);
				if (Value == STBC_NONE)
					wlan_config_set_vht_stbc(wdev, STBC_NONE);
				else
					wlan_config_set_vht_stbc(wdev, STBC_USE);

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(IF-ra%d) VHT: VHT_STBC = %d\n", i, wlan_config_get_vht_stbc(wdev)));
			}
		}
#endif /* CONFIG_AP_SUPPORT */

	}

	/* bandwidth signaling */
	if (RTMPGetKeyParameter("VHT_BW_SIGNAL", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value >= 0 && Value <= 2)
			pAd->CommonCfg.vht_bw_signal = Value;
		else
			pAd->CommonCfg.vht_bw_signal = BW_SIGNALING_DISABLE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VHT: BW SIGNALING = %d\n", pAd->CommonCfg.vht_bw_signal));
	}

	/* Disallow non-VHT connection */
	if (RTMPGetKeyParameter("VHT_DisallowNonVHT", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value == 0)
			pAd->CommonCfg.bNonVhtDisallow = FALSE;
		else
			pAd->CommonCfg.bNonVhtDisallow = TRUE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VHT: VHT_DisallowNonVHT = %d\n", pAd->CommonCfg.bNonVhtDisallow));
	}

	/* VHT LDPC */
	if (RTMPGetKeyParameter("VHT_LDPC", pValueStr, 25, pInput, TRUE))
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			for (i = 0, Bufptr = rstrtok(pValueStr,";"); (Bufptr && i < MAX_MBSSID_NUM(pAd)); Bufptr = rstrtok(NULL,";"), i++)
			{
				struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[i].wdev;

				Value = simple_strtol(Bufptr, 0, 10);

				if (Value == 0)
					wlan_config_set_vht_ldpc(wdev, FALSE);
				else
					wlan_config_set_vht_ldpc(wdev, TRUE);

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(IF-ra%d) VHT: VHT_LDPC = %d\n", i, wlan_config_get_vht_ldpc(wdev)));
			}
		}
#endif /* CONFIG_AP_SUPPORT */

	}

	/* VHT Secondary80 */
	if (RTMPGetKeyParameter("VHT_Sec80_Channel", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);
		pAd->CommonCfg.vht_cent_ch2 = vht_cent_ch_freq((UCHAR)Value, VHT_BW_80);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("VHT: Secondary80 = %ld, Center = %d\n",
					Value, pAd->CommonCfg.vht_cent_ch2));
	}

    /* 2.4G 256QAM */
    if (RTMPGetKeyParameter("G_BAND_256QAM", pValueStr, 25, pInput, TRUE)) {
		Value = simple_strtol(pValueStr, 0, 10);
        pAd->CommonCfg.g_band_256_qam = (Value) ? TRUE : FALSE;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("VHT: G_BAND_256QAM = %ld\n", Value));
    }

#ifdef WFA_VHT_PF
	/* VHT highest Tx Rate with LGI */
	if (RTMPGetKeyParameter("VHT_TX_HRATE", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value >= 0 && Value <= 2)
			pAd->CommonCfg.vht_tx_hrate = Value;
		else
			pAd->CommonCfg.vht_tx_hrate = 0;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VHT: TX HighestRate = %d\n", pAd->CommonCfg.vht_tx_hrate));
	}

	if (RTMPGetKeyParameter("VHT_RX_HRATE", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value >= 0 && Value <= 2)
			pAd->CommonCfg.vht_rx_hrate = Value;
		else
			pAd->CommonCfg.vht_rx_hrate = 0;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VHT: RX HighestRate = %d\n", pAd->CommonCfg.vht_rx_hrate));
	}

	if (RTMPGetKeyParameter("VHT_MCS_CAP", pValueStr, 25, pInput, TRUE))
		set_vht_nss_mcs_cap(pAd, pValueStr);
#endif /* WFA_VHT_PF */

}

#endif /* DOT11_VHT_AC */


#ifdef DOT11_N_SUPPORT
static void HTParametersHook(
	IN	PRTMP_ADAPTER pAd,
	IN	RTMP_STRING *pValueStr,
	IN	RTMP_STRING *pInput)
{
	long Value;
#ifdef CONFIG_AP_SUPPORT
	INT			i=0;
	RTMP_STRING *Bufptr;
#endif /* CONFIG_AP_SUPPORT */

    if (RTMPGetKeyParameter("HT_PROTECT", pValueStr, 25, pInput, TRUE))
    {
        Value = simple_strtol(pValueStr, 0, 10);
        if (Value == 0)
        {
            pAd->CommonCfg.bHTProtect = FALSE;
        }
        else
        {
            pAd->CommonCfg.bHTProtect = TRUE;
        }
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: Protection  = %s\n", (Value==0) ? "Disable" : "Enable"));
    }


    if (RTMPGetKeyParameter("HT_MIMOPSMode", pValueStr, 25, pInput, TRUE))
    {
        Value = simple_strtol(pValueStr, 0, 10);
        if (Value > MMPS_DISABLE)
        {
			pAd->CommonCfg.BACapability.field.MMPSmode = MMPS_DISABLE;
        }
        else
        {
            /*TODO: add mimo power saving mechanism*/
            pAd->CommonCfg.BACapability.field.MMPSmode = MMPS_DISABLE;
			/*pAd->CommonCfg.BACapability.field.MMPSmode = Value;*/
        }
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: MIMOPS Mode  = %d\n", (INT) Value));
    }

    if (RTMPGetKeyParameter("HT_BADecline", pValueStr, 25, pInput, TRUE))
    {
        Value = simple_strtol(pValueStr, 0, 10);
        if (Value == 0)
        {
            pAd->CommonCfg.bBADecline = FALSE;
        }
        else
        {
            pAd->CommonCfg.bBADecline = TRUE;
        }
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: BA Decline  = %s\n", (Value==0) ? "Disable" : "Enable"));
    }


    if (RTMPGetKeyParameter("HT_AutoBA", pValueStr, 25, pInput, TRUE))
    {
        Value = simple_strtol(pValueStr, 0, 10);
        if (Value == 0)
        {
            pAd->CommonCfg.BACapability.field.AutoBA = FALSE;
			pAd->CommonCfg.BACapability.field.Policy = BA_NOTUSE;
        }
        else
        {
            pAd->CommonCfg.BACapability.field.AutoBA = TRUE;
			pAd->CommonCfg.BACapability.field.Policy = IMMED_BA;
        }
        pAd->CommonCfg.REGBACapability.field.AutoBA = pAd->CommonCfg.BACapability.field.AutoBA;
		pAd->CommonCfg.REGBACapability.field.Policy = pAd->CommonCfg.BACapability.field.Policy;
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: Auto BA  = %s\n", (Value==0) ? "Disable" : "Enable"));
    }


	/* Reverse Direction Mechanism*/
    if (RTMPGetKeyParameter("HT_RDG", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);
		if (Value == 0)
			pAd->CommonCfg.bRdg = FALSE;
		else
            pAd->CommonCfg.bRdg = TRUE;

#ifdef MT_MAC
        if (pAd->chipCap.hif_type == HIF_MT) {
            if ((MTK_REV_GTE(pAd, MT7628, MT7628E2))
                    || (IS_MT7637(pAd)) 
                    || (IS_MT7615(pAd))
                    || (IS_MT7622(pAd)))
                ;
            else {
                //MT7628 E2 should could skip this operation.
                pAd->CommonCfg.bRdg = FALSE;
            }
        }
#endif

        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
                ("HT: RDG = %s\n", (Value==0) ? "Disable" : "Enable(+HTC)"));
    }




    /* Tx A-MSUD ?*/
    if (RTMPGetKeyParameter("HT_AMSDU", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);
		pAd->CommonCfg.BACapability.field.AmsduEnable = (Value == 0) ? FALSE : TRUE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: Tx A-MSDU = %s\n", (Value==0) ? "Disable" : "Enable"));
	}

#ifdef WFA_VHT_PF
	if (RTMPGetKeyParameter("FORCE_AMSDU", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);
		pAd->force_amsdu = (Value == 0) ? FALSE : TRUE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: FORCE A-MSDU = %s\n", (Value==0) ? "Disable" : "Enable"));
	}
#endif /* WFA_VHT_PF */

	/* MPDU Density*/
    if (RTMPGetKeyParameter("HT_MpduDensity", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);
		if (Value <=7 && Value >= 0)
		{
			pAd->CommonCfg.BACapability.field.MpduDensity = Value;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: MPDU Density = %d\n", (INT) Value));
		}
		else
		{
			pAd->CommonCfg.BACapability.field.MpduDensity = 4;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: MPDU Density = %d (Default)\n", 4));
		}
	}

	/* Max Rx BA Window Size*/
    if (RTMPGetKeyParameter("HT_BAWinSize", pValueStr, 25, pInput, TRUE))
	{
		RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value >= 1 && Value <= 64)
		{
			pAd->CommonCfg.REGBACapability.field.RxBAWinLimit = min((UINT8)Value, pChipCap->RxBAWinSize);
			pAd->CommonCfg.BACapability.field.RxBAWinLimit = min((UINT8)Value, pChipCap->RxBAWinSize);
#ifdef MT_MAC_BTCOEX
			pAd->CommonCfg.REGBACapability.field.TxBAWinLimit = min((UINT8)Value, pChipCap->TxBAWinSize);
			pAd->CommonCfg.BACapability.field.TxBAWinLimit = min((UINT8)Value, pChipCap->TxBAWinSize);
#endif /*MT_MAC_BTCOEX*/
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: BA Windw Size = %d\n", min((UINT8)Value, pChipCap->RxBAWinSize)));
		}
		else
		{
            pAd->CommonCfg.REGBACapability.field.RxBAWinLimit = min((UINT8)64, pChipCap->RxBAWinSize);
			pAd->CommonCfg.BACapability.field.RxBAWinLimit = min((UINT8)64, pChipCap->RxBAWinSize);
#ifdef MT_MAC_BTCOEX
           		pAd->CommonCfg.REGBACapability.field.TxBAWinLimit = min((UINT8)64, pChipCap->TxBAWinSize);
			pAd->CommonCfg.BACapability.field.TxBAWinLimit = min((UINT8)64, pChipCap->TxBAWinSize);
#endif /*MT_MAC_BTCOEX*/
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: BA Windw Size = %d\n", min((UINT8)64, pChipCap->RxBAWinSize)));
		}

	}

	/* Guard Interval*/
	if (RTMPGetKeyParameter("HT_GI", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value == GI_400)
		{
			pAd->CommonCfg.RegTransmitSetting.field.ShortGI = GI_400;
		}
		else
		{
			pAd->CommonCfg.RegTransmitSetting.field.ShortGI = GI_800;
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: Guard Interval = %s\n", (Value==GI_400) ? "400" : "800" ));
	}

	/* HT LDPC */
	if (RTMPGetKeyParameter("HT_LDPC", pValueStr, 25, pInput, TRUE))
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			for (i = 0, Bufptr = rstrtok(pValueStr,";"); (Bufptr && i < MAX_MBSSID_NUM(pAd)); Bufptr = rstrtok(NULL,";"), i++)
			{
				struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[i].wdev;

				Value = simple_strtol(Bufptr, 0, 10);
				if (Value == 0)
					wlan_config_set_ht_ldpc(wdev, FALSE);
				else
					wlan_config_set_ht_ldpc(wdev, TRUE);

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(IF-ra%d) HT: HT_LDPC = %d\n", i, wlan_config_get_ht_ldpc(wdev)));
			}
		}
#endif /* CONFIG_AP_SUPPORT */

	}

	/* HT Operation Mode : Mixed Mode , Green Field*/
	if (RTMPGetKeyParameter("HT_OpMode", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value == HTMODE_GF)
		{

			pAd->CommonCfg.RegTransmitSetting.field.HTMODE  = HTMODE_GF;
		}
		else
		{
			pAd->CommonCfg.RegTransmitSetting.field.HTMODE  = HTMODE_MM;
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: Operate Mode = %s\n", (Value==HTMODE_GF) ? "Green Field" : "Mixed Mode" ));
	}

	/* Fixed Tx mode : CCK, OFDM*/
	if (RTMPGetKeyParameter("FixedTxMode", pValueStr, 25, pInput, TRUE))
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			for (i = 0, Bufptr = rstrtok(pValueStr,";"); (Bufptr && i < MAX_MBSSID_NUM(pAd)); Bufptr = rstrtok(NULL,";"), i++)
			{
				pAd->ApCfg.MBSSID[i].wdev.DesiredTransmitSetting.field.FixedTxMode =
														RT_CfgSetFixedTxPhyMode(Bufptr);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(IF-ra%d) Fixed Tx Mode = %d\n", i,
											pAd->ApCfg.MBSSID[i].wdev.DesiredTransmitSetting.field.FixedTxMode));
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}


	/* Channel Width */
	if (RTMPGetKeyParameter("HT_BW", pValueStr, 25, pInput, TRUE))
	{
		UCHAR ht_bw;
		Value = simple_strtol(pValueStr, 0, 10);
		if (Value == BW_40)
			ht_bw = BW_40;
		else
			ht_bw = BW_20;

		wlan_config_set_ht_bw_all(&pAd->wpf,ht_bw);
#ifdef MCAST_RATE_SPECIFIC
		pAd->CommonCfg.MCastPhyMode.field.BW = ht_bw;
		pAd->CommonCfg.MCastPhyMode_5G.field.BW = ht_bw;
#endif /* MCAST_RATE_SPECIFIC */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: Channel Width = %s\n", (Value==BW_40) ? "40 MHz" : "20 MHz" ));
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd){
			struct wifi_dev *wdev;
			UCHAR ht_bw;
			for (i = 0, Bufptr = rstrtok(pValueStr,";"); (Bufptr && (i < MAX_MBSSID_NUM(pAd))); Bufptr = rstrtok(NULL,";"), i++) {
				Value = simple_strtol(Bufptr, 0, 10);
				wdev = &pAd->ApCfg.MBSSID[i].wdev;
				ht_bw = (Value == HT_BW_40 ) ?  HT_BW_40 : HT_BW_20;
				wlan_config_set_ht_bw(wdev,ht_bw);
			}
		}
#endif /*CONFIG_AP_SUPPORT*/
	}

	if (RTMPGetKeyParameter("HT_EXTCHA", pValueStr, 25, pInput, TRUE))
	{
		struct wifi_dev *wdev;
		UCHAR ext_cha;

#ifdef CONFIG_AP_SUPPORT
		for (i = 0, Bufptr = rstrtok(pValueStr,";"); (Bufptr && (i < MAX_MBSSID_NUM(pAd))); Bufptr = rstrtok(NULL,";"), i++) {
			Value = simple_strtol(Bufptr, 0, 10);

			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
				wdev = &pAd->ApCfg.MBSSID[i].wdev;
			}
			if (Value == 0)
				ext_cha = EXTCHA_BELOW;
			else
				ext_cha = EXTCHA_ABOVE;
			
			wlan_config_set_ext_cha(wdev,ext_cha);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HT: WDEV[%x] Ext Channel = %s\n", i,(Value==0) ? "BELOW" : "ABOVE" ));
		}

		ext_cha = wlan_config_get_ext_cha(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);
		for(i=0; i < pAd->ApCfg.BssidNum ; i++){
			wdev = &pAd->ApCfg.MBSSID[i].wdev;
			if(wlan_config_get_ext_cha(wdev)==EXTCHA_NOASSIGN){
#ifdef WH_EZ_SETUP
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HT: WDEV[%x] Ext Channel = %x\n", i,ext_cha));
#endif
				wlan_config_set_ext_cha(wdev,ext_cha);
			}
		}
#endif /*CONFIG_AP_SUPPORT*/
	}

	/* MSC*/
	if (RTMPGetKeyParameter("HT_MCS", pValueStr, 50, pInput, TRUE))
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			for (i = 0, Bufptr = rstrtok(pValueStr,";"); (Bufptr && i < MAX_MBSSID_NUM(pAd)); Bufptr = rstrtok(NULL,";"), i++)
			{
				struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[i].wdev;
				Value = simple_strtol(Bufptr, 0, 10);
				if (Value >= MCS_0 && Value <= MCS_32)
					wdev->DesiredTransmitSetting.field.MCS = Value;
				else
					wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(IF-ra%d) HT: MCS = %s(%d)\n", i,
						(wdev->DesiredTransmitSetting.field.MCS == MCS_AUTO ? "AUTO" : "Fixed"),
						wdev->DesiredTransmitSetting.field.MCS));
			}
		}
#endif /* CONFIG_AP_SUPPORT */

	}

	/* STBC */
    if (RTMPGetKeyParameter("HT_STBC", pValueStr, 25, pInput, TRUE))
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			for (i = 0, Bufptr = rstrtok(pValueStr,";"); (Bufptr && i < MAX_MBSSID_NUM(pAd)); Bufptr = rstrtok(NULL,";"), i++)
			{
				struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[i].wdev;

				Value = simple_strtol(Bufptr, 0, 10);
				if (Value == STBC_NONE)
					wlan_config_set_ht_stbc(wdev, STBC_NONE);
				else
					wlan_config_set_ht_stbc(wdev, STBC_USE);

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(IF-ra%d) HT: HT_STBC = %d\n", i, wlan_config_get_ht_stbc(wdev)));
			}
		}
#endif /* CONFIG_AP_SUPPORT */

	}

	/* 40_Mhz_Intolerant*/
	if (RTMPGetKeyParameter("HT_40MHZ_INTOLERANT", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);
		if (Value == 0)
		{
			pAd->CommonCfg.bForty_Mhz_Intolerant = FALSE;
		}
		else
		{
			pAd->CommonCfg.bForty_Mhz_Intolerant = TRUE;
		}
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: 40MHZ INTOLERANT = %d\n", pAd->CommonCfg.bForty_Mhz_Intolerant));
	}
	/*HT_TxStream*/
	if(RTMPGetKeyParameter("HT_TxStream", pValueStr, 10, pInput, TRUE))
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			for (i = 0, Bufptr = rstrtok(pValueStr,";"); (Bufptr && i < MAX_MBSSID_NUM(pAd)); Bufptr = rstrtok(NULL,";"), i++)
			{
				struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[i].wdev;

				Value = simple_strtol(Bufptr, 0, 10);
                if (Value <= 4)
                    wlan_config_set_tx_stream(wdev, Value);
                else
                    wlan_config_set_tx_stream(wdev, 1);

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(IF-ra%d) HT: Tx Stream = %d\n", i, wlan_config_get_tx_stream(wdev)));
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}
	/*HT_RxStream*/
	if(RTMPGetKeyParameter("HT_RxStream", pValueStr, 10, pInput, TRUE))
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			for (i = 0, Bufptr = rstrtok(pValueStr,";"); (Bufptr && i < MAX_MBSSID_NUM(pAd)); Bufptr = rstrtok(NULL,";"), i++)
			{
				struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[i].wdev;

				Value = simple_strtol(Bufptr, 0, 10);
                if (Value <= 4)
                    wlan_config_set_rx_stream(wdev, Value);
                else
                    wlan_config_set_rx_stream(wdev, 1);

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(IF-ra%d) HT: Rx Stream = %d\n", i, wlan_config_get_rx_stream(wdev)));
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}
#ifdef GREENAP_SUPPORT
	/*Green AP*/
	if(RTMPGetKeyParameter("GreenAP", pValueStr, 10, pInput, TRUE))
	{
            struct greenap_ctrl *greenap = &pAd->ApCfg.greenap;

		Value = simple_strtol(pValueStr, 0, 10);
		if (Value == 0) {
			greenap_set_capability(greenap, FALSE);
		}
		else {
			greenap_set_capability(greenap, TRUE);
		}
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HT: greenap_cap = %d\n", greenap_get_capability(greenap)));
	}
#endif /* GREENAP_SUPPORT */
	/* HT_DisallowTKIP*/
	if (RTMPGetKeyParameter("HT_DisallowTKIP", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value == 1)
		{
			pAd->CommonCfg.HT_DisallowTKIP = TRUE;
		}
		else
		{
			pAd->CommonCfg.HT_DisallowTKIP = FALSE;
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: Disallow TKIP mode = %s\n", (pAd->CommonCfg.HT_DisallowTKIP == TRUE) ? "ON" : "OFF" ));
	}

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
			if (RTMPGetKeyParameter("OBSSScanParam", pValueStr, 32, pInput, TRUE))
			{
				int ObssScanValue, idx;
				RTMP_STRING *macptr;
				for (idx = 0, macptr = rstrtok(pValueStr,";"); macptr; macptr = rstrtok(NULL,";"), idx++)
				{
					ObssScanValue = simple_strtol(macptr, 0, 10);
					switch (idx)
					{
						case 0:
							if (ObssScanValue < 5 || ObssScanValue > 1000)
							{
								MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Invalid OBSSScanParam for Dot11OBssScanPassiveDwell(%d), should in range 5~1000\n", ObssScanValue));
							}
							else
							{
								pAd->CommonCfg.Dot11OBssScanPassiveDwell = ObssScanValue;	/* Unit : TU. 5~1000*/
								MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OBSSScanParam for Dot11OBssScanPassiveDwell=%d\n", ObssScanValue));
							}
							break;
						case 1:
							if (ObssScanValue < 10 || ObssScanValue > 1000)
							{
								MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Invalid OBSSScanParam for Dot11OBssScanActiveDwell(%d), should in range 10~1000\n", ObssScanValue));
							}
							else
							{
								pAd->CommonCfg.Dot11OBssScanActiveDwell = ObssScanValue;	/* Unit : TU. 10~1000*/
								MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OBSSScanParam for Dot11OBssScanActiveDwell=%d\n", ObssScanValue));
							}
							break;
						case 2:
							pAd->CommonCfg.Dot11BssWidthTriggerScanInt = ObssScanValue;	/* Unit : Second*/
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OBSSScanParam for Dot11BssWidthTriggerScanInt=%d\n", ObssScanValue));
							break;
						case 3:
							if (ObssScanValue < 200 || ObssScanValue > 10000)
							{
								MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Invalid OBSSScanParam for Dot11OBssScanPassiveTotalPerChannel(%d), should in range 200~10000\n", ObssScanValue));
							}
							else
							{
								pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = ObssScanValue;	/* Unit : TU. 200~10000*/
								MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OBSSScanParam for Dot11OBssScanPassiveTotalPerChannel=%d\n", ObssScanValue));
							}
							break;
						case 4:
							if (ObssScanValue < 20 || ObssScanValue > 10000)
							{
								MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Invalid OBSSScanParam for Dot11OBssScanActiveTotalPerChannel(%d), should in range 20~10000\n", ObssScanValue));
							}
							else
							{
								pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = ObssScanValue;	/* Unit : TU. 20~10000*/
								MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OBSSScanParam for Dot11OBssScanActiveTotalPerChannel=%d\n", ObssScanValue));
							}
							break;
						case 5:
							pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor = ObssScanValue;
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OBSSScanParam for Dot11BssWidthChanTranDelayFactor=%d\n", ObssScanValue));
							break;
						case 6:
							pAd->CommonCfg.Dot11OBssScanActivityThre = ObssScanValue;	/* Unit : percentage*/
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OBSSScanParam for Dot11BssWidthChanTranDelayFactor=%d\n", ObssScanValue));
							break;
					}
				}

				if (idx != 7)
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Wrong OBSSScanParamtetrs format in dat file!!!!! Use default value.\n"));

					pAd->CommonCfg.Dot11OBssScanPassiveDwell = dot11OBSSScanPassiveDwell;	/* Unit : TU. 5~1000*/
					pAd->CommonCfg.Dot11OBssScanActiveDwell = dot11OBSSScanActiveDwell;	/* Unit : TU. 10~1000*/
					pAd->CommonCfg.Dot11BssWidthTriggerScanInt = dot11BSSWidthTriggerScanInterval;	/* Unit : Second	*/
					pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = dot11OBSSScanPassiveTotalPerChannel;	/* Unit : TU. 200~10000*/
					pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = dot11OBSSScanActiveTotalPerChannel;	/* Unit : TU. 20~10000*/
					pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor = dot11BSSWidthChannelTransactionDelayFactor;
					pAd->CommonCfg.Dot11OBssScanActivityThre = dot11BSSScanActivityThreshold;	/* Unit : percentage*/
				}
				pAd->CommonCfg.Dot11BssWidthChanTranDelay = (pAd->CommonCfg.Dot11BssWidthTriggerScanInt * pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor);
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OBSSScanParam for Dot11BssWidthChanTranDelay=%ld\n", pAd->CommonCfg.Dot11BssWidthChanTranDelay));
			}

			if (RTMPGetKeyParameter("HT_BSSCoexistence", pValueStr, 25, pInput, TRUE))
			{
				Value = simple_strtol(pValueStr, 0, 10);
				pAd->CommonCfg.bBssCoexEnable = ((Value == 1) ? TRUE : FALSE);

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: 20/40 BssCoexSupport = %s\n", (pAd->CommonCfg.bBssCoexEnable == TRUE) ? "ON" : "OFF" ));
			}


			if (RTMPGetKeyParameter("HT_BSSCoexApCntThr", pValueStr, 25, pInput, TRUE))
			{
				pAd->CommonCfg.BssCoexApCntThr = simple_strtol(pValueStr, 0, 10);;

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: 20/40 BssCoexApCntThr = %d\n", pAd->CommonCfg.BssCoexApCntThr));
			}

#endif /* DOT11N_DRAFT3 */

	if (RTMPGetKeyParameter("BurstMode", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);
		pAd->CommonCfg.bRalinkBurstMode = ((Value == 1) ? 1 : 0);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT: RaBurstMode= %d\n", pAd->CommonCfg.bRalinkBurstMode));
	}
#endif /* DOT11_N_SUPPORT */

    if (RTMPGetKeyParameter("TXRX_RXV_ON", pValueStr, 25, pInput, TRUE)) {

		Value = simple_strtol(pValueStr, 0, 10);
        pAd->CommonCfg.bTXRX_RXV_ON = Value;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("TXRX_RXV_ON = %s\n", (Value == 1) ? "ON" : "OFF" ));
    }
}
#endif /* DOT11_N_SUPPORT */




void RTMPSetCountryCode(RTMP_ADAPTER *pAd, RTMP_STRING *CountryCode)
{
	NdisMoveMemory(pAd->CommonCfg.CountryCode, CountryCode , 2);
	pAd->CommonCfg.CountryCode[2] = ' ';
	if (strlen((RTMP_STRING *) pAd->CommonCfg.CountryCode) != 0)
		pAd->CommonCfg.bCountryFlag = TRUE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CountryCode=%s\n", pAd->CommonCfg.CountryCode));
}


NDIS_STATUS	RTMPSetProfileParameters(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *pBuffer)
{
	RTMP_STRING *tmpbuf;
	RTMP_STRING *macptr = NULL;
	INT		i = 0, retval;
    CHAR    *value = 0;

#ifdef CONFIG_AP_SUPPORT
    RTMP_STRING tok_str[16];
    UCHAR BssidCountSupposed = 0;
    BOOLEAN bSSIDxIsUsed = FALSE;
#endif

	os_alloc_mem(NULL, (UCHAR **)&tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if(tmpbuf == NULL)
		return NDIS_STATUS_FAILURE;

	do
	{
#ifdef CONFIG_AP_SUPPORT
        IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
        {
#ifdef MBSS_SUPPORT
            /*BSSIDNum; This must read first of other multiSSID field, so list this field first in configuration file*/
            if(RTMPGetKeyParameter("BssidNum", tmpbuf, 25, pBuffer, TRUE))
            {
                pAd->ApCfg.BssidNum = (UCHAR) simple_strtol(tmpbuf, 0, 10);
                if(pAd->ApCfg.BssidNum > MAX_MBSSID_NUM(pAd))
                {
                    pAd->ApCfg.BssidNum = MAX_MBSSID_NUM(pAd);
                    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
                            ("BssidNum=%d(MAX_MBSSID_NUM is %d)\n",
                                pAd->ApCfg.BssidNum,MAX_MBSSID_NUM(pAd)));
                }
                else
                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BssidNum=%d\n", pAd->ApCfg.BssidNum));
            }

#if defined (RLT_MAC) || defined (RTMP_MAC)
            if (pAd->ApCfg.BssidNum != 0)
            {
                if (HW_BEACON_OFFSET > (HW_BEACON_MAX_SIZE(pAd) / pAd->ApCfg.BssidNum))
                {
                    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("mbss> fatal error! beacon offset is error in driver! "
                            "Please re-assign HW_BEACON_OFFSET!\n"));
                }
            }
#endif /*RLT_MAC || RTMP_MAC*/

#else
            pAd->ApCfg.BssidNum = 1;
#endif /* MBSS_SUPPORT */
        }
#endif /* CONFIG_AP_SUPPORT */


		/* set file parameter to portcfg*/
		if (RTMPGetKeyParameter("MacAddress", tmpbuf, 25, pBuffer, TRUE))
		{
			retval = RT_CfgSetMacAddress(pAd, tmpbuf,0);
			if (retval)
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MacAddress = %02x:%02x:%02x:%02x:%02x:%02x\n",
											PRINT_MAC(pAd->CurrentAddress)));
		}
#ifdef MT_MAC
#ifdef CONFIG_AP_SUPPORT
        IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
        {
#ifdef MBSS_SUPPORT
            if (IS_MT7615(pAd) || (IS_MT7637(pAd)) || IS_MT7622(pAd))
            {
                /* for MT7615, we could assign extend BSSID mac address by ourself. */
                /* extend index starts from 1.*/
                for (i = 1; i < pAd->ApCfg.BssidNum; i++)
                {
                    snprintf(tok_str, sizeof(tok_str), "MacAddress%d", i);
                    if(RTMPGetKeyParameter(tok_str, tmpbuf, 25, pBuffer, TRUE))
                    {
                        retval = RT_CfgSetMacAddress(pAd, tmpbuf, i);
                        if (retval)
                            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MacAddress%d = %02x:%02x:%02x:%02x:%02x:%02x\n",
                                            i, PRINT_MAC(pAd->ExtendMBssAddr[i])));
                    }
                }
            }
#endif /* MBSS_SUPPORT */
        }
#endif /*CONFIG_AP_SUPPORT*/
#endif /*MT_MAC*/

		/*CountryRegion*/
		if(RTMPGetKeyParameter("CountryRegion", tmpbuf, 25, pBuffer, TRUE))
		{
			retval = RT_CfgSetCountryRegion(pAd, tmpbuf, BAND_24G);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CountryRegion=%d\n", pAd->CommonCfg.CountryRegion));
		}
		/*CountryRegionABand*/
		if(RTMPGetKeyParameter("CountryRegionABand", tmpbuf, 25, pBuffer, TRUE))
		{
			retval = RT_CfgSetCountryRegion(pAd, tmpbuf, BAND_5G);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CountryRegionABand=%d\n", pAd->CommonCfg.CountryRegionForABand));
		}

#ifdef BB_SOC
#ifdef RTMP_EFUSE_SUPPORT
		/*EfuseBufferMode*/
		if(RTMPGetKeyParameter("EfuseBufferMode", tmpbuf, 25, pBuffer, TRUE))
		{
			pAd->E2pAccessMode = ((UCHAR) simple_strtol(tmpbuf, 0, 10) == 1)? 4: (UCHAR) simple_strtol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("EfuseBufferMode=%d\n", pAd->E2pAccessMode));
		}
#endif /* RTMP_EFUSE_SUPPORT */
#endif /* BB_SOC */

		/* E2pAccessMode */
		if (RTMPGetKeyParameter("E2pAccessMode", tmpbuf, 25, pBuffer, TRUE))
		{
			pAd->E2pAccessMode = (UCHAR) simple_strtol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("E2pAccessMode=%d\n", pAd->E2pAccessMode));
		}

		/*CountryCode*/
		if (pAd->CommonCfg.bCountryFlag == 0)
		{
		if(RTMPGetKeyParameter("CountryCode", tmpbuf, 25, pBuffer, TRUE))
			RTMPSetCountryCode(pAd, tmpbuf);
		}

#ifdef EXT_BUILD_CHANNEL_LIST
		/*ChannelGeography*/
		if(RTMPGetKeyParameter("ChannelGeography", tmpbuf, 25, pBuffer, TRUE))
		{
			UCHAR Geography = (UCHAR) simple_strtol(tmpbuf, 0, 10);
			if (Geography <= BOTH)
			{
				pAd->CommonCfg.Geography = Geography;
				pAd->CommonCfg.CountryCode[2] =
					(pAd->CommonCfg.Geography == BOTH) ? ' ' : ((pAd->CommonCfg.Geography == IDOR) ? 'I' : 'O');
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ChannelGeography=%d\n", pAd->CommonCfg.Geography));
			}
		}
		else
		{
			pAd->CommonCfg.Geography = BOTH;
			pAd->CommonCfg.CountryCode[2] = ' ';
		}
#endif /* EXT_BUILD_CHANNEL_LIST */

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			/* SSID*/
			if (TRUE)
			{
				//PRINT(DBG_LVL_TRACE, ("pAd->ApCfg.BssidNum=%d\n", pAd->ApCfg.BssidNum));
				for (i = 0; i < pAd->ApCfg.BssidNum; i++)
				{
					snprintf(tok_str, sizeof(tok_str), "SSID%d", i + 1);
					if(RTMPGetKeyParameter(tok_str, tmpbuf, 33, pBuffer, FALSE))
					{
						macptr = rstrtok(tmpbuf,";");
						if (macptr == NULL)
							continue; /* SSID(%i+1) is empty */
						NdisMoveMemory(pAd->ApCfg.MBSSID[i].Ssid, macptr , strlen(tmpbuf));
				    	pAd->ApCfg.MBSSID[i].Ssid[strlen(macptr)] = '\0';
						pAd->ApCfg.MBSSID[i].SsidLen = strlen((RTMP_STRING *) pAd->ApCfg.MBSSID[i].Ssid);
						if (bSSIDxIsUsed == FALSE)
						{
							bSSIDxIsUsed = TRUE;
						}
				    	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("SSID[%d]=%s, EdcaIdx=%d\n", i, pAd->ApCfg.MBSSID[i].Ssid,
							pAd->ApCfg.MBSSID[i].wdev.EdcaIdx));
					}
				}
				if (bSSIDxIsUsed == FALSE)
				{
					if(RTMPGetKeyParameter("SSID", tmpbuf, 256, pBuffer, FALSE))
					{
						BssidCountSupposed = delimitcnt(tmpbuf, ";") + 1;
						if (pAd->ApCfg.BssidNum != BssidCountSupposed)
						{
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Your no. of SSIDs( = %d) does not match your BssidNum( = %d)!\n", BssidCountSupposed, pAd->ApCfg.BssidNum));
						}
						if (pAd->ApCfg.BssidNum > 1)
						{
							/* Anyway, we still do the legacy dissection of the whole SSID string.*/
							for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
							{
								int apidx = 0;

								if (i < pAd->ApCfg.BssidNum)
								{
									apidx = i;
								}
								else
								{
									break;
								}

								NdisMoveMemory(pAd->ApCfg.MBSSID[apidx].Ssid, macptr , strlen(macptr));
				    			pAd->ApCfg.MBSSID[apidx].Ssid[strlen(macptr)] = '\0';
							   pAd->ApCfg.MBSSID[apidx].SsidLen = strlen((RTMP_STRING *)pAd->ApCfg.MBSSID[apidx].Ssid);

				    			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("SSID[%d]=%s\n", i, pAd->ApCfg.MBSSID[apidx].Ssid));
							}
						}
						else
						{
							if ((strlen(tmpbuf) > 0) && (strlen(tmpbuf) <= 32))
							{
								NdisMoveMemory(pAd->ApCfg.MBSSID[BSS0].Ssid, tmpbuf , strlen(tmpbuf));
						    	pAd->ApCfg.MBSSID[BSS0].Ssid[strlen(tmpbuf)] = '\0';
									    	pAd->ApCfg.MBSSID[BSS0].SsidLen = strlen((RTMP_STRING *) pAd->ApCfg.MBSSID[BSS0].Ssid);
								MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("SSID=%s\n", pAd->ApCfg.MBSSID[BSS0].Ssid));
							}
						}
					}
				}

				if(RTMPGetKeyParameter("EdcaIdx", tmpbuf, 256, pBuffer, FALSE))
				{
					UCHAR edca_idx =0;
					for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
					{
						if(i < pAd->ApCfg.BssidNum){
							edca_idx = simple_strtol(macptr, 0, 10);
							pAd->ApCfg.MBSSID[i].wdev.EdcaIdx = edca_idx;
						}
					}
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */



#ifdef DBDC_MODE
		/*Note: must be put before WirelessMode/Channel for check phy mode*/
		if (RTMPGetKeyParameter("DBDC_MODE", tmpbuf, 25, pBuffer, TRUE)) {
			ULONG dbdc_mode = simple_strtol(tmpbuf, 0, 10);

			pAd->CommonCfg.dbdc_mode = dbdc_mode > 0 ? TRUE : FALSE;

			/*
				TODO
				For DBDC mode, currently cannot use this wf_fwd function!
			*/

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("%s(): DBDC Mode=%d\n",
						__FUNCTION__, pAd->CommonCfg.dbdc_mode));
		}
#endif /* DBDC_MODE */

#ifdef MT_DFS_SUPPORT
		if (RTMPGetKeyParameter("DfsCalibration", tmpbuf, 25, pBuffer, TRUE))
		{
			UINT_32 DisableDfsCal = simple_strtol(tmpbuf, 0, 10);
			pAd->DfsParameter.DisableDfsCal = DisableDfsCal;
		}
		if(RTMPGetKeyParameter("DfsEnable", tmpbuf, 25, pBuffer, TRUE))
		{
			UINT_32 DfsEnable = simple_strtol(tmpbuf, 0, 10);
			pAd->DfsParameter.bDfsEnable = DfsEnable;
        	}
		if (RTMPGetKeyParameter("DfsTPDutyRatio", tmpbuf, 25, pBuffer, TRUE))			
		{	UCHAR DfsTPDutyRatio = (UCHAR) simple_strtol(tmpbuf, 0, 10);
			pAd->DfsParameter.TPDutyRatio = DfsTPDutyRatio;
		}		
#endif
		/* Channel Group */
		if(RTMPGetKeyParameter("ChannelGrp", tmpbuf, 25, pBuffer, TRUE))
		{
			MTSetChGrp(pAd, tmpbuf);
		}


		/*Channel*/
		if(RTMPGetKeyParameter("Channel", tmpbuf, 100, pBuffer, TRUE))
		{
			RTMPChannelCfg(pAd,tmpbuf);
		}

		/* EtherTrafficBand */
		if (RTMPGetKeyParameter("EtherTrafficBand", tmpbuf, 10, pBuffer, TRUE))
		{
			pAd->CommonCfg.EtherTrafficBand = (UCHAR) simple_strtol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("EtherTrafficBand=%d\n", pAd->CommonCfg.EtherTrafficBand));
			
			if (pAd->CommonCfg.EtherTrafficBand > EtherTrafficBand5G)
				pAd->CommonCfg.EtherTrafficBand = EtherTrafficBand5G;
		}

		/* Wf_fwd_ */
		if (RTMPGetKeyParameter("WfFwdDisabled", tmpbuf, 10, pBuffer, TRUE))
		{	
			pAd->CommonCfg.WfFwdDisabled = simple_strtol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WfFwdDisabled=%d\n", pAd->CommonCfg.WfFwdDisabled));
		}

		/*WirelessMode*/
		/*Note: BssidNum must be put before WirelessMode in dat file*/
		if(RTMPGetKeyParameter("WirelessMode", tmpbuf, 100, pBuffer, TRUE))
		{
			RTMPWirelessModeCfg(pAd,tmpbuf);
		}

	    /*BasicRate*/
		if(RTMPGetKeyParameter("BasicRate", tmpbuf, 10, pBuffer, TRUE))
		{
			pAd->CommonCfg.BasicRateBitmap = (ULONG) simple_strtol(tmpbuf, 0, 10);
			pAd->CommonCfg.BasicRateBitmapOld = (ULONG) simple_strtol(tmpbuf, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BasicRate=%ld\n", pAd->CommonCfg.BasicRateBitmap));
		}
		/*BeaconPeriod*/
		if(RTMPGetKeyParameter("BeaconPeriod", tmpbuf, 10, pBuffer, TRUE))
		{
			USHORT bcn_val = (USHORT) simple_strtol(tmpbuf, 0, 10);

			/* The acceptable is 20~1000 ms. Refer to WiFi test plan. */
			if (bcn_val >= 20 && bcn_val <= 1000)
				pAd->CommonCfg.BeaconPeriod = bcn_val;
			else
				pAd->CommonCfg.BeaconPeriod = 100;	/* Default value*/
#ifdef APCLI_CONNECTION_TRIAL
			pAd->CommonCfg.BeaconPeriod = 200;
#endif /* APCLI_CONNECTION_TRIAL */
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BeaconPeriod=%d\n", pAd->CommonCfg.BeaconPeriod));
		}


#ifdef DOT11V_WNM_SUPPORT
		WNM_ReadParametersFromFile(pAd, tmpbuf, pBuffer);
#endif /* DOT11V_WNM_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef DFS_SUPPORT
	/*DFSIndoor*/
	{
		PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
		PDFS_PROGRAM_PARAM pDfsProgramParam = &pRadarDetect->DfsProgramParam;

		if (RTMPGetKeyParameter("DfsIndoor", tmpbuf, 10, pBuffer, TRUE))
		{
			pAd->Dot11_H.bDFSIndoor = (USHORT) (simple_strtol(tmpbuf, 0, 10) != 0);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DfsIndoor=%d\n", pAd->Dot11_H.bDFSIndoor));
		}
		{
			INT k=0;
		/*SymRoundFromCfg*/
	            if (RTMPGetKeyParameter("SymRoundFromCfg", tmpbuf, 10, pBuffer, TRUE))
	            {
		                pRadarDetect->SymRoundFromCfg = (UCHAR) simple_strtol(tmpbuf, 0, 10);
	                pRadarDetect->SymRoundCfgValid = 1;
	                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("SymRoundFromCfg=%d\n", pRadarDetect->SymRoundFromCfg));
	            }

	            /*BusyIdleFromCfg*/
	            if (RTMPGetKeyParameter("BusyIdleFromCfg", tmpbuf, 10, pBuffer, TRUE))
	            {
	                pRadarDetect->BusyIdleFromCfg = (UCHAR) simple_strtol(tmpbuf, 0, 10);
	                pRadarDetect->BusyIdleCfgValid = 1;
	                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BusyIdleFromCfg=%d\n", pRadarDetect->BusyIdleFromCfg));
	            }
	            /*DfsRssiHighFromCfg*/
	            if (RTMPGetKeyParameter("DfsRssiHighFromCfg", tmpbuf, 10, pBuffer, TRUE))
	            {
	                pRadarDetect->DfsRssiHighFromCfg = simple_strtol(tmpbuf, 0, 10);
	                pRadarDetect->DfsRssiHighCfgValid = 1;
	                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DfsRssiHighFromCfg=%d\n", pRadarDetect->DfsRssiHighFromCfg));
	            }

	            /*DfsRssiLowFromCfg*/
	            if (RTMPGetKeyParameter("DfsRssiLowFromCfg", tmpbuf, 10, pBuffer, TRUE))
	            {
	                pRadarDetect->DfsRssiLowFromCfg = simple_strtol(tmpbuf, 0, 10);
	                pRadarDetect->DfsRssiLowCfgValid = 1;
	                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DfsRssiLowFromCfg=%d\n", pRadarDetect->DfsRssiLowFromCfg));
	            }

		/*DFSParamFromConfig*/
		 if (RTMPGetKeyParameter("DFSParamFromConfig", tmpbuf, 10, pBuffer, TRUE))
		 {
				pRadarDetect->DFSParamFromConfig = (UCHAR) simple_strtol(tmpbuf, 0, 10);

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DFSParamFromConfig=%d\n", pRadarDetect->DFSParamFromConfig));
		 }

		/* DFSParam*/
			for(k = 0; k < 4*pAd->chipCap.DfsEngineNum; k++)
		{
			RTMP_STRING tok_str[32];
			INT index ;
				UINT8 DfsEngineNum = pAd->chipCap.DfsEngineNum;
				index = (k%DfsEngineNum);
				if (((k-k%DfsEngineNum)/DfsEngineNum) == 0)
				snprintf(tok_str, sizeof(tok_str), "FCCParamCh%d", index);
				else if (((k-k%DfsEngineNum)/DfsEngineNum) == 1)
				snprintf(tok_str, sizeof(tok_str), "CEParamCh%d", index);
				else if (((k-k%DfsEngineNum)/DfsEngineNum) == 2)
				snprintf(tok_str, sizeof(tok_str), "JAPParamCh%d", index);
				else if (((k-k%DfsEngineNum)/DfsEngineNum) == 3)
				snprintf(tok_str, sizeof(tok_str), "JAPW53ParamCh%d", index);

			if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, pBuffer, TRUE))
			{
				ULONG DfsParam;
		    		for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		    		{
		    			DfsParam = simple_strtol(macptr, 0, 10);
		    			switch (i)
		    			{
		    				case 0:
		    					pDfsProgramParam->NewDFSTableEntry[k].mode = DfsParam;
		    					break;
		    				case 1:
		    					pDfsProgramParam->NewDFSTableEntry[k].avgLen = DfsParam;
								pDfsProgramParam->NewDFSTableEntry[k].valid = 1;
		    					break;
		    		 		case 2:
								pDfsProgramParam->NewDFSTableEntry[k].ELow = DfsParam;
		    					break;
		    				case 3:
								pDfsProgramParam->NewDFSTableEntry[k].EHigh = DfsParam;
		    					break;
		    				case 4:
								pDfsProgramParam->NewDFSTableEntry[k].WLow = DfsParam;
		    					break;
		    				case 5:
								pDfsProgramParam->NewDFSTableEntry[k].WHigh = DfsParam;
		    					break;
		    				case 6:
								pDfsProgramParam->NewDFSTableEntry[k].EpsilonW = DfsParam;
		    					break;
		    				case 7:
								pDfsProgramParam->NewDFSTableEntry[k].TLow = DfsParam;
		    					break;
		    				case 8:
								pDfsProgramParam->NewDFSTableEntry[k].THigh = DfsParam;
		    					break;
		    				case 9:
								pDfsProgramParam->NewDFSTableEntry[k].EpsilonT = DfsParam;
		    					break;

							case 10:
								pDfsProgramParam->NewDFSTableEntry[k].BLow = DfsParam;
		    					break;
							case 11:
								pDfsProgramParam->NewDFSTableEntry[k].BHigh = DfsParam;
		    					break;

		    				default:
		    					break;
		    			}
		    		}
			}
		}
		}
	}
#endif /* DFS_SUPPORT */
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			/*DtimPeriod*/
			if(RTMPGetKeyParameter("DtimPeriod", tmpbuf, 10, pBuffer, TRUE))
			{
				pAd->ApCfg.DtimPeriod = (UCHAR) simple_strtol(tmpbuf, 0, 10);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DtimPeriod=%d\n", pAd->ApCfg.DtimPeriod));
			}
#ifdef RADIO_LINK_SELECTION
			if(RTMPGetKeyParameter("RadioLinkSelection", tmpbuf, 10, pBuffer, TRUE))
			{
				pAd->ApCfg.RadioLinkSelection = (UCHAR) simple_strtol(tmpbuf, 0, 10);
				MTWF_LOG(DBG_CAT_CFG, DBG_CAT_AP, DBG_LVL_TRACE, ("RadioLinkSelection=%d\n", pAd->ApCfg.RadioLinkSelection));
			}
#endif /* RADIO_LINK_SELECTION */
#ifdef BAND_STEERING
			// Read BandSteering profile parameters
			BndStrgSetProfileParam(pAd,tmpbuf,pBuffer);
#endif /* BAND_STEERING */
#ifdef GPIO_CONTROL_SUPPORT
			if(RTMPGetKeyParameter("GPIOOutputPin", tmpbuf, 30, pBuffer, TRUE))
			{
				for (i = 0, macptr = rstrtok(tmpbuf,";");macptr && (i < MAX_GPIO_AVAILABLE); macptr = rstrtok(NULL,";"), i++)
				{
					pAd->ApCfg.GPIOOutputPin[i] = simple_strtoul(macptr,0,10);
					MTWF_LOG(DBG_CAT_CFG, DBG_CAT_AP, DBG_LVL_ERROR,("%s:GPIO%u -->Output\n",__FUNCTION__,pAd->ApCfg.GPIOOutputPin[i]));
				}
				pAd->ApCfg.NoOfGPIOOutput = i;
			}
			if(RTMPGetKeyParameter("GPIOOutputData", tmpbuf, 30, pBuffer, TRUE))
			{
				for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr && (i< pAd->ApCfg.NoOfGPIOOutput); macptr = rstrtok(NULL,";"), i++)
				{
					pAd->ApCfg.GPIOOutputData[i] = simple_strtoul(macptr,0,10);
					MTWF_LOG(DBG_CAT_CFG, DBG_CAT_AP, DBG_LVL_ERROR,("%s:%u -->Output data\n",__FUNCTION__,pAd->ApCfg.GPIOOutputData[i]));
				}
				pAd->ApCfg.NoOfGPIOOutput = i;
			}
#endif /* GPIO_CONTROL_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */

        
		/* TxPower */
		if(RTMPGetKeyParameter("TxPower", tmpbuf, 10, pBuffer, TRUE))
		{
            /* parameter parsing */
            for (i = BAND0, value = rstrtok(tmpbuf,";"); value; value = rstrtok(NULL,";"), i++)
            {
#ifdef DBDC_MODE
                if (pAd->CommonCfg.dbdc_mode)
                {
                    switch (i)
                    { 
                        case 0:
                            pAd->CommonCfg.TxPowerPercentage[BAND1] = simple_strtol(value, 0, 10);
                            break;

                        case 1:
                            pAd->CommonCfg.TxPowerPercentage[BAND0] = simple_strtol(value, 0, 10);
                            break;    

                        default:
                            break;
                    }
                }
                else
                {
                    switch (i)
                    { 
                        case 0:
                            pAd->CommonCfg.TxPowerPercentage[BAND0] = simple_strtol(value, 0, 10);
                            break;   

                        default:
                            break;
                    }
                }
#else
                switch (i)
                {
                    case 0:
                        pAd->CommonCfg.TxPowerPercentage[BAND0] = simple_strtol(value, 0, 10);
                        break;   

                    default:
                        break;
                }
#endif /* DBDC_MODE */
            }

#ifdef DBDC_MODE
            if (pAd->CommonCfg.dbdc_mode)
                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[TxPower] BAND0: %ld, BAND1: %ld \n", pAd->CommonCfg.TxPowerPercentage[BAND0], pAd->CommonCfg.TxPowerPercentage[BAND1]));
            else
                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[TxPower] BAND0: %ld \n", pAd->CommonCfg.TxPowerPercentage[BAND0]));
#else
            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[TxPower] BAND0: %ld \n", pAd->CommonCfg.TxPowerPercentage[BAND0]));
#endif /* DBDC_MODE */      

		}

#ifdef TX_POWER_CONTROL_SUPPORT
		/* Power Boost (CCK, OFDM) */
        if(RTMPGetKeyParameter("PowerUpCckOfdm", tmpbuf, 32, pBuffer, TRUE))
        {
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode)
			{
				RTMP_STRING *ptmpStr[DBDC_BAND_NUM];

				/* parameter parsing (Phase I) */
	            for (i = 0, value = rstrtok(tmpbuf,";"); value; value = rstrtok(NULL,";"), i++)
					ptmpStr[i] = value;

				/* sanity check for paramter parsing (Phase I) */
				if (i != DBDC_BAND_NUM)
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PowerUpCckOfdm] Input parameter incorrect!!\n"));

				/* Band1 Parameter parsing (Phase II) */
				value = ptmpStr[0];
				for (i = 0, value = rstrtok(value,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
					pAd->CommonCfg.cPowerUpCckOfdm[BAND1][i] = simple_strtol(value, 0, 10);

				/* Band0 Parameter parsing (Phase II) */
				value = ptmpStr[1];
				for (i = 0, value = rstrtok(value,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
					pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i] = simple_strtol(value, 0, 10);
			}
			else
			{
				/* parameter parsing */
	            for (i = 0, value = rstrtok(tmpbuf,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
					pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i] = simple_strtol(value, 0, 10);
			}
#else
			/* parameter parsing */
            for (i = 0, value = rstrtok(tmpbuf,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
				pAd->CommonCfg.cPowerUpCckOfdm[BAND0][i] = simple_strtol(value, 0, 10);
#endif /* DBDC_MODE */

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[PowerUpCckOfdm] BAND0: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
																pAd->CommonCfg.cPowerUpCckOfdm[BAND0][0],
																pAd->CommonCfg.cPowerUpCckOfdm[BAND0][1],
																pAd->CommonCfg.cPowerUpCckOfdm[BAND0][2],
																pAd->CommonCfg.cPowerUpCckOfdm[BAND0][3],
																pAd->CommonCfg.cPowerUpCckOfdm[BAND0][4],
																pAd->CommonCfg.cPowerUpCckOfdm[BAND0][5],
																pAd->CommonCfg.cPowerUpCckOfdm[BAND0][6]
																));

#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode)
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[PowerUpCckOfdm] BAND1: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
																	pAd->CommonCfg.cPowerUpCckOfdm[BAND1][0],
																	pAd->CommonCfg.cPowerUpCckOfdm[BAND1][1],
																	pAd->CommonCfg.cPowerUpCckOfdm[BAND1][2],
																	pAd->CommonCfg.cPowerUpCckOfdm[BAND1][3],
																	pAd->CommonCfg.cPowerUpCckOfdm[BAND1][4],
																	pAd->CommonCfg.cPowerUpCckOfdm[BAND1][5],
																	pAd->CommonCfg.cPowerUpCckOfdm[BAND1][6]
																	));
			}
#endif /* DBDC_MODE */
		}

		/* Power Boost (HT20) */
        if(RTMPGetKeyParameter("PowerUpHT20", tmpbuf, 32, pBuffer, TRUE))
        {
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode)
			{
				RTMP_STRING *ptmpStr[DBDC_BAND_NUM];

				/* parameter parsing (Phase I) */
				for (i = 0, value = rstrtok(tmpbuf,";"); value; value = rstrtok(NULL,";"), i++)
					ptmpStr[i] = value;

				/* sanity check for paramter parsing (Phase I) */
				if (i != DBDC_BAND_NUM)
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PowerUpHT20] Input parameter incorrect!!\n"));

				/* Band1 Parameter parsing (Phase II) */
				value = ptmpStr[0];
				for (i = 0, value = rstrtok(value,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
					pAd->CommonCfg.cPowerUpHt20[BAND1][i] = simple_strtol(value, 0, 10);

				/* Band0 Parameter parsing (Phase II) */
				value = ptmpStr[1];
				for (i = 0, value = rstrtok(value,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
					pAd->CommonCfg.cPowerUpHt20[BAND0][i] = simple_strtol(value, 0, 10);
			}
			else
			{
				/* parameter parsing */
				for (i = 0, value = rstrtok(tmpbuf,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
					pAd->CommonCfg.cPowerUpHt20[BAND0][i] = simple_strtol(value, 0, 10);
			}
#else
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
				pAd->CommonCfg.cPowerUpHt20[BAND0][i] = simple_strtol(value, 0, 10);
#endif /* DBDC_MODE */

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[PowerUpHT20] BAND0: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
																pAd->CommonCfg.cPowerUpHt20[BAND0][0],
																pAd->CommonCfg.cPowerUpHt20[BAND0][1],
																pAd->CommonCfg.cPowerUpHt20[BAND0][2],
																pAd->CommonCfg.cPowerUpHt20[BAND0][3],
																pAd->CommonCfg.cPowerUpHt20[BAND0][4],
																pAd->CommonCfg.cPowerUpHt20[BAND0][5],
																pAd->CommonCfg.cPowerUpHt20[BAND0][6]
																));
			
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode)
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[PowerUpHT20] BAND1: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
																	pAd->CommonCfg.cPowerUpHt20[BAND1][0],
																	pAd->CommonCfg.cPowerUpHt20[BAND1][1],
																	pAd->CommonCfg.cPowerUpHt20[BAND1][2],
																	pAd->CommonCfg.cPowerUpHt20[BAND1][3],
																	pAd->CommonCfg.cPowerUpHt20[BAND1][4],
																	pAd->CommonCfg.cPowerUpHt20[BAND1][5],
																	pAd->CommonCfg.cPowerUpHt20[BAND1][6]
																	));
			}
#endif /* DBDC_MODE */
		}

		/* Power Boost (HT40) */
        if(RTMPGetKeyParameter("PowerUpHT40", tmpbuf, 32, pBuffer, TRUE))
        {
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode)
			{
				RTMP_STRING *ptmpStr[DBDC_BAND_NUM];

				/* parameter parsing (Phase I) */
				for (i = 0, value = rstrtok(tmpbuf,";"); value; value = rstrtok(NULL,";"), i++)
					ptmpStr[i] = value;

				/* sanity check for paramter parsing (Phase I) */
				if (i != DBDC_BAND_NUM)
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PowerUpHT40] Input parameter incorrect!!\n"));

				/* Band1 Parameter parsing (Phase II) */
				value = ptmpStr[0];
				for (i = 0, value = rstrtok(value,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
					pAd->CommonCfg.cPowerUpHt40[BAND1][i] = simple_strtol(value, 0, 10);

				/* Band0 Parameter parsing (Phase II) */
				value = ptmpStr[1];
				for (i = 0, value = rstrtok(value,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
					pAd->CommonCfg.cPowerUpHt40[BAND0][i] = simple_strtol(value, 0, 10);
			}
			else
			{
				/* parameter parsing */
				for (i = 0, value = rstrtok(tmpbuf,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
					pAd->CommonCfg.cPowerUpHt40[BAND0][i] = simple_strtol(value, 0, 10);
			}
#else
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
				pAd->CommonCfg.cPowerUpHt40[BAND0][i] = simple_strtol(value, 0, 10);
#endif /* DBDC_MODE */

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[PowerUpHT40] BAND0: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
																pAd->CommonCfg.cPowerUpHt40[BAND0][0],
																pAd->CommonCfg.cPowerUpHt40[BAND0][1],
																pAd->CommonCfg.cPowerUpHt40[BAND0][2],
																pAd->CommonCfg.cPowerUpHt40[BAND0][3],
																pAd->CommonCfg.cPowerUpHt40[BAND0][4],
																pAd->CommonCfg.cPowerUpHt40[BAND0][5],
																pAd->CommonCfg.cPowerUpHt40[BAND0][6]
																));
			
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode)
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[PowerUpHT40] BAND1: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
																	pAd->CommonCfg.cPowerUpHt40[BAND1][0],
																	pAd->CommonCfg.cPowerUpHt40[BAND1][1],
																	pAd->CommonCfg.cPowerUpHt40[BAND1][2],
																	pAd->CommonCfg.cPowerUpHt40[BAND1][3],
																	pAd->CommonCfg.cPowerUpHt40[BAND1][4],
																	pAd->CommonCfg.cPowerUpHt40[BAND1][5],
																	pAd->CommonCfg.cPowerUpHt40[BAND1][6]
																	));
			}
#endif /* DBDC_MODE */      
		}

		/* Power Boost (VHT20) */
        if(RTMPGetKeyParameter("PowerUpVHT20", tmpbuf, 32, pBuffer, TRUE))
        {
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode)
			{
				RTMP_STRING *ptmpStr[DBDC_BAND_NUM];

				/* parameter parsing (Phase I) */
				for (i = 0, value = rstrtok(tmpbuf,";"); value; value = rstrtok(NULL,";"), i++)
					ptmpStr[i] = value;

				/* sanity check for paramter parsing (Phase I) */
				if (i != DBDC_BAND_NUM)
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PowerUpVHT20] Input parameter incorrect!!\n"));

				/* Band1 Parameter parsing (Phase II) */
				value = ptmpStr[0];
				for (i = 0, value = rstrtok(value,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
					pAd->CommonCfg.cPowerUpVht20[BAND1][i] = simple_strtol(value, 0, 10);

				/* Band0 Parameter parsing (Phase II) */
				value = ptmpStr[1];
				for (i = 0, value = rstrtok(value,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
					pAd->CommonCfg.cPowerUpVht20[BAND0][i] = simple_strtol(value, 0, 10);
			}
			else
			{
				/* parameter parsing */
				for (i = 0, value = rstrtok(tmpbuf,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
					pAd->CommonCfg.cPowerUpVht20[BAND0][i] = simple_strtol(value, 0, 10);
			}
#else
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
				pAd->CommonCfg.cPowerUpVht20[BAND0][i] = simple_strtol(value, 0, 10);
#endif /* DBDC_MODE */

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[PowerUpVHT20] BAND0: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
																pAd->CommonCfg.cPowerUpVht20[BAND0][0],
																pAd->CommonCfg.cPowerUpVht20[BAND0][1],
																pAd->CommonCfg.cPowerUpVht20[BAND0][2],
																pAd->CommonCfg.cPowerUpVht20[BAND0][3],
																pAd->CommonCfg.cPowerUpVht20[BAND0][4],
																pAd->CommonCfg.cPowerUpVht20[BAND0][5],
																pAd->CommonCfg.cPowerUpVht20[BAND0][6]
																));
					
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode)
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[PowerUpVHT20] BAND1: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
																	pAd->CommonCfg.cPowerUpVht20[BAND1][0],
																	pAd->CommonCfg.cPowerUpVht20[BAND1][1],
																	pAd->CommonCfg.cPowerUpVht20[BAND1][2],
																	pAd->CommonCfg.cPowerUpVht20[BAND1][3],
																	pAd->CommonCfg.cPowerUpVht20[BAND1][4],
																	pAd->CommonCfg.cPowerUpVht20[BAND1][5],
																	pAd->CommonCfg.cPowerUpVht20[BAND1][6]
																	));
			}
#endif /* DBDC_MODE */      
		}

		/* Power Boost (VHT40) */
        if(RTMPGetKeyParameter("PowerUpVHT40", tmpbuf, 32, pBuffer, TRUE))
        {
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode)
			{
				RTMP_STRING *ptmpStr[DBDC_BAND_NUM];

				/* parameter parsing (Phase I) */
				for (i = 0, value = rstrtok(tmpbuf,";"); value; value = rstrtok(NULL,";"), i++)
					ptmpStr[i] = value;

				/* sanity check for paramter parsing (Phase I) */
				if (i != DBDC_BAND_NUM)
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PowerUpVHT40] Input parameter incorrect!!\n"));

				/* Band1 Parameter parsing (Phase II) */
				value = ptmpStr[0];
				for (i = 0, value = rstrtok(value,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
					pAd->CommonCfg.cPowerUpVht40[BAND1][i] = simple_strtol(value, 0, 10);

				/* Band0 Parameter parsing (Phase II) */
				value = ptmpStr[1];
				for (i = 0, value = rstrtok(value,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
					pAd->CommonCfg.cPowerUpVht40[BAND0][i] = simple_strtol(value, 0, 10);
			}
			else
			{
				/* parameter parsing */
				for (i = 0, value = rstrtok(tmpbuf,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
					pAd->CommonCfg.cPowerUpVht40[BAND0][i] = simple_strtol(value, 0, 10);
			}
#else
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
				pAd->CommonCfg.cPowerUpVht40[BAND0][i] = simple_strtol(value, 0, 10);
#endif /* DBDC_MODE */

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[PowerUpVHT40] BAND0: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
																pAd->CommonCfg.cPowerUpVht40[BAND0][0],
																pAd->CommonCfg.cPowerUpVht40[BAND0][1],
																pAd->CommonCfg.cPowerUpVht40[BAND0][2],
																pAd->CommonCfg.cPowerUpVht40[BAND0][3],
																pAd->CommonCfg.cPowerUpVht40[BAND0][4],
																pAd->CommonCfg.cPowerUpVht40[BAND0][5],
																pAd->CommonCfg.cPowerUpVht40[BAND0][6]
																));
							
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode)
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[PowerUpVHT40] BAND1: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
																	pAd->CommonCfg.cPowerUpVht40[BAND1][0],
																	pAd->CommonCfg.cPowerUpVht40[BAND1][1],
																	pAd->CommonCfg.cPowerUpVht40[BAND1][2],
																	pAd->CommonCfg.cPowerUpVht40[BAND1][3],
																	pAd->CommonCfg.cPowerUpVht40[BAND1][4],
																	pAd->CommonCfg.cPowerUpVht40[BAND1][5],
																	pAd->CommonCfg.cPowerUpVht40[BAND1][6]
																	));
			}
#endif /* DBDC_MODE */      
		}

		/* Power Boost (VHT80) */
        if(RTMPGetKeyParameter("PowerUpVHT80", tmpbuf, 32, pBuffer, TRUE))
        {
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode)
			{
				RTMP_STRING *ptmpStr[DBDC_BAND_NUM];

				/* parameter parsing (Phase I) */
				for (i = 0, value = rstrtok(tmpbuf,";"); value; value = rstrtok(NULL,";"), i++)
					ptmpStr[i] = value;

				/* sanity check for paramter parsing (Phase I) */
				if (i != DBDC_BAND_NUM)
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PowerUpVHT80] Input parameter incorrect!!\n"));

				/* Band1 Parameter parsing (Phase II) */
				value = ptmpStr[0];
				for (i = 0, value = rstrtok(value,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
					pAd->CommonCfg.cPowerUpVht80[BAND1][i] = simple_strtol(value, 0, 10);

				/* Band0 Parameter parsing (Phase II) */
				value = ptmpStr[1];
				for (i = 0, value = rstrtok(value,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
					pAd->CommonCfg.cPowerUpVht80[BAND0][i] = simple_strtol(value, 0, 10);
			}
			else
			{
				/* parameter parsing */
				for (i = 0, value = rstrtok(tmpbuf,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
					pAd->CommonCfg.cPowerUpVht80[BAND0][i] = simple_strtol(value, 0, 10);
			}
#else
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
				pAd->CommonCfg.cPowerUpVht80[BAND0][i] = simple_strtol(value, 0, 10);
#endif /* DBDC_MODE */

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[PowerUpVHT80] BAND0: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
																pAd->CommonCfg.cPowerUpVht80[BAND0][0],
																pAd->CommonCfg.cPowerUpVht80[BAND0][1],
																pAd->CommonCfg.cPowerUpVht80[BAND0][2],
																pAd->CommonCfg.cPowerUpVht80[BAND0][3],
																pAd->CommonCfg.cPowerUpVht80[BAND0][4],
																pAd->CommonCfg.cPowerUpVht80[BAND0][5],
																pAd->CommonCfg.cPowerUpVht80[BAND0][6]
																));

#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode)
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[PowerUpVHT80] BAND1: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
																	pAd->CommonCfg.cPowerUpVht80[BAND1][0],
																	pAd->CommonCfg.cPowerUpVht80[BAND1][1],
																	pAd->CommonCfg.cPowerUpVht80[BAND1][2],
																	pAd->CommonCfg.cPowerUpVht80[BAND1][3],
																	pAd->CommonCfg.cPowerUpVht80[BAND1][4],
																	pAd->CommonCfg.cPowerUpVht80[BAND1][5],
																	pAd->CommonCfg.cPowerUpVht80[BAND1][6]
																	));
			}
#endif /* DBDC_MODE */      
		}

		/* Power Boost (VHT160) */
        if(RTMPGetKeyParameter("PowerUpVHT160", tmpbuf, 32, pBuffer, TRUE))
        {
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode)
			{
				RTMP_STRING *ptmpStr[DBDC_BAND_NUM];

				/* parameter parsing (Phase I) */
				for (i = 0, value = rstrtok(tmpbuf,";"); value; value = rstrtok(NULL,";"), i++)
					ptmpStr[i] = value;

				/* sanity check for paramter parsing (Phase I) */
				if (i != DBDC_BAND_NUM)
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PowerUpVHT160] Input parameter incorrect!!\n"));

				/* Band1 Parameter parsing (Phase II) */
				value = ptmpStr[0];
				for (i = 0, value = rstrtok(value,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
					pAd->CommonCfg.cPowerUpVht160[BAND1][i] = simple_strtol(value, 0, 10);

				/* Band0 Parameter parsing (Phase II) */
				value = ptmpStr[1];
				for (i = 0, value = rstrtok(value,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
					pAd->CommonCfg.cPowerUpVht160[BAND0][i] = simple_strtol(value, 0, 10);
			}
			else
			{
				/* parameter parsing */
				for (i = 0, value = rstrtok(tmpbuf,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
					pAd->CommonCfg.cPowerUpVht160[BAND0][i] = simple_strtol(value, 0, 10);
			}
#else
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf,":"); (value) && (i < POWER_UP_CATEGORY_RATE_NUM); value = rstrtok(NULL,":"), i++)
				pAd->CommonCfg.cPowerUpVht160[BAND0][i] = simple_strtol(value, 0, 10);
#endif /* DBDC_MODE */

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[PowerUpVHT160] BAND0: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
																pAd->CommonCfg.cPowerUpVht160[BAND0][0],
																pAd->CommonCfg.cPowerUpVht160[BAND0][1],
																pAd->CommonCfg.cPowerUpVht160[BAND0][2],
																pAd->CommonCfg.cPowerUpVht160[BAND0][3],
																pAd->CommonCfg.cPowerUpVht160[BAND0][4],
																pAd->CommonCfg.cPowerUpVht160[BAND0][5],
																pAd->CommonCfg.cPowerUpVht160[BAND0][6]
																));
									
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode)
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[PowerUpVHT160] BAND1: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
																	pAd->CommonCfg.cPowerUpVht160[BAND1][0],
																	pAd->CommonCfg.cPowerUpVht160[BAND1][1],
																	pAd->CommonCfg.cPowerUpVht160[BAND1][2],
																	pAd->CommonCfg.cPowerUpVht160[BAND1][3],
																	pAd->CommonCfg.cPowerUpVht160[BAND1][4],
																	pAd->CommonCfg.cPowerUpVht160[BAND1][5],
																	pAd->CommonCfg.cPowerUpVht160[BAND1][6]
																	));
			}
#endif /* DBDC_MODE */
		}
#endif /* TX_POWER_CONTROL_SUPPORT */

#ifdef SINGLE_SKU_V2

        /* TxPower SKU */
        if(RTMPGetKeyParameter("SKUenable", tmpbuf, 32, pBuffer, TRUE))
        {
            /* parameter parsing */
            for (i = BAND0, value = rstrtok(tmpbuf,";"); value; value = rstrtok(NULL,";"), i++)
            {
#ifdef DBDC_MODE
                if (pAd->CommonCfg.dbdc_mode)
                {
                    switch (i)
                    { 
                        case 0:
                            pAd->CommonCfg.SKUenable[BAND1] = simple_strtol(value, 0, 10);
                            break;

                        case 1:
                            pAd->CommonCfg.SKUenable[BAND0] = simple_strtol(value, 0, 10);
                            break;    

                        default:
                            break;
                    }
                }
                else
                {
                    switch (i)
                    { 
                        case 0:
                            pAd->CommonCfg.SKUenable[BAND0] = simple_strtol(value, 0, 10);
                            break;   

                        default:
                            break;
                    }
                }
#else
                switch (i)
                {
                    case 0:
                        pAd->CommonCfg.SKUenable[BAND0] = simple_strtol(value, 0, 10);
                        break;

                    default:
                        break;
                }
#endif /* DBDC_MODE */
            }

#ifdef DBDC_MODE
            if (pAd->CommonCfg.dbdc_mode)
                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[SKUenable] BAND0: %d, BAND1: %d \n", pAd->CommonCfg.SKUenable[BAND0], pAd->CommonCfg.SKUenable[BAND1]));
            else
                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[SKUenable] BAND0: %d \n", pAd->CommonCfg.SKUenable[BAND0]));
#else
            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[SKUenable] BAND0: %d \n", pAd->CommonCfg.SKUenable[BAND0]));
#endif /* DBDC_MODE */      
        }
#endif /*SINGLE_SKU_V2 */

        /* TxPower Percentage */
        if(RTMPGetKeyParameter("PERCENTAGEenable", tmpbuf, 32, pBuffer, TRUE))
        {
            /* parameter parsing */
            for (i = BAND0, value = rstrtok(tmpbuf,";"); value; value = rstrtok(NULL,";"), i++)
            {
#ifdef DBDC_MODE
                if (pAd->CommonCfg.dbdc_mode)
                {
                    switch (i)
                    { 
                        case 0:
                            pAd->CommonCfg.PERCENTAGEenable[BAND1] = simple_strtol(value, 0, 10);
                            break;

                        case 1:
                            pAd->CommonCfg.PERCENTAGEenable[BAND0] = simple_strtol(value, 0, 10);
                            break;    

                        default:
                            break;
                    }
                }
                else
                {
                    switch (i)
                    { 
                        case 0:
                            pAd->CommonCfg.PERCENTAGEenable[BAND0] = simple_strtol(value, 0, 10);
                            break;   

                        default:
                            break;
                    }
                }
#else
                switch (i)
                {
                    case 0:
                        pAd->CommonCfg.PERCENTAGEenable[BAND0] = simple_strtol(value, 0, 10);
                        break;   

                    default:
                        break;
                }
#endif /* DBDC_MODE */
            }

#ifdef DBDC_MODE
            if (pAd->CommonCfg.dbdc_mode)
                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[PERCENTAGEenable] BAND0: %d, BAND1: %d \n", pAd->CommonCfg.PERCENTAGEenable[BAND0], pAd->CommonCfg.PERCENTAGEenable[BAND1]));
            else
                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[PERCENTAGEenable] BAND0: %d \n", pAd->CommonCfg.PERCENTAGEenable[BAND0]));
#else
            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[PERCENTAGEenable] BAND0: %d \n", pAd->CommonCfg.PERCENTAGEenable[BAND0]));
#endif /* DBDC_MODE */      
        }

        /* TxPower BF Backoff */
        if(RTMPGetKeyParameter("BFBACKOFFenable", tmpbuf, 32, pBuffer, TRUE))
        {
            /* parameter parsing */
            for (i = BAND0, value = rstrtok(tmpbuf,";"); value; value = rstrtok(NULL,";"), i++)
            {        
#ifdef DBDC_MODE
                if (pAd->CommonCfg.dbdc_mode)
                {
                    switch (i)
                    { 
                        case 0:
                            pAd->CommonCfg.BFBACKOFFenable[BAND1] = simple_strtol(value, 0, 10);
                            break;

                        case 1:
                            pAd->CommonCfg.BFBACKOFFenable[BAND0] = simple_strtol(value, 0, 10);
                            break;    

                        default:
                            break;
                    }
                }
                else
                {
                    switch (i)
                    { 
                        case 0:
                            pAd->CommonCfg.BFBACKOFFenable[BAND0] = simple_strtol(value, 0, 10);
                            break;

                        default:
                            break;
                    }
                }
#else
                switch (i)
                {
                    case 0:
                        pAd->CommonCfg.BFBACKOFFenable[BAND0] = simple_strtol(value, 0, 10);
                        break;   

                    default:
                        break;
                }
#endif /* DBDC_MODE */
            }

#ifdef DBDC_MODE
            if (pAd->CommonCfg.dbdc_mode)
                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[BFBACKOFFenable] BAND0: %d, BAND1: %d \n", pAd->CommonCfg.BFBACKOFFenable[BAND0], pAd->CommonCfg.BFBACKOFFenable[BAND1]));
            else
                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[BFBACKOFFenable] BAND0: %d \n", pAd->CommonCfg.BFBACKOFFenable[BAND0]));
#else
            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[BFBACKOFFenable] BAND0: %d \n", pAd->CommonCfg.BFBACKOFFenable[BAND0]));
#endif /* DBDC_MODE */
        }
        
#ifdef LINK_TEST_SUPPORT
			/* CMW270 Link */
			if(RTMPGetKeyParameter("LinkTestSupport", tmpbuf, 32, pBuffer, TRUE))
			{
				/* parameter parsing */
				for (i = BAND0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++)
				{
#ifdef DBDC_MODE
					if (pAd->CommonCfg.dbdc_mode)
					{
						switch (i)
						{ 
							case 0:
								pAd->CommonCfg.LinkTestSupportTemp[BAND1] = simple_strtol(value, 0, 10);
								break;
	
							case 1:
								pAd->CommonCfg.LinkTestSupportTemp[BAND0] = simple_strtol(value, 0, 10);
								break;	  
	
							default:
								break;
						}
					}
					else
					{
						switch (i)
						{ 
							case 0:
								pAd->CommonCfg.LinkTestSupportTemp[BAND0] = simple_strtol(value, 0, 10);
								break;
	
							default:
								break;
						}
					}
#else
					switch (i)
					{
						case 0:
							pAd->CommonCfg.LinkTestSupportTemp[BAND0] = simple_strtol(value, 0, 10);
							break;	 
	
						default:
							break;
					}
#endif /* DBDC_MODE */
				}
	
				/* LinkTestSupport can be enabled by any profile */
#ifdef DBDC_MODE            
				if (pAd->CommonCfg.LinkTestSupportTemp[BAND0] || pAd->CommonCfg.LinkTestSupportTemp[BAND1])
#else
				if (pAd->CommonCfg.LinkTestSupportTemp[BAND0])
#endif /* DBDC_MODE */
				{
					pAd->CommonCfg.LinkTestSupport = TRUE;
				}
				else
				{
					pAd->CommonCfg.LinkTestSupport = FALSE;
				}
	
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("LinkTestSupport = %d \n", pAd->CommonCfg.LinkTestSupport));	
			}
#endif /* LINK_TEST_SUPPORT */

#ifdef RLM_CAL_CACHE_SUPPORT
        /* Calibration Cache Support */
        if(RTMPGetKeyParameter("CalCacheApply", tmpbuf, 32, pBuffer, TRUE))
        {
            pAd->CommonCfg.CalCacheApply = (ULONG) simple_strtol(tmpbuf, 0, 10);
            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("CalCacheApply = %d \n", pAd->CommonCfg.CalCacheApply));
        }
#endif /* RLM_CAL_CACHE_SUPPORT */
        
		/*BGProtection*/
		if(RTMPGetKeyParameter("BGProtection", tmpbuf, 10, pBuffer, TRUE))
		{
	/*#if 0	#ifndef WIFI_TEST*/
	/*		pAd->CommonCfg.UseBGProtection = 2; disable b/g protection for throughput test*/
	/*#else*/
			switch (simple_strtol(tmpbuf, 0, 10))
			{
				case 1: /*Always On*/
					pAd->CommonCfg.UseBGProtection = 1;
					break;
				case 2: /*Always OFF*/
					pAd->CommonCfg.UseBGProtection = 2;
					break;
				case 0: /*AUTO*/
				default:
					pAd->CommonCfg.UseBGProtection = 0;
					break;
			}
	/*#endif*/
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BGProtection=%ld\n", pAd->CommonCfg.UseBGProtection));
		}

#ifdef CONFIG_AP_SUPPORT
		/*OLBCDetection*/
		if(RTMPGetKeyParameter("DisableOLBC", tmpbuf, 10, pBuffer, TRUE))
		{
			switch (simple_strtol(tmpbuf, 0, 10))
			{
				case 1: /*disable OLBC Detection*/
					pAd->CommonCfg.DisableOLBCDetect = 1;
					break;
				case 0: /*enable OLBC Detection*/
					pAd->CommonCfg.DisableOLBCDetect = 0;
					break;
				default:
					pAd->CommonCfg.DisableOLBCDetect= 0;
					break;
			}
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OLBCDetection=%ld\n", pAd->CommonCfg.DisableOLBCDetect));
		}
#endif /* CONFIG_AP_SUPPORT */
		/*TxPreamble*/
		if(RTMPGetKeyParameter("TxPreamble", tmpbuf, 10, pBuffer, TRUE))
		{
			switch (simple_strtol(tmpbuf, 0, 10))
			{
				case Rt802_11PreambleShort:
					pAd->CommonCfg.TxPreamble = Rt802_11PreambleShort;
					break;
				case Rt802_11PreambleAuto:
					pAd->CommonCfg.TxPreamble = Rt802_11PreambleAuto;
					break;
				case Rt802_11PreambleLong:
				default:
					pAd->CommonCfg.TxPreamble = Rt802_11PreambleLong;
					break;
			}
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("TxPreamble=%ld\n", pAd->CommonCfg.TxPreamble));
		}

		/*RTSPktThreshold*/
		read_rts_pkt_thld_from_file(pAd, tmpbuf, pBuffer);
		/*RTSThreshold*/
		read_rts_len_thld_from_file(pAd, tmpbuf, pBuffer);

		/*FragThreshold*/
		read_frag_thld_from_file(pAd, tmpbuf, pBuffer);

		/*Udmaenable & UdmaPortNum*/
#ifdef RTMP_UDMA_SUPPORT
		if(RTMPGetKeyParameter("UdmaEnable",tmpbuf,10,pBuffer ,TRUE))
		{
			if(simple_strtol(tmpbuf ,0,10) != 0)
			{
				pAd->CommonCfg.bUdmaFlag = TRUE;/*Enable*/
			}
			else
				pAd->CommonCfg.bUdmaFlag = FALSE;
		}
		
		if(RTMPGetKeyParameter("UdmaPortNum",tmpbuf,10,pBuffer ,TRUE))
		{
			if(simple_strtol(tmpbuf,0,10) != 0) /*Enable*/
				pAd->CommonCfg.UdmaPortNum = UDMA_PORT1;
			else
				pAd->CommonCfg.UdmaPortNum = UDMA_PORT0;
		}
#endif/*RTMP_UDMA_SUPPORT*/
		/*TxBurst*/
		if(RTMPGetKeyParameter("TxBurst", tmpbuf, 10, pBuffer, TRUE))
		{
	/*#ifdef WIFI_TEST*/
	/*						pAd->CommonCfg.bEnableTxBurst = FALSE;*/
	/*#else*/
			if(simple_strtol(tmpbuf, 0, 10) != 0)  /*Enable*/
				pAd->CommonCfg.bEnableTxBurst = TRUE;
			else /*Disable*/
				pAd->CommonCfg.bEnableTxBurst = FALSE;
	/*#endif*/
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("TxBurst=%d\n", pAd->CommonCfg.bEnableTxBurst));
		}

#ifdef AGGREGATION_SUPPORT
		/*PktAggregate*/
		if(RTMPGetKeyParameter("PktAggregate", tmpbuf, 10, pBuffer, TRUE))
		{
			if(simple_strtol(tmpbuf, 0, 10) != 0)  /*Enable*/
				pAd->CommonCfg.bAggregationCapable = TRUE;
			else /*Disable*/
				pAd->CommonCfg.bAggregationCapable = FALSE;
#ifdef PIGGYBACK_SUPPORT
			pAd->CommonCfg.bPiggyBackCapable = pAd->CommonCfg.bAggregationCapable;
#endif /* PIGGYBACK_SUPPORT */
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PktAggregate=%d\n", pAd->CommonCfg.bAggregationCapable));
		}
#else
		pAd->CommonCfg.bAggregationCapable = FALSE;
		pAd->CommonCfg.bPiggyBackCapable = FALSE;
#endif /* AGGREGATION_SUPPORT */

		/* WmmCapable*/
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			rtmp_read_ap_wmm_parms_from_file(pAd, tmpbuf, pBuffer);
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			/* MaxStaNum*/
			if (RTMPGetKeyParameter("MbssMaxStaNum", tmpbuf, 32, pBuffer, TRUE))
			{
			    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
			    {
					if (i >= pAd->ApCfg.BssidNum)
						break;

					ApCfg_Set_PerMbssMaxStaNum_Proc(pAd, i, macptr);
			    }
			}

			/* IdleTimeout*/
			if(RTMPGetKeyParameter("IdleTimeout", tmpbuf, 10, pBuffer, TRUE))
			{
				ApCfg_Set_IdleTimeout_Proc(pAd, tmpbuf);
			}

			/*NoForwarding*/
			if(RTMPGetKeyParameter("NoForwarding", tmpbuf, 32, pBuffer, TRUE))
			{
			    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
			    {
					if (i >= pAd->ApCfg.BssidNum)
						break;

					if(simple_strtol(macptr, 0, 10) != 0)  /*Enable*/
						pAd->ApCfg.MBSSID[i].IsolateInterStaTraffic = TRUE;
					else /*Disable*/
						pAd->ApCfg.MBSSID[i].IsolateInterStaTraffic = FALSE;

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(ra%d) NoForwarding=%ld\n", i, pAd->ApCfg.MBSSID[i].IsolateInterStaTraffic));
			    }
			}
			/*NoForwardingBTNBSSID*/
			if(RTMPGetKeyParameter("NoForwardingBTNBSSID", tmpbuf, 10, pBuffer, TRUE))
			{
				if(simple_strtol(tmpbuf, 0, 10) != 0)  /*Enable*/
					pAd->ApCfg.IsolateInterStaTrafficBTNBSSID = TRUE;
				else /*Disable*/
					pAd->ApCfg.IsolateInterStaTrafficBTNBSSID = FALSE;

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("NoForwardingBTNBSSID=%ld\n", pAd->ApCfg.IsolateInterStaTrafficBTNBSSID));
			}
#ifdef DSCP_QOS_MAP_SUPPORT
			/*DscpQosMapEnable*/
			if(RTMPGetKeyParameter("DscpQosMapEnable", tmpbuf, 10, pBuffer, TRUE))
			{
				for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
				{
					if (i >= 2)
						break;
				
					if(simple_strtol(macptr, 0, 10) != 0)  /*Enable*/
						pAd->ApCfg.DscpQosMapSupport[i] = 1;
					else /*Disable*/
						pAd->ApCfg.DscpQosMapSupport[i] = 0;
				}

				for (i = 0; i < pAd->ApCfg.BssidNum; i++)
				{
					if(pAd->ApCfg.DscpQosMapSupport[0])
					{
						pAd->ApCfg.MBSSID[i].DscpQosMapEnable = TRUE;
						pAd->ApCfg.MBSSID[i].DscpQosPoolId = 0;
					}
					else
						pAd->ApCfg.MBSSID[i].DscpQosMapEnable = FALSE;
				}
#ifdef MULTI_PROFILE
				{
					
					UINT8 bssNum = multi_profile_get_bss_num(pAd,1);
					MTWF_LOG(DBG_CAT_CFG, DBG_CAT_AP, DBG_LVL_OFF,("QosMapping: BssNumber for profile 1 is %d \n",bssNum));
					if(bssNum != 0)
					{
						for(i= bssNum; i < pAd->ApCfg.BssidNum; i++)
						{
							if(pAd->ApCfg.DscpQosMapSupport[1])
							{
								pAd->ApCfg.MBSSID[i].DscpQosMapEnable = TRUE;
								pAd->ApCfg.MBSSID[i].DscpQosPoolId = 1;
							}
							else
								pAd->ApCfg.MBSSID[i].DscpQosMapEnable = FALSE;
						}
					}
				}
#endif
			   for (i = 0; i < pAd->ApCfg.BssidNum; i++)
				{
					if(pAd->ApCfg.MBSSID[i].DscpQosMapEnable)
						MTWF_LOG(DBG_CAT_CFG, DBG_CAT_AP, DBG_LVL_OFF,("Pool id for bss %d is %d \n",i,pAd->ApCfg.MBSSID[i].DscpQosPoolId));
					else
						MTWF_LOG(DBG_CAT_CFG, DBG_CAT_AP, DBG_LVL_OFF,("DscpQosMapping Not enabled for Bss %d \n",i));
			   	}

			}
			/*DscpQosMap*/
			if(RTMPGetKeyParameter("DscpQosMap", tmpbuf, 80, pBuffer, TRUE))
			{
				int j;
				P_DSCP_QOS_MAP_TABLE_T pQosMapPool; 
				UCHAR dscp_buf[16]={0,7,8,15,16,23,24,31,32,39,40,47,48,55,56,63};
				
				RTMP_STRING *macptr2[2];
				macptr2[0]=rstrtok(tmpbuf,";");
				macptr2[1]=rstrtok(NULL,";");
				
				for(j=0; j<2; j++)
				{
					UINT32 ac_map = 0;
				    pQosMapPool = &pAd->ApCfg.DscpQosMapTable[j];
					
					for (i = 0, macptr = rstrtok(macptr2[j],":"); macptr; macptr = rstrtok(NULL,":"), i++)
					{
						UCHAR ac_category;
						if (i > 7)
							break;

						ac_category = simple_strtol(macptr, 0, 10);
						if(ac_category > 7)
							ac_category = 0;
						ac_map = (ac_map | (ac_category << (i*4)));
					}
					if(ac_map == 0)
						ac_map = 3;
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("DSCPQosMAP Setting for Profile %d ==>AC MAP =%08x \n",j,ac_map));
					pQosMapPool->ucPoolValid = 1;
					pQosMapPool->ucDscpExceptionCount=0;
					pQosMapPool->u4Ac = ac_map;
					memset(pQosMapPool->au2DscpException, 0xff, 42);
					memcpy((UCHAR*)pQosMapPool->au2DscpRange, dscp_buf, 16);
				}
			}
#endif	/*DSCP_QOS_MAP_SUPPORT*/

			/*HideSSID*/
			if(RTMPGetKeyParameter("HideSSID", tmpbuf, 32, pBuffer, TRUE))
			{
				for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
			    {
					int apidx = i;

					if (i >= pAd->ApCfg.BssidNum)
						break;

					if(simple_strtol(macptr, 0, 10) != 0)  /*Enable*/
					{
						pAd->ApCfg.MBSSID[apidx].bHideSsid = TRUE;
#ifdef WSC_V2_SUPPORT
						pAd->ApCfg.MBSSID[apidx].WscControl.WscV2Info.bWpsEnable = FALSE;
#endif /* WSC_V2_SUPPORT */
					}
					else /*Disable*/
						pAd->ApCfg.MBSSID[apidx].bHideSsid = FALSE;

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(ra%d) HideSSID=%d\n", i, pAd->ApCfg.MBSSID[apidx].bHideSsid));
				}
			}

			/*StationKeepAlive*/
			if(RTMPGetKeyParameter("StationKeepAlive", tmpbuf, 32, pBuffer, TRUE))
			{
				for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
			    {
					int apidx = i;

					if (i >= pAd->ApCfg.BssidNum)
						break;

					pAd->ApCfg.MBSSID[apidx].StationKeepAliveTime = simple_strtol(macptr, 0, 10);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(ra%d) StationKeepAliveTime=%d\n", i, pAd->ApCfg.MBSSID[apidx].StationKeepAliveTime));
				}
			}

			/*AutoChannelSelect*/
			if(RTMPGetKeyParameter("AutoChannelSelect", tmpbuf, 10, pBuffer, TRUE))
			{
				if(simple_strtol(tmpbuf, 0, 10) != 0)  /*Enable*/
				{
					ChannelSel_Alg SelAlg=(ChannelSel_Alg)simple_strtol(tmpbuf, 0, 10);
					if (SelAlg > 3 || SelAlg < 0)
					{
						pAd->ApCfg.bAutoChannelAtBootup = FALSE;
					}
					else /*Enable*/
					{
                                              if (IS_MT7615(pAd))
                                              {
                                                    SelAlg = 3;
                                               }					
						pAd->ApCfg.bAutoChannelAtBootup = TRUE;
						pAd->ApCfg.AutoChannelAlg = SelAlg;
					}
				}
				else /*Disable*/
					pAd->ApCfg.bAutoChannelAtBootup = FALSE;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("AutoChannelAtBootup=%d\n", pAd->ApCfg.bAutoChannelAtBootup));
			}

			/*AutoChannelSkipList*/
			if (RTMPGetKeyParameter("AutoChannelSkipList", tmpbuf, 50, pBuffer, FALSE))
			{
				pAd->ApCfg.AutoChannelSkipListNum = delimitcnt(tmpbuf, ";") + 1;
				if ( pAd->ApCfg.AutoChannelSkipListNum > 10 )
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Your no. of AutoChannelSkipList( %d ) is larger than 10 (boundary)\n",pAd->ApCfg.AutoChannelSkipListNum));
					pAd->ApCfg.AutoChannelSkipListNum = 10;
				}

				for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr ; macptr = rstrtok(NULL,";"), i++)
				{
					if (i < pAd->ApCfg.AutoChannelSkipListNum )
					{
						pAd->ApCfg.AutoChannelSkipList[i] = simple_strtol(macptr, 0, 10);
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" AutoChannelSkipList[%d]= %d \n", i, pAd->ApCfg.AutoChannelSkipList[i]));
					}
					else
					{
						break;
					}
				}
			}

#ifdef BACKGROUND_SCAN_SUPPORT
			if (RTMPGetKeyParameter("DfsZeroWait", tmpbuf, 50, pBuffer, FALSE))
			{
                UINT8 DfsZeroWait = simple_strtol(tmpbuf ,0,10);
			    if ((DfsZeroWait == 1) 
#ifdef MT_DFS_SUPPORT                    
                    && IS_SUPPORT_MT_DFS(pAd)
#endif              
                    )
			    {
			        pAd->BgndScanCtrl.DfsZeroWaitSupport = TRUE;/*Enable*/
#ifdef MT_DFS_SUPPORT
                    UPDATE_MT_ZEROWAIT_DFS_Support(pAd, TRUE);
#endif
			    }
			    else
			    {
				    pAd->BgndScanCtrl.DfsZeroWaitSupport = FALSE;
#ifdef MT_DFS_SUPPORT
                    UPDATE_MT_ZEROWAIT_DFS_Support(pAd, FALSE);
#endif
			    }
                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("DfsZeroWait Support=%d/%d \n", DfsZeroWait,pAd->BgndScanCtrl.DfsZeroWaitSupport));
			}
#ifdef MT_DFS_SUPPORT
		if (RTMPGetKeyParameter("DfsDedicatedZeroWait", tmpbuf, 25, pBuffer, TRUE))			
		{	UCHAR DfsDedicatedZeroWait = (UCHAR) simple_strtol(tmpbuf, 0, 10);
			pAd->DfsParameter.bDedicatedZeroWaitSupport= DfsDedicatedZeroWait;
		}
#endif			
			if (RTMPGetKeyParameter("BgndScanSkipCh", tmpbuf, 50, pBuffer, FALSE))
			{
				pAd->BgndScanCtrl.SkipChannelNum= delimitcnt(tmpbuf, ";") + 1;
				
				for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr ; macptr = rstrtok(NULL,";"), i++)
				{
					if (i < pAd->BgndScanCtrl.SkipChannelNum)
					{
						pAd->BgndScanCtrl.SkipChannelList[i] = simple_strtol(macptr, 0, 10);
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" Background Skip Channel list[%d]= %d \n", i, pAd->BgndScanCtrl.SkipChannelList[i]));
					}
					else
					{
						break;
					}
				}
			}

#endif /* BACKGROUND_SCAN_SUPPORT */

			if (RTMPGetKeyParameter("EDCCAEnable", tmpbuf, 10, pBuffer, FALSE))
			{
				for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr ; macptr = rstrtok(NULL,";"), i++)
				{
					if (i < DBDC_BAND_NUM)
					{
#ifdef DEFAULT_5G_PROFILE				
						if ( i ==0 && (pAd->CommonCfg.dbdc_mode == 1)) {
							pAd->CommonCfg.ucEDCCACtrl[BAND1] = simple_strtol(macptr, 0, 10);
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" EDCCA band[1]= %d \n", pAd->CommonCfg.ucEDCCACtrl[BAND1]));
						} else {
							pAd->CommonCfg.ucEDCCACtrl[BAND0] = simple_strtol(macptr, 0, 10);
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" EDCCA band[0]= %d \n", pAd->CommonCfg.ucEDCCACtrl[BAND0]));
						}	
#else
						pAd->CommonCfg.ucEDCCACtrl[i] = simple_strtol(macptr, 0, 10);
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" EDCCA band[%d]= %d \n", i, pAd->CommonCfg.ucEDCCACtrl[i]));
#endif /* DEFAULT_5G_PROFILE */						
					}
					else
					{
						break;
					}
				}
				
			}
#ifdef MT_DFS_SUPPORT
            if (RTMPGetKeyParameter("DfsZeroWaitCacTime", tmpbuf, 50, pBuffer, FALSE))
            {
                UINT8 OffChnlCacTime = simple_strtol(tmpbuf ,0,10);
                
                pAd->DfsParameter.DfsZeroWaitCacTime = OffChnlCacTime; /* Unit is minute */
                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("DfsZeroWaitCacTime=%d/%d \n", 
                                                          OffChnlCacTime, 
                                                          pAd->DfsParameter.DfsZeroWaitCacTime));    
            }
#endif /* MT_DFS_SUPPORT  */

#ifdef AP_SCAN_SUPPORT
			/*ACSCheckTime*/
			if (RTMPGetKeyParameter("ACSCheckTime", tmpbuf, 32, pBuffer, TRUE))
			{
				UINT8 i, Hour;
				RTMP_STRING *ptr;
				
				for (i = 0, ptr = rstrtok(tmpbuf,";"); ptr; ptr = rstrtok(NULL,";"), i++)
				{
					if (i >= DBDC_BAND_NUM)
						break;
					
					Hour = simple_strtol(ptr, 0, 10);
					pAd->ApCfg.ACSCheckTime[i] = Hour * 3600; /* Hour to second */

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s(): ACSCheckTime[%d]=%u seconds(%u hours)\n",
					__FUNCTION__, i, pAd->ApCfg.ACSCheckTime[i], Hour));
				}
			}
#endif /* AP_SCAN_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */

		/*ShortSlot*/
		if(RTMPGetKeyParameter("ShortSlot", tmpbuf, 10, pBuffer, TRUE))
		{
			RT_CfgSetShortSlot(pAd, tmpbuf);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ShortSlot=%d\n", pAd->CommonCfg.bUseShortSlotTime));
		}

#ifdef TXBF_SUPPORT
		if (pAd->chipCap.FlgHwTxBfCap)
		{
#if defined(CONFIG_AP_SUPPORT) || defined(STA_ITXBF_SUPPORT)
			/*ITxBfEn*/
			if(RTMPGetKeyParameter("ITxBfEn", tmpbuf, 32, pBuffer, TRUE))
			{
				pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn = (simple_strtol(tmpbuf, 0, 10) != 0);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ITxBfEn = %d\n", pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn));


			}

			/* ITxBfTimeout */
			if(RTMPGetKeyParameter("ITxBfTimeout", tmpbuf, 32, pBuffer, TRUE))
			{
				pAd->CommonCfg.ITxBfTimeout = simple_strtol(tmpbuf, 0, 10);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ITxBfTimeout = %ld\n", pAd->CommonCfg.ITxBfTimeout));
			}
#endif /* defined(CONFIG_AP_SUPPORT) || defined(STA_ITXBF_SUPPORT) */

			/* ETxBfEnCond*/
			if(RTMPGetKeyParameter("ETxBfEnCond", tmpbuf, 32, pBuffer, TRUE))
			{
				pAd->CommonCfg.ETxBfEnCond = simple_strtol(tmpbuf, 0, 10);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ETxBfEnCond = %ld\n", pAd->CommonCfg.ETxBfEnCond));

				if (pAd->CommonCfg.ETxBfEnCond)
				{
					pAd->CommonCfg.RegTransmitSetting.field.TxBF = TRUE;
				}
				else
				{
					pAd->CommonCfg.RegTransmitSetting.field.TxBF = FALSE;
			}

			/* MUTxRxEnable*/
			if(RTMPGetKeyParameter("MUTxRxEnable", tmpbuf, 32, pBuffer, TRUE))
			{
				pAd->CommonCfg.MUTxRxEnable = simple_strtol(tmpbuf, 0, 10);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MUTxRxEnable = %ld\n", pAd->CommonCfg.MUTxRxEnable));
			}

#ifdef TXBF_BY_CHANNEL
			/* BandNoBf */
			if(RTMPGetKeyParameter("BandNoBf", tmpbuf, 32, pBuffer, TRUE))
			{
				pAd->CommonCfg.BandNoBf = simple_strtol(tmpbuf, 0, 10);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BandNoBf = %d\n", pAd->CommonCfg.BandNoBf));
			}
			pAd->CommonCfg.ETxBfEnCondBackup = SUBF_OFF;
			pAd->CommonCfg.ITxBfEnBackup = FALSE;
#endif /* TXBF_BY_CHANNEL */



			}

			/* ETxBfTimeout*/
			if(RTMPGetKeyParameter("ETxBfTimeout", tmpbuf, 32, pBuffer, TRUE))
			{
				pAd->CommonCfg.ETxBfTimeout = simple_strtol(tmpbuf, 0, 10);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ETxBfTimeout = %ld\n", pAd->CommonCfg.ETxBfTimeout));
			}

			/* ETxBfNoncompress*/
			if(RTMPGetKeyParameter("ETxBfNoncompress", tmpbuf, 32, pBuffer, TRUE))
			{
				pAd->CommonCfg.ETxBfNoncompress = simple_strtol(tmpbuf, 0, 10);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ETxBfNoncompress = %d\n", pAd->CommonCfg.ETxBfNoncompress));
			}

			/* ETxBfIncapable */
			if(RTMPGetKeyParameter("ETxBfIncapable", tmpbuf, 32, pBuffer, TRUE))
			{
				pAd->CommonCfg.ETxBfIncapable = simple_strtol(tmpbuf, 0, 10);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ETxBfIncapable = %d\n", pAd->CommonCfg.ETxBfIncapable));
			}
		}
#endif /* TXBF_SUPPORT */


#ifdef PRE_ANT_SWITCH
		/*PreAntSwitch*/
		if(RTMPGetKeyParameter("PreAntSwitch", tmpbuf, 32, pBuffer, TRUE))
		{
			pAd->CommonCfg.PreAntSwitch = (simple_strtol(tmpbuf, 0, 10) != 0);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PreAntSwitch = %d\n", pAd->CommonCfg.PreAntSwitch));
		}
#endif /* PRE_ANT_SWITCH */



#ifdef STREAM_MODE_SUPPORT
		/* StreamMode*/
		if (pAd->chipCap.FlgHwStreamMode)
		{
			if(RTMPGetKeyParameter("StreamMode", tmpbuf, 32, pBuffer, TRUE))
			{
				pAd->CommonCfg.StreamMode = (simple_strtol(tmpbuf, 0, 10) & 0x03);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("StreamMode= %d\n", pAd->CommonCfg.StreamMode));
			}

			/* StreamModeMac*/
			for (i = 0; i < STREAM_MODE_STA_NUM; i++)
			{
				RTMP_STRING tok_str[32];

				sprintf(tok_str, "StreamModeMac%d", i);

				if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAM_BUFFER_SIZE, pBuffer, TRUE))
				{
					int j;
					if(strlen(tmpbuf) != 17)  /*Mac address acceptable format 01:02:03:04:05:06 length 17*/
						continue;

					for (j=0; j<MAC_ADDR_LEN; j++)
					{
						AtoH(tmpbuf, &pAd->CommonCfg.StreamModeMac[i][j], 1);
						tmpbuf=tmpbuf+3;
					}
				}
			}

			if (NdisEqualMemory(ZERO_MAC_ADDR, &pAd->CommonCfg.StreamModeMac[0][0], MAC_ADDR_LEN))
			{
				/* set default broadcast mac to entry 0 if user not set it */
				NdisMoveMemory(&pAd->CommonCfg.StreamModeMac[0][0], BROADCAST_ADDR, MAC_ADDR_LEN);
			}
		}
#endif /* STREAM_MODE_SUPPORT */

#ifdef DBG_CTRL_SUPPORT
		/*DebugFlags*/
		if(RTMPGetKeyParameter("DebugFlags", tmpbuf, 32, pBuffer, TRUE))
		{
			pAd->CommonCfg.DebugFlags = simple_strtol(tmpbuf, 0, 16);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DebugFlags = 0x%02lx\n", pAd->CommonCfg.DebugFlags));
		}
#endif /* DBG_CTRL_SUPPORT */

		/*IEEE80211H*/
		if(RTMPGetKeyParameter("IEEE80211H", tmpbuf, 10, pBuffer, TRUE))
		{
		    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		    {
				if(simple_strtol(macptr, 0, 10) != 0)  /*Enable*/
					pAd->CommonCfg.bIEEE80211H = TRUE;
				else /*Disable*/
                {            
					pAd->CommonCfg.bIEEE80211H = FALSE;
                    
#ifdef BACKGROUND_SCAN_SUPPORT                    
                    pAd->BgndScanCtrl.DfsZeroWaitSupport = FALSE;                    
#endif
#ifdef MT_DFS_SUPPORT
                    pAd->DfsParameter.bDfsEnable = FALSE;
                    UPDATE_MT_ZEROWAIT_DFS_Support(pAd, FALSE);
                    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]Disable DFS/Zero wait=%d/%d\n", 
                                                                        __FUNCTION__, 
                                                                        IS_SUPPORT_MT_DFS(pAd), 
                                                                        IS_SUPPORT_MT_ZEROWAIT_DFS(pAd)));
#endif
                }
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IEEE80211H=%d\n", pAd->CommonCfg.bIEEE80211H));
		    }
		}

#ifdef DFS_SUPPORT
	{
		/*CSPeriod*/
		if(RTMPGetKeyParameter("CSPeriod", tmpbuf, 10, pBuffer, TRUE))
		{
		    if(simple_strtol(tmpbuf, 0, 10) != 0)
				pAd->Dot11_H.CSPeriod = simple_strtol(tmpbuf, 0, 10);
			else
				pAd->Dot11_H.CSPeriod = 0;

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CSPeriod=%d\n", pAd->Dot11_H.CSPeriod));
		}

		}
#endif /* DFS_SUPPORT */

		/*RDRegion*/
		if(RTMPGetKeyParameter("RDRegion", tmpbuf, 128, pBuffer, TRUE))
		{
			if ((strncmp(tmpbuf, "JAP_W53", 7) == 0) || (strncmp(tmpbuf, "jap_w53", 7) == 0))
			{
							pAd->CommonCfg.RDDurRegion = JAP_W53;
							/*pRadarDetect->DfsSessionTime = 15;*/
			}
			else if ((strncmp(tmpbuf, "JAP_W56", 7) == 0) || (strncmp(tmpbuf, "jap_w56", 7) == 0))
			{
							pAd->CommonCfg.RDDurRegion = JAP_W56;
							/*pRadarDetect->DfsSessionTime = 13;*/
			}
			else if ((strncmp(tmpbuf, "JAP", 3) == 0) || (strncmp(tmpbuf, "jap", 3) == 0))
			{
							pAd->CommonCfg.RDDurRegion = JAP;
							/*pRadarDetect->DfsSessionTime = 5;*/
			}
			else  if ((strncmp(tmpbuf, "FCC", 3) == 0) || (strncmp(tmpbuf, "fcc", 3) == 0))
			{
							pAd->CommonCfg.RDDurRegion = FCC;
							/*pRadarDetect->DfsSessionTime = 5;*/
			}
			else if ((strncmp(tmpbuf, "CE", 2) == 0) || (strncmp(tmpbuf, "ce", 2) == 0))
			{
							pAd->CommonCfg.RDDurRegion = CE;
							/*pRadarDetect->DfsSessionTime = 13;*/
			}
			else
			{
							pAd->CommonCfg.RDDurRegion = CE;
							/*pRadarDetect->DfsSessionTime = 13;*/
			}

						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RDRegion=%d\n", pAd->CommonCfg.RDDurRegion));
		}
		else
		{
			pAd->CommonCfg.RDDurRegion = CE;
			/*pRadarDetect->DfsSessionTime = 13;*/
		}

#ifdef SYSTEM_LOG_SUPPORT
		/*WirelessEvent*/
		if(RTMPGetKeyParameter("WirelessEvent", tmpbuf, 10, pBuffer, TRUE))
		{
			BOOLEAN FlgIsWEntSup = FALSE;

			if(simple_strtol(tmpbuf, 0, 10) != 0)
				FlgIsWEntSup = TRUE;

			RtmpOsWlanEventSet(pAd, &pAd->CommonCfg.bWirelessEvent, FlgIsWEntSup);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("WirelessEvent=%d\n", pAd->CommonCfg.bWirelessEvent));
		}
#endif /* SYSTEM_LOG_SUPPORT */


 		/*Security Parameters */
		ReadSecurityParameterFromFile(pAd, tmpbuf, pBuffer);
 


#ifdef CONFIG_AP_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
				{
					/*Access Control List*/
					rtmp_read_acl_parms_from_file(pAd, tmpbuf, pBuffer);

#ifdef APCLI_SUPPORT
					rtmp_read_ap_client_from_file(pAd, tmpbuf, pBuffer);
#endif /* APCLI_SUPPORT */

#ifdef IGMP_SNOOP_SUPPORT
					/* Igmp Snooping information*/
					rtmp_read_igmp_snoop_from_file(pAd, tmpbuf, pBuffer);
#endif /* IGMP_SNOOP_SUPPORT */

#ifdef WDS_SUPPORT
					rtmp_read_wds_from_file(pAd, tmpbuf, pBuffer);
#endif /* WDS_SUPPORT */

#ifdef IDS_SUPPORT
					rtmp_read_ids_from_file(pAd, tmpbuf, pBuffer);
#endif /* IDS_SUPPORT */

#ifdef MWDS
					rtmp_read_MWDS_from_file(pAd, tmpbuf, pBuffer);
#endif /* MWDS */

#ifdef MAC_REPEATER_SUPPORT
					if (RTMPGetKeyParameter("MACRepeaterEn", tmpbuf, 10, pBuffer, FALSE))
					{
                        BOOLEAN bEnable = FALSE;
						if(simple_strtol(tmpbuf, 0, 10) != 0)
							bEnable = TRUE;
						else
							bEnable = FALSE;

						AsicSetReptFuncEnable(pAd, bEnable);
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MACRepeaterEn=%d\n", pAd->ApCfg.bMACRepeaterEn));
                        /* Disable DFS zero wait support */
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
                        if (pAd->ApCfg.bMACRepeaterEn)
                        {
                             pAd->BgndScanCtrl.DfsZeroWaitSupport = FALSE;
                             UPDATE_MT_ZEROWAIT_DFS_Support(pAd, FALSE);
                             MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m%s:Disable DfsZeroWait\x1b[m\n", __FUNCTION__));
                        }
#endif /* defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT) */
					}

					if (RTMPGetKeyParameter("MACRepeaterOuiMode", tmpbuf, 10, pBuffer, FALSE))
					{
						INT OuiMode = simple_strtol(tmpbuf, 0, 10);

						if (OuiMode == CASUALLY_DEFINE_MAC_ADDR)
							pAd->ApCfg.MACRepeaterOuiMode = CASUALLY_DEFINE_MAC_ADDR;
						else if (OuiMode == VENDOR_DEFINED_MAC_ADDR_OUI)
							pAd->ApCfg.MACRepeaterOuiMode = VENDOR_DEFINED_MAC_ADDR_OUI; /* customer specific */
						else
							pAd->ApCfg.MACRepeaterOuiMode = FOLLOW_CLI_LINK_MAC_ADDR_OUI; /* use Ap-Client first 3 bytes MAC assress (default) */

						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MACRepeaterOuiMode=%d\n", pAd->ApCfg.MACRepeaterOuiMode));

					}
#endif /* MAC_REPEATER_SUPPORT */
				}

#endif /* CONFIG_AP_SUPPORT */

				if(RTMPGetKeyParameter("SE_OFF", tmpbuf, 25, pBuffer, TRUE))
				{
					ULONG SeOff = simple_strtol(tmpbuf, 0, 10);

					pAd->CommonCfg.bSeOff = SeOff > 0 ? TRUE : FALSE;
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): SE_OFF=%d\n",
								__FUNCTION__, pAd->CommonCfg.bSeOff));
				}

				if(RTMPGetKeyParameter("AntennaIndex", tmpbuf, 25, pBuffer, TRUE))
				{
					ULONG antenna_index = simple_strtol(tmpbuf, 0, 10);

					if (antenna_index > 28)
					{
						antenna_index = 0;
					}
					if (antenna_index == 24 || antenna_index == 25)
					{
						antenna_index = 0;                        
					}

					pAd->CommonCfg.ucAntennaIndex = antenna_index;
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): antenna_index=%d\n",
								__FUNCTION__, pAd->CommonCfg.ucAntennaIndex));
				}

#ifdef DOT11_N_SUPPORT
				HTParametersHook(pAd, tmpbuf, pBuffer);
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
				VHTParametersHook(pAd, tmpbuf, pBuffer);
#endif /* DOT11_VHT_AC */

#ifdef CONFIG_FPGA_MODE
				if(RTMPGetKeyParameter("FPGA_ON", tmpbuf, 25, pBuffer, TRUE))
				{
					ULONG fpga_on = simple_strtol(tmpbuf, 0, 10);

					pAd->fpga_ctl.fpga_on = fpga_on > 0 ? fpga_on : 0;
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): FPGA_ON=%d\n",
								__FUNCTION__, pAd->fpga_ctl.fpga_on));
				}

#ifdef CAPTURE_MODE
				if(RTMPGetKeyParameter("cap_support", tmpbuf, 25, pBuffer, TRUE))
				{
					ULONG cap_support = simple_strtol(tmpbuf, 0, 10);

					pAd->fpga_ctl.cap_support = cap_support > 0 ? TRUE : FALSE;
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): CaptureMode=%d\n",
								__FUNCTION__, pAd->fpga_ctl.cap_support));
				}
#endif /* CAPTURE_MODE */
#endif /* CONFIG_FPGA_MODE */

#ifdef CONFIG_AP_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
				{
#ifdef WSC_AP_SUPPORT
					RTMP_STRING tok_str[16] = {0};
					for (i = 0; i < pAd->ApCfg.BssidNum; i++)
					{
						snprintf(tok_str, sizeof(tok_str), "WscDefaultSSID%d", i + 1);
						if(RTMPGetKeyParameter(tok_str, tmpbuf, 33, pBuffer, FALSE))
						{
							NdisZeroMemory(&pAd->ApCfg.MBSSID[i].WscControl.WscDefaultSsid, sizeof(NDIS_802_11_SSID));
							NdisMoveMemory(pAd->ApCfg.MBSSID[i].WscControl.WscDefaultSsid.Ssid, tmpbuf , strlen(tmpbuf));
							pAd->ApCfg.MBSSID[i].WscControl.WscDefaultSsid.SsidLength = strlen(tmpbuf);
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("WscDefaultSSID[%d]=%s\n", i, pAd->ApCfg.MBSSID[i].WscControl.WscDefaultSsid.Ssid));
						}
					}

					/*WscConfMode*/
					if(RTMPGetKeyParameter("WscConfMode", tmpbuf, 32, pBuffer, TRUE))
					{
						for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
						{
							INT WscConfMode = simple_strtol(macptr, 0, 10);

							if (i >= pAd->ApCfg.BssidNum)
								break;

							if (WscConfMode > 0 && WscConfMode < 8)
							{
								pAd->ApCfg.MBSSID[i].WscControl.WscConfMode = WscConfMode;
							}
							else
							{
								pAd->ApCfg.MBSSID[i].WscControl.WscConfMode = WSC_DISABLE;
							}

							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(ra%d) WscConfMode=%d\n", i, pAd->ApCfg.MBSSID[i].WscControl.WscConfMode));
						}
					}

					/*WscConfStatus*/
					if(RTMPGetKeyParameter("WscConfStatus", tmpbuf, 32, pBuffer, TRUE))
					{
						for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
						{
							if (i >= pAd->ApCfg.BssidNum)
								break;

							pAd->ApCfg.MBSSID[i].WscControl.WscConfStatus = (INT) simple_strtol(macptr, 0, 10);
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(ra%d) WscConfStatus=%d\n", i, pAd->ApCfg.MBSSID[i].WscControl.WscConfStatus));
						}
					}
					/*WscConfMethods*/
					if(RTMPGetKeyParameter("WscConfMethods", tmpbuf, 32, pBuffer, TRUE))
					{
						for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
						{
							if (i >= pAd->ApCfg.BssidNum)
								break;

							pAd->ApCfg.MBSSID[i].WscControl.WscConfigMethods = (USHORT)simple_strtol(macptr, 0, 16);
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(ra%d) WscConfMethods=0x%x\n", i, pAd->ApCfg.MBSSID[i].WscControl.WscConfigMethods));
						}
					}

					/*WscKeyASCII (0:Hex, 1:ASCII(random length), others: ASCII length, default 8)*/
					if (RTMPGetKeyParameter("WscKeyASCII", tmpbuf, 10, pBuffer, TRUE))
					{
						for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
						{
							INT Value;

							if (i >= pAd->ApCfg.BssidNum)
								break;

							Value = (INT) simple_strtol(tmpbuf, 0, 10);
							if(Value==0 || Value==1)
								pAd->ApCfg.MBSSID[i].WscControl.WscKeyASCII = Value;
							else if(Value >= 8 && Value <=63)
								pAd->ApCfg.MBSSID[i].WscControl.WscKeyASCII = Value;
							else
								pAd->ApCfg.MBSSID[i].WscControl.WscKeyASCII = 8;
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("WscKeyASCII=%d\n", pAd->ApCfg.MBSSID[i].WscControl.WscKeyASCII));
						}
					}

					if (RTMPGetKeyParameter("WscSecurityMode", tmpbuf, 50, pBuffer, TRUE))
					{
						for (i= 0; i<pAd->ApCfg.BssidNum; i++)
							pAd->ApCfg.MBSSID[i].WscSecurityMode = WPAPSKTKIP;

						for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
						{
							INT tmpMode = 0;

							if (i >= pAd->ApCfg.BssidNum)
								break;

							tmpMode = (INT) simple_strtol(macptr, 0, 10);
							if (tmpMode <= WPAPSKTKIP)
								pAd->ApCfg.MBSSID[i].WscSecurityMode = tmpMode;
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPSetProfileParameters I/F(ra%d) WscSecurityMode=%d\n",
								i, pAd->ApCfg.MBSSID[i].WscSecurityMode));
						}
					}

					/* WCNTest*/
					if(RTMPGetKeyParameter("WCNTest", tmpbuf, 10, pBuffer, TRUE))
					{
						BOOLEAN	bEn = FALSE;

						if ((strncmp(tmpbuf, "0", 1) == 0))
							bEn = FALSE;
						else
							bEn = TRUE;

						for (i = 0; i < pAd->ApCfg.BssidNum; i++)
						{
							pAd->ApCfg.MBSSID[i].WscControl.bWCNTest = bEn;
						}
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("WCNTest=%d\n", bEn));
					}

					/*WSC UUID Str*/
					for (i = 0; i < pAd->ApCfg.BssidNum; i++)
					{
						PWSC_CTRL	pWpsCtrl = &pAd->ApCfg.MBSSID[i].WscControl;
						snprintf(tok_str, sizeof(tok_str), "WSC_UUID_Str%d", i + 1);
						if(RTMPGetKeyParameter(tok_str, tmpbuf, 40, pBuffer, FALSE))
						{
							NdisMoveMemory(&pWpsCtrl->Wsc_Uuid_Str[0], tmpbuf , strlen(tmpbuf));
					    	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("UUID_Str[%d]=%s\n", i+1, pWpsCtrl->Wsc_Uuid_Str));
						}
					}

					/*WSC UUID Hex*/
					for (i = 0; i < pAd->ApCfg.BssidNum; i++)
					{
						PWSC_CTRL	pWpsCtrl = &pAd->ApCfg.MBSSID[i].WscControl;
						snprintf(tok_str, sizeof(tok_str), "WSC_UUID_E%d", i + 1);
						if(RTMPGetKeyParameter(tok_str, tmpbuf, 40, pBuffer, FALSE))
						{
							AtoH(tmpbuf, &pWpsCtrl->Wsc_Uuid_E[0], UUID_LEN_HEX);
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Wsc_Uuid_E[%d]", i+1));
							hex_dump("", &pWpsCtrl->Wsc_Uuid_E[0], UUID_LEN_HEX);
						}
					}

					/* WSC AutoTrigger Disable */
					if(RTMPGetKeyParameter("WscAutoTriggerDisable", tmpbuf, 10, pBuffer, TRUE))
					{
						BOOLEAN	bEn = FALSE;

						if ((strncmp(tmpbuf, "0", 1) == 0))
							bEn = FALSE;
						else
							bEn = TRUE;

						for (i = 0; i < pAd->ApCfg.BssidNum; i++)
						{
							pAd->ApCfg.MBSSID[i].WscControl.bWscAutoTriggerDisable = bEn;
						}
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("bWscAutoTriggerDisable=%d\n", bEn));
					}

		
#endif /* WSC_AP_SUPPORT */


#ifdef WH_EZ_SETUP
					ez_read_parms_from_file(pAd, tmpbuf, pBuffer);			
#endif /* WH_EZ_SETUP */

#ifdef STA_FORCE_ROAM_SUPPORT
					if(RTMPGetKeyParameter("ForceRoamSupport", tmpbuf, 10, pBuffer, TRUE))	// use as global OR in multi-profile??
					{
						if((strlen(tmpbuf) > 0) && (strlen(tmpbuf) <= 3)){

							macptr = rstrtok(tmpbuf,";");

							if( macptr!= NULL){

								if(strlen(macptr) == 1){
									INT Value = 0;

									Value = simple_strtol(macptr, 0, 10);

									if((Value == 0) || (Value == 1))
										pAd->en_force_roam_supp = Value;
				
									MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("I/F(ra0) ForceRoamSupport=%d\n",pAd->en_force_roam_supp));	
								}
							}
						}
					}
#endif

				}
#ifdef APCLI_SUPPORT
#ifdef ROAMING_ENHANCE_SUPPORT
                if (RTMPGetKeyParameter("RoamingEnhance", tmpbuf, 32, pBuffer, TRUE))
            	{
            		for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
            		{
            			pAd->ApCfg.bRoamingEnhance = (simple_strtol(tmpbuf, 0, 10) > 0)?TRUE:FALSE;
            		}        	
            	}
#endif /* ROAMING_ENHANCE_SUPPORT */
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef CARRIER_DETECTION_SUPPORT
					/*CarrierDetect*/
					if(RTMPGetKeyParameter("CarrierDetect", tmpbuf, 128, pBuffer, TRUE))
					{
						if ((strncmp(tmpbuf, "0", 1) == 0))
							pAd->CommonCfg.CarrierDetect.Enable = FALSE;
						else if ((strncmp(tmpbuf, "1", 1) == 0))
							pAd->CommonCfg.CarrierDetect.Enable = TRUE;
						else
							pAd->CommonCfg.CarrierDetect.Enable = FALSE;

						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CarrierDetect.Enable=%d\n", pAd->CommonCfg.CarrierDetect.Enable));
					}
					else
						pAd->CommonCfg.CarrierDetect.Enable = FALSE;
#endif /* CARRIER_DETECTION_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
				{
#ifdef MCAST_RATE_SPECIFIC
					/* McastPhyMode*/
					if (RTMPGetKeyParameter("McastPhyMode", tmpbuf, 32, pBuffer, TRUE))
					{
						UCHAR PhyMode;
						HTTRANSMIT_SETTING *pTransmit;
						struct wifi_dev *wdev = get_default_wdev(pAd);
						UCHAR ht_bw = wlan_config_get_ht_bw(wdev);
						for (i=0, macptr = rstrtok(tmpbuf,";"); (macptr && i < 2); macptr = rstrtok(NULL,";"), i++)
						{
							PhyMode = (UCHAR)simple_strtol(macptr, 0, 10);
							printk("%s: Mcast frame, i=%d,  Mode=%d!\n", __FUNCTION__, i, PhyMode);						
							//UCHAR PhyMode = simple_strtol(tmpbuf, 0, 10);
							pTransmit = (i==0) ? (&pAd->CommonCfg.MCastPhyMode) : (&pAd->CommonCfg.MCastPhyMode_5G);
							
							pTransmit->field.BW = ht_bw;
							switch (PhyMode)
							{
							case MCAST_DISABLE: // disable
								NdisMoveMemory(pTransmit,
												&pAd->MacTab.Content[MCAST_WCID].HTPhyMode, sizeof(HTTRANSMIT_SETTING));
								if (i == 0)
								{
									pTransmit->field.MODE = MODE_CCK;
									pTransmit->field.BW =  BW_20;
									pTransmit->field.MCS = RATE_1;
								} else {
									pTransmit->field.MODE = MODE_OFDM;
									pTransmit->field.BW =  BW_20;
									pTransmit->field.MCS = OfdmRateToRxwiMCS[RATE_6];
								}
								break;

							case MCAST_CCK:	/* CCK*/
								if (i == 0)
								{
									pTransmit->field.MODE = MODE_CCK;
									pTransmit->field.BW =  BW_20;
								} else {
									MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Could not set CCK mode for 5G band so set OFDM!\n"));
									pTransmit->field.MODE = MODE_OFDM;
									pTransmit->field.BW =  BW_20;
									//pTransmit->field.MCS = OfdmRateToRxwiMCS[RATE_6];
								}
								break;

							case MCAST_OFDM:	/* OFDM*/
								pTransmit->field.MODE = MODE_OFDM;
								pTransmit->field.BW =  BW_20;
								break;
#ifdef DOT11_N_SUPPORT
							case MCAST_HTMIX:	/* HTMIX*/
								pTransmit->field.MODE = MODE_HTMIX;
								break;
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC
							case MCAST_VHT: /* VHT */
								pTransmit->field.MODE = MODE_VHT;
								break;
#endif /* DOT11_VHT_AC */

							default:
								MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Unknown Multicast PhyMode %d.\n", PhyMode));
								MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Set the default mode, MCAST_CCK!\n"));
								pTransmit->field.MODE = MODE_CCK;
								pTransmit->field.BW =  BW_20;
								break;
							}
						}
						if (i == 0)
						{
							memset(&pAd->CommonCfg.MCastPhyMode, 0, sizeof(HTTRANSMIT_SETTING));
							memset(&pAd->CommonCfg.MCastPhyMode_5G, 0, sizeof(HTTRANSMIT_SETTING));
						} else if (i == 1)   // single band
						{
							NdisMoveMemory(&pAd->CommonCfg.MCastPhyMode_5G,
												&pAd->CommonCfg.MCastPhyMode, sizeof(HTTRANSMIT_SETTING));
							if (pAd->CommonCfg.MCastPhyMode_5G.field.MODE == MODE_CCK)
								pAd->CommonCfg.MCastPhyMode_5G.field.MODE = MODE_OFDM;
						}
					} else {
						/*
						NdisMoveMemory(&pAd->CommonCfg.MCastPhyMode,
										&pAd->MacTab.Content[MCAST_WCID].HTPhyMode, sizeof(HTTRANSMIT_SETTING));
										*/
						memset(&pAd->CommonCfg.MCastPhyMode, 0, sizeof(HTTRANSMIT_SETTING));
						memset(&pAd->CommonCfg.MCastPhyMode_5G, 0, sizeof(HTTRANSMIT_SETTING));
						//printk("%s: Zero McastPhyMode!\n", __FUNCTION__);
					}

					/* McastMcs*/
					if (RTMPGetKeyParameter("McastMcs", tmpbuf, 32, pBuffer, TRUE))
					{
						HTTRANSMIT_SETTING *pTransmit;
						UCHAR Mcs;
						for (i=0, macptr = rstrtok(tmpbuf,";"); (macptr && i < 2); macptr = rstrtok(NULL,";"), i++)
						{
							Mcs = (UCHAR)simple_strtol(macptr, 0, 10);
							pTransmit = (i==0) ? (&pAd->CommonCfg.MCastPhyMode) : (&pAd->CommonCfg.MCastPhyMode_5G);
							
							printk("%s: Mcast frame, i=%d,  MCS=%d!\n", __FUNCTION__, i, Mcs);						
							switch(pTransmit->field.MODE)
							{
							case MODE_CCK:
								if (i == 1)
								{
									MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Could not set CCK mode for 5G band!\n"));
									break;
								}
								if ((Mcs <= 3) || (Mcs >= 8 && Mcs <= 11))
									pTransmit->field.MCS = Mcs;
								else
									MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MCS must in range of 0 ~ 3 and 8 ~ 11 for CCK Mode.\n"));
								break;

							case MODE_OFDM:
								if (Mcs > 7)
									MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MCS must in range from 0 to 7 for OFDM Mode.\n"));
								else
									pTransmit->field.MCS = Mcs;
								break;

							default:
								pTransmit->field.MCS = Mcs;
								break;
							}
						}
					} else {
						pAd->CommonCfg.MCastPhyMode.field.MCS = RATE_1;
						pAd->CommonCfg.MCastPhyMode_5G.field.MCS = OfdmRateToRxwiMCS[RATE_6];
					}
					/*
					printk("%s: Mcast Mode=%d %d, BW=%d %d, MCS=%d %d\n", __FUNCTION__, 
						pAd->CommonCfg.MCastPhyMode.field.MODE, pAd->CommonCfg.MCastPhyMode_5G.field.MODE,
						pAd->CommonCfg.MCastPhyMode.field.BW, pAd->CommonCfg.MCastPhyMode_5G.field.BW,
						pAd->CommonCfg.MCastPhyMode.field.MCS,  pAd->CommonCfg.MCastPhyMode_5G.field.MCS);
					*/
#endif /* MCAST_RATE_SPECIFIC */
				}
#endif /* CONFIG_AP_SUPPORT */


#ifdef WSC_INCLUDED

				rtmp_read_wsc_user_parms_from_file(pAd, tmpbuf, pBuffer);

				/* Wsc4digitPinCode = TRUE use 4-digit Pin code, otherwise 8-digit Pin code */
				if (RTMPGetKeyParameter("Wsc4digitPinCode", tmpbuf, 32, pBuffer, TRUE))
				{
#ifdef CONFIG_AP_SUPPORT
					IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
					{
						for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
						{
							if (i >= pAd->ApCfg.BssidNum)
								break;

							if (simple_strtol(macptr, 0, 10) != 0)	//Enable
								pAd->ApCfg.MBSSID[i].WscControl.WscEnrollee4digitPinCode = TRUE;
							else //Disable
								pAd->ApCfg.MBSSID[i].WscControl.WscEnrollee4digitPinCode = FALSE;

							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(ra%d) Wsc4digitPinCode=%d\n", i, pAd->ApCfg.MBSSID[i].WscControl.WscEnrollee4digitPinCode));
						}

					}
#endif // CONFIG_AP_SUPPORT //
				}

				if (RTMPGetKeyParameter("WscVendorPinCode", tmpbuf, 256, pBuffer, TRUE))
				{
					PWSC_CTRL pWscContrl;
					int bSetOk;
#ifdef CONFIG_AP_SUPPORT
					IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
					{
						pWscContrl = &pAd->ApCfg.MBSSID[BSS0].WscControl;
					}
#endif /* CONFIG_AP_SUPPORT */
					bSetOk = RT_CfgSetWscPinCode(pAd, tmpbuf, pWscContrl);
					if (bSetOk)
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s - WscVendorPinCode= (%d)\n", __FUNCTION__, bSetOk));
						else
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s - WscVendorPinCode: invalid pin code(%s)\n", __FUNCTION__, tmpbuf));
				}
#ifdef WSC_V2_SUPPORT
				if (RTMPGetKeyParameter("WscV2Support", tmpbuf, 32, pBuffer, TRUE))
				{
					UCHAR 			bEnable;
#ifdef CONFIG_AP_SUPPORT
					IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
					{
						for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
						{
							if (i >= pAd->ApCfg.BssidNum)
								break;
							bEnable = (UCHAR)simple_strtol(macptr, 0, 10);
							pAd->ApCfg.MBSSID[i].WscControl.WscV2Info.bEnableWpsV2 = bEnable;
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(ra%d) WscV2Support=%d\n", i, bEnable));
						}
					}
#endif /* CONFIG_AP_SUPPORT */
				}
#endif /* WSC_V2_SUPPORT */


#endif /* WSC_INCLUDED */

#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11R_FT_SUPPORT
				FT_rtmp_read_parameters_from_file(pAd, tmpbuf, pBuffer);
#endif /* DOT11R_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
				/* EntryLifeCheck is used to check */
				if (RTMPGetKeyParameter("EntryLifeCheck", tmpbuf, 256, pBuffer, TRUE))
				{
					long LifeCheckCnt = simple_strtol(tmpbuf, 0, 10);
					if ((LifeCheckCnt <= 65535) && (LifeCheckCnt != 0))
						pAd->ApCfg.EntryLifeCheck = LifeCheckCnt;
					else
						pAd->ApCfg.EntryLifeCheck = MAC_ENTRY_LIFE_CHECK_CNT;

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("EntryLifeCheck=%ld\n", pAd->ApCfg.EntryLifeCheck));
				}

#ifdef DOT11K_RRM_SUPPORT
				RRM_ReadParametersFromFile(pAd, tmpbuf, pBuffer);
#endif /* DOT11K_RRM_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


#ifdef SINGLE_SKU
				if(RTMPGetKeyParameter("AntGain", tmpbuf, 10, pBuffer, TRUE))
				{
					UCHAR AntGain = simple_strtol(tmpbuf, 0, 10);
					pAd->CommonCfg.AntGain= AntGain;

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("AntGain=%d\n", pAd->CommonCfg.AntGain));
				}
				if(RTMPGetKeyParameter("BandedgeDelta", tmpbuf, 10, pBuffer, TRUE))
				{
					UCHAR Bandedge = simple_strtol(tmpbuf, 0, 10);
					pAd->CommonCfg.BandedgeDelta = Bandedge;

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BandedgeDelta=%d\n", pAd->CommonCfg.BandedgeDelta));
				}
#endif /* SINGLE_SKU */


#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)
		/* set GPIO pin for wake-up signal */
		if (RTMPGetKeyParameter("WOW_GPIO", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_GPIO(pAd, tmpbuf);

		/* set WOW enable/disable */
		if (RTMPGetKeyParameter("WOW_Enable", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_Enable(pAd, tmpbuf);

		/* set delay time for WOW really enable */
		if (RTMPGetKeyParameter("WOW_Delay", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_Delay(pAd, tmpbuf);

		/* set GPIO pulse hold time */
		if (RTMPGetKeyParameter("WOW_Hold", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_Hold(pAd, tmpbuf);

		/* set wakeup signal type */
		if (RTMPGetKeyParameter("WOW_InBand", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_InBand(pAd, tmpbuf);

		/* set wakeup interface */
		if (RTMPGetKeyParameter("WOW_Interface", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_Interface(pAd, tmpbuf);

		/* set if down interface */
		if (RTMPGetKeyParameter("WOW_IfDown_Support", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_IfDown_Support(pAd, tmpbuf);

		/* set GPIO High Low */
		if (RTMPGetKeyParameter("WOW_GPIOHighLow", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_GPIOHighLow(pAd, tmpbuf);

#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT) */

#ifdef MICROWAVE_OVEN_SUPPORT
		if (RTMPGetKeyParameter("MO_FalseCCATh", tmpbuf, 10, pBuffer, TRUE))
			Set_MO_FalseCCATh_Proc(pAd, tmpbuf);
#endif /* MICROWAVE_OVEN_SUPPORT */


      if (RTMPGetKeyParameter("PS_RETRIEVE", tmpbuf, 10, pBuffer, TRUE))
      {
         long PS_RETRIEVE;
         PS_RETRIEVE = simple_strtol(tmpbuf, 0, 10);
         pAd->bPS_Retrieve = PS_RETRIEVE;
         MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PS_RETRIEVE = %lx\n",PS_RETRIEVE));
      }

#ifdef FW_DUMP_SUPPORT
		if (RTMPGetKeyParameter("FWDump_Path", tmpbuf, 10, pBuffer, TRUE))
        {
			set_fwdump_path(pAd, tmpbuf);
        }

		if (RTMPGetKeyParameter("FWDump_MaxSize", tmpbuf, 10, pBuffer, TRUE))
        {
			set_fwdump_max_size(pAd, tmpbuf);
        }
#endif

#ifdef INTERNAL_CAPTURE_SUPPORT
	if (RTMPGetKeyParameter("IcapMode", tmpbuf, 10, pBuffer, TRUE))
      {
		UINT8 IcapMode; /* 0: Normal Mode; 1:Internal Capture; 2:Wifi Spectrum */
		IcapMode = simple_strtol(tmpbuf, 0, 10);
		pAd->IcapMode = IcapMode;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("IcapMode = %d\n",IcapMode));
	}
#endif /* INTERNAL_CAPTURE_SUPPORT */

#ifdef VOW_SUPPORT
#ifdef CONFIG_AP_SUPPORT
    rtmp_read_vow_parms_from_file(pAd, tmpbuf, pBuffer);
#endif /* CONFIG_AP_SUPPORT */
#endif /* VOW_SUPPORT */

#ifdef RED_SUPPORT
	rtmp_read_red_parms_from_file(pAd, tmpbuf, pBuffer);
#endif /* RED_SUPPORT */
	rtmp_read_cp_parms_from_file(pAd, tmpbuf, pBuffer);

#ifdef LINUX_NET_TXQ_SUPPORT
	if(RTMPGetKeyParameter("NET_TXQ_LEN", tmpbuf, 128, pBuffer,TRUE) && (strlen(tmpbuf) > 0))
	{
	    	pAd->tx_net_queue_len =  (UINT16)simple_strtol(tmpbuf, 0, 10);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("TX queue len --> %u\n", 
                                    pAd->tx_net_queue_len));
	}
#endif /* LINUX_NET_TXQ_SUPPORT */

	}while(0);

	os_free_mem( tmpbuf);

	return NDIS_STATUS_SUCCESS;
}

#ifdef MULTIPLE_CARD_SUPPORT
/* record whether the card in the card list is used in the card file*/
UINT8 MC_CardUsed[MAX_NUM_OF_MULTIPLE_CARD] = {0};
/* record used card mac address in the card list*/
static UINT8  MC_CardMac[MAX_NUM_OF_MULTIPLE_CARD][6];

BOOLEAN get_mac_from_eeprom(RTMP_ADAPTER *pAd, UCHAR *mac)
{
	USHORT addr, ee_addr;
	INT idx;

	for (ee_addr = 0x04, idx = 0; ee_addr <= 0x08; ee_addr += 2, idx +=2) {
		RT28xx_EEPROM_READ16(pAd, ee_addr, addr);
		mac[idx] = (UCHAR)(addr & 0xff);
		mac[idx + 1] = (UCHAR)(addr >> 8);
	}

	return TRUE;
}


#ifdef RTMP_FLASH_SUPPORT
#define EEPROM_SEG_IN_NVM 2	/* segments for EEPROM in flash */
#endif /* RTMP_FLASH_SUPPORT */

/*
========================================================================
Routine Description:
	Get card profile path.

Arguments:
	pAd

Return Value:
	TRUE		- Find a card profile
	FALSE		- use default profile

Note:
========================================================================
*/
BOOLEAN RTMP_CardInfoRead(
	IN	PRTMP_ADAPTER pAd)
{
#define MC_SELECT_CARDID		0	/* use CARD ID (0 ~ 31) to identify different cards */
#define MC_SELECT_MAC			1	/* use CARD MAC to identify different cards */
#define MC_SELECT_CARDTYPE		2	/* use CARD type (abgn or bgn) to identify different cards */

#define LETTER_CASE_TRANSLATE(txt_p, card_id)			\
	{	UINT32 _len; char _char;						\
		for(_len=0; _len<strlen(card_id); _len++) {		\
			_char = *(txt_p + _len);					\
			if (('A' <= _char) && (_char <= 'Z'))		\
				*(txt_p+_len) = 'a'+(_char-'A');		\
		} }

	RTMP_OS_FD srcf;
	INT retval;
	RTMP_STRING *buffer, *tmpbuf;
	RTMP_STRING card_id_buf[30], RFIC_word[30];
	BOOLEAN flg_match_ok = FALSE;
	INT32 card_select_method;
	INT32 card_free_id, card_nouse_id, card_same_mac_id, card_match_id;
	EEPROM_ANTENNA_STRUC antenna;
	USHORT addr01, addr23, addr45;
	UINT8 mac[6];
#ifdef RTMP_FLASH_SUPPORT
	UINT8 mac_maybe[EEPROM_SEG_IN_NVM][MAC_ADDR_LEN];
	INT segment = 0;
#endif /* RTMP_FLASH_SUPPORT */
	UINT32 data, card_index;
	UCHAR *start_ptr;
	RTMP_OS_FS_INFO osFSInfo;

	/* init*/
	os_alloc_mem(NULL, (UCHAR **)&buffer, MAX_INI_BUFFER_SIZE);
	if (buffer == NULL)
		return FALSE;

	os_alloc_mem(NULL, (UCHAR **)&tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if(tmpbuf == NULL)
	{
		os_free_mem( buffer);
		return NDIS_STATUS_FAILURE;
	}

	RT28xx_EEPROM_READ16(pAd, EEPROM_NIC1_OFFSET, antenna.word);

	if ((antenna.field.RfIcType == RFIC_2850) ||
		(antenna.field.RfIcType == RFIC_2750) ||
		(antenna.field.RfIcType == RFIC_2853) ||
		(antenna.field.RfIcType == RFIC_3853) ||
		(antenna.field.RfIcType == RFIC_7650) ||
		(antenna.field.RfIcType == RFIC_7610U) ||
		(antenna.field.RfIcType == RFIC_5592))
	{
		/* ABGN card */
		strcpy(RFIC_word, "abgn");
	}
	else if (antenna.field.RfIcType == RFIC_7610E)
	{
		/* ABGN card */
		strcpy(RFIC_word, "an");
	}
	else
	{
		/* BGN card */
		strcpy(RFIC_word, "bgn");
	}

	/* get MAC address*/
#ifdef RTMP_FLASH_SUPPORT
	RtmpFlashRead(&mac_maybe[0][0], 0x40000 + 0x04, MAC_ADDR_LEN);
	RtmpFlashRead(&mac_maybe[1][0], 0x48000 + 0x04, MAC_ADDR_LEN);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("mac addr1 in flash=%02x:%02x:%02x:%02x:%02x:%02x!\n", PRINT_MAC(mac_maybe[0])));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("mac addr2 in flash=%02x:%02x:%02x:%02x:%02x:%02x!\n", PRINT_MAC(mac_maybe[1])));
#endif /* RTMP_FLASH_SUPPORT */

	get_mac_from_eeprom(pAd, &mac[0]);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("mac addr=%02x:%02x:%02x:%02x:%02x:%02x!\n", PRINT_MAC(mac)));

	RtmpOSFSInfoChange(&osFSInfo, TRUE);
	/* open card information file*/
	srcf = RtmpOSFileOpen(CARD_INFO_PATH, O_RDONLY, 0);
	if (IS_FILE_OPEN_ERR(srcf))
	{
		/* card information file does not exist */
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("--> Error opening %s\n", CARD_INFO_PATH));
		goto  free_resource;
	}

	/* card information file exists so reading the card information */
	memset(buffer, 0x00, MAX_INI_BUFFER_SIZE);
	retval = RtmpOSFileRead(srcf, buffer, MAX_INI_BUFFER_SIZE);
	if (retval < 0)
	{
		/* read fail */
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("--> Read %s error %d\n", CARD_INFO_PATH, -retval));
	}
	else
	{
		/* get card selection method */
		memset(tmpbuf, 0x00, MAX_PARAM_BUFFER_SIZE);
		card_select_method = MC_SELECT_CARDTYPE; /* default*/

		if (RTMPGetKeyParameter("SELECT", tmpbuf, 256, buffer, TRUE))
		{
			if (strcmp(tmpbuf, "CARDID") == 0)
				card_select_method = MC_SELECT_CARDID;
			else if (strcmp(tmpbuf, "MAC") == 0)
				card_select_method = MC_SELECT_MAC;
			else if (strcmp(tmpbuf, "CARDTYPE") == 0)
				card_select_method = MC_SELECT_CARDTYPE;
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("MC> Card Selection = %d\n", card_select_method));

		/* init*/
		card_free_id = -1;
		card_nouse_id = -1;
		card_same_mac_id = -1;
		card_match_id = -1;

		/* search current card information records*/
		for (card_index=0; card_index<MAX_NUM_OF_MULTIPLE_CARD; card_index++)
		{
			if ((*(UINT32 *)&MC_CardMac[card_index][0] == 0) &&
				(*(UINT16 *)&MC_CardMac[card_index][4] == 0))
			{
				/* MAC is all-0 so the entry is available*/
				MC_CardUsed[card_index] = 0;
				if (card_free_id < 0)
					card_free_id = card_index; /* 1st free entry*/
			}
			else
			{
				if (memcmp(MC_CardMac[card_index], mac, 6) == 0)
				{
					/* we find the entry with same MAC*/
					if (card_same_mac_id < 0)
						card_same_mac_id = card_index; /* 1st same entry*/
				}
				else
				{
					/* MAC is not all-0 but used flag == 0*/
					if ((MC_CardUsed[card_index] == 0) &&
						(card_nouse_id < 0))
					{
						card_nouse_id = card_index; /* 1st available entry*/
					}
				}
			}
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("MC> Free = %d, Same = %d, NOUSE = %d\n",
				card_free_id, card_same_mac_id, card_nouse_id));

		if ((card_same_mac_id >= 0) &&
			((card_select_method == MC_SELECT_CARDID) ||
			(card_select_method == MC_SELECT_CARDTYPE)))
		{
			/* same MAC entry is found*/
			card_match_id = card_same_mac_id;

			if (card_select_method == MC_SELECT_CARDTYPE)
			{
				/* for CARDTYPE*/
				snprintf(card_id_buf, sizeof(card_id_buf), "%02dCARDTYPE%s",
						card_match_id, RFIC_word);

				if ((start_ptr = (PUCHAR)rtstrstruncasecmp(buffer, card_id_buf)) != NULL)
				{
					/* we found the card ID*/
					LETTER_CASE_TRANSLATE(start_ptr, card_id_buf);
				}
			}
		}
		else
		{
			/* the card is 1st plug-in, try to find the match card profile*/
			switch(card_select_method)
			{
				case MC_SELECT_CARDID: /* CARDID*/
				default:
					if (card_free_id >= 0)
						card_match_id = card_free_id;
					else
						card_match_id = card_nouse_id;
					break;

				case MC_SELECT_MAC: /* MAC*/
#ifdef RTMP_FLASH_SUPPORT
					memcpy(&mac[0], &mac_maybe[segment][0], MAC_ADDR_LEN);
#endif /* RTMP_FLASH_SUPPORT */
					snprintf(card_id_buf, sizeof(card_id_buf), "MAC%02x:%02x:%02x:%02x:%02x:%02x",
							mac[0], mac[1], mac[2],
							mac[3], mac[4], mac[5]);

					/* try to find the key word in the card file */
					if ((start_ptr = (PUCHAR)rtstrstruncasecmp(buffer, card_id_buf)) != NULL)
					{
						LETTER_CASE_TRANSLATE(start_ptr, card_id_buf);

						/* get the row ID (2 ASCII characters) */
						start_ptr -= 2;
						card_id_buf[0] = *(start_ptr);
						card_id_buf[1] = *(start_ptr+1);
						card_id_buf[2] = 0x00;
						card_match_id = simple_strtol(card_id_buf, 0, 10);
#ifdef RTMP_FLASH_SUPPORT
						if (MC_CardUsed[card_match_id] == 1)
						{
							/* try again to find the key word in the card file by the other MAC address */
							segment += 1;
							memcpy(&mac[0], &mac_maybe[segment][0], MAC_ADDR_LEN);
							snprintf(card_id_buf, sizeof(card_id_buf), "MAC%02x:%02x:%02x:%02x:%02x:%02x",
									mac[0], mac[1], mac[2],
									mac[3], mac[4], mac[5]);

							if ((start_ptr = (PUCHAR)rtstrstruncasecmp(buffer, card_id_buf)) != NULL)
							{
								LETTER_CASE_TRANSLATE(start_ptr, card_id_buf);

								/* get the row ID (2 ASCII characters) */
								start_ptr -= 2;
								card_id_buf[0] = *(start_ptr);
								card_id_buf[1] = *(start_ptr+1);
								card_id_buf[2] = 0x00;
								card_match_id = simple_strtol(card_id_buf, 0, 10);
							}
						}
#endif /* RTMP_FLASH_SUPPORT */
					}
					break;

				case MC_SELECT_CARDTYPE: /* CARDTYPE*/
					card_nouse_id = -1;

					for(card_index=0;
						card_index<MAX_NUM_OF_MULTIPLE_CARD;
						card_index++)
					{
						snprintf(card_id_buf, sizeof(card_id_buf), "%02dCARDTYPE%s",
								card_index, RFIC_word);

						if ((start_ptr = (PUCHAR)rtstrstruncasecmp(buffer,
													card_id_buf)) != NULL)
						{
							LETTER_CASE_TRANSLATE(start_ptr, card_id_buf);

							if (MC_CardUsed[card_index] == 0)
							{
								/* current the card profile is not used */
								if ((*(UINT32 *)&MC_CardMac[card_index][0] == 0) &&
									(*(UINT16 *)&MC_CardMac[card_index][4] == 0))
								{
									/* find it and no previous card use it*/
									card_match_id = card_index;
									break;
								}
								else
								{
									/* ever a card use it*/
									if (card_nouse_id < 0)
										card_nouse_id = card_index;
								}
							}
						}
					}

					/* if not find a free one, use the available one*/
					if (card_match_id < 0)
						card_match_id = card_nouse_id;
					break;
			}
		}

		if (card_match_id >= 0)
		{
			/* make up search keyword*/
			switch(card_select_method)
			{
				case MC_SELECT_CARDID: /* CARDID*/
					snprintf(card_id_buf, sizeof(card_id_buf), "%02dCARDID", card_match_id);
					break;

				case MC_SELECT_MAC: /* MAC*/
					snprintf(card_id_buf, sizeof(card_id_buf),
							"%02dmac%02x:%02x:%02x:%02x:%02x:%02x",
							card_match_id,
							mac[0], mac[1], mac[2],
							mac[3], mac[4], mac[5]);
					break;

				case MC_SELECT_CARDTYPE: /* CARDTYPE*/
				default:
					snprintf(card_id_buf, sizeof(card_id_buf), "%02dcardtype%s",
							card_match_id, RFIC_word);
					break;
			}

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Search Keyword = %s\n", card_id_buf));

			/* read card file path*/
			if (RTMPGetKeyParameter(card_id_buf, tmpbuf, 256, buffer, TRUE))
			{
				if (strlen(tmpbuf) < sizeof(pAd->MC_FileName))
				{
					/* backup card information*/
					pAd->MC_RowID = card_match_id; /* base 0 */
					/*
						If we are run in Multicard mode, the eeinit shall execute
						here instead of NICReadEEPROMParameters()
					*/
					if (pAd->chipOps.eeinit)
						pAd->chipOps.eeinit(pAd);

					get_mac_from_eeprom(pAd, &mac[0]);
					MC_CardUsed[card_match_id] = 1;
					memcpy(MC_CardMac[card_match_id], mac, sizeof(mac));

					/* backup card file path*/
					NdisMoveMemory(pAd->MC_FileName, tmpbuf , strlen(tmpbuf));
					pAd->MC_FileName[strlen(tmpbuf)] = '\0';
					flg_match_ok = TRUE;

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
							("Card Profile Name = %s\n", pAd->MC_FileName));
				}
				else
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("Card Profile Name length too large!\n"));
				}
			}
			else
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("Can not find search key word in card.dat!\n"));
			}

			if ((flg_match_ok != TRUE) &&
				(card_match_id < MAX_NUM_OF_MULTIPLE_CARD))
			{
				MC_CardUsed[card_match_id] = 0;
				memset(MC_CardMac[card_match_id], 0, sizeof(mac));
			}
		} /* if (card_match_id >= 0)*/
		}


/* close file*/
retval = RtmpOSFileClose(srcf);

free_resource:
RtmpOSFSInfoChange(&osFSInfo, FALSE);
os_free_mem( buffer);
os_free_mem( tmpbuf);

return flg_match_ok;
}
#endif /* MULTIPLE_CARD_SUPPORT */


#ifdef WSC_INCLUDED
void rtmp_read_wsc_user_parms(
	PWSC_CTRL pWscControl,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *buffer)
{
		if(RTMPGetKeyParameter("WscManufacturer", tmpbuf, WSC_MANUFACTURE_LEN, buffer,TRUE))
	{
		NdisZeroMemory(pWscControl->RegData.SelfInfo.Manufacturer, WSC_MANUFACTURE_LEN);
		NdisMoveMemory(pWscControl->RegData.SelfInfo.Manufacturer, tmpbuf, strlen(tmpbuf));
			if(pWscControl->RegData.SelfInfo.Manufacturer[0] != 0x00)
		RTMP_SET_FLAG(pWscControl, 0x01);
	}

	/*WSC_User_ModelName*/
		if(RTMPGetKeyParameter("WscModelName", tmpbuf, WSC_MODELNAME_LEN, buffer,TRUE))
	{
		NdisZeroMemory(pWscControl->RegData.SelfInfo.ModelName, WSC_MODELNAME_LEN);
		NdisMoveMemory(pWscControl->RegData.SelfInfo.ModelName, tmpbuf, strlen(tmpbuf));
			if(pWscControl->RegData.SelfInfo.ModelName[0] != 0x00)
		RTMP_SET_FLAG(pWscControl, 0x02);
	}

	/*WSC_User_DeviceName*/
		if(RTMPGetKeyParameter("WscDeviceName", tmpbuf, WSC_DEVICENAME_LEN, buffer,TRUE))
	{
		NdisZeroMemory(pWscControl->RegData.SelfInfo.DeviceName, WSC_DEVICENAME_LEN);
		NdisMoveMemory(pWscControl->RegData.SelfInfo.DeviceName, tmpbuf, strlen(tmpbuf));
			if(pWscControl->RegData.SelfInfo.DeviceName[0] != 0x00)
		RTMP_SET_FLAG(pWscControl, 0x04);
	}

	/*WSC_User_ModelNumber*/
		if(RTMPGetKeyParameter("WscModelNumber", tmpbuf, WSC_MODELNUNBER_LEN, buffer,TRUE))
	{
		NdisZeroMemory(pWscControl->RegData.SelfInfo.ModelNumber, WSC_MODELNUNBER_LEN);
		NdisMoveMemory(pWscControl->RegData.SelfInfo.ModelNumber, tmpbuf, strlen(tmpbuf));
			if(pWscControl->RegData.SelfInfo.ModelNumber[0] != 0x00)
		RTMP_SET_FLAG(pWscControl, 0x08);
	}

	/*WSC_User_SerialNumber*/
		if(RTMPGetKeyParameter("WscSerialNumber", tmpbuf, WSC_SERIALNUNBER_LEN, buffer,TRUE))
	{
		NdisZeroMemory(pWscControl->RegData.SelfInfo.SerialNumber, WSC_SERIALNUNBER_LEN);
		NdisMoveMemory(pWscControl->RegData.SelfInfo.SerialNumber, tmpbuf, strlen(tmpbuf));
			if(pWscControl->RegData.SelfInfo.SerialNumber[0] != 0x00)
		RTMP_SET_FLAG(pWscControl, 0x10);
	}
}

void rtmp_read_wsc_user_parms_from_file(IN	PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
	PWSC_CTRL           pWscControl;

#ifdef WSC_AP_SUPPORT
	int i=0;
	for(i = 0; i < MAX_MBSSID_NUM(pAd); i++)
	{
		pWscControl = &pAd->ApCfg.MBSSID[i].WscControl;
		rtmp_read_wsc_user_parms(pWscControl, tmpbuf, buffer);
}
#ifdef APCLI_SUPPORT
	pWscControl = &pAd->ApCfg.ApCliTab[0].WscControl;
	rtmp_read_wsc_user_parms(pWscControl, tmpbuf, buffer);
#endif /* APCLI_SUPPORT */
#endif /* WSC_AP_SUPPORT */

#ifdef WSC_STA_SUPPORT
	pWscControl = &pAd->StaCfg[0].WscControl;
	rtmp_read_wsc_user_parms(pWscControl, tmpbuf, buffer);
#endif /* WSC_STA_SUPPORT */


}
#endif/*WSC_INCLUDED*/

#ifdef VOW_SUPPORT
#ifdef CONFIG_AP_SUPPORT
void rtmp_read_vow_parms_from_file(IN	PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
    UINT8 		i = 0, j = 0;
	CHAR 		*ptok = NULL;
	CHAR		*macptr; 
	CHAR		*tmp;
	CHAR		*pwatf_string;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: begin --> \n", 
                                    __FUNCTION__));

    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
    /* for enable/disable */
	if(RTMPGetKeyParameter("VOW_BW_Ctrl", tmpbuf, 128, buffer,TRUE) && (strlen(tmpbuf) > 0))
	{
	    pAd->vow_cfg.en_bw_ctrl =  simple_strtol(tmpbuf, 0, 10) != 0 ? TRUE : FALSE;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_BW_Ctrl --> %d\n", 
                                    pAd->vow_cfg.en_bw_ctrl));
	}

    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if(RTMPGetKeyParameter("VOW_Airtime_Fairness_En", tmpbuf, 128, buffer,TRUE) && (strlen(tmpbuf) > 0))
	{
	    pAd->vow_cfg.en_airtime_fairness =  simple_strtol(tmpbuf, 0, 10) != 0 ? TRUE : FALSE;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_Airtime_Fairness_En --> %d\n", 
                                    pAd->vow_cfg.en_airtime_fairness));
	}

    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if(RTMPGetKeyParameter("VOW_Airtime_Ctrl_En", tmpbuf, 128, buffer,TRUE) && (strlen(tmpbuf) > 0))
	{
		for (i = 0, ptok = rstrtok(tmpbuf,";"); ptok; ptok = rstrtok(NULL,";"), i++)
        {
			if (i >= pAd->ApCfg.BssidNum)
				break;

	        pAd->vow_bss_cfg[i].at_on =  simple_strtol(ptok, 0, 10) != 0 ? TRUE : FALSE;

		    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS(%d) VOW_Airtime_Ctrl_En --> %d\n", 
                                    i, pAd->vow_bss_cfg[i].at_on));
        }
	}

    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if(RTMPGetKeyParameter("VOW_Rate_Ctrl_En", tmpbuf, 128, buffer,TRUE) && (strlen(tmpbuf) > 0))
	{
		for (i = 0, ptok = rstrtok(tmpbuf,";"); ptok; ptok = rstrtok(NULL,";"), i++)
        {
			if (i >= pAd->ApCfg.BssidNum)
				break;

	        pAd->vow_bss_cfg[i].bw_on =  simple_strtol(ptok, 0, 10) != 0 ? TRUE : FALSE;

		    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS(%d) VOW_Rate_Ctrl_En --> %d\n", 
                                    i, pAd->vow_bss_cfg[i].bw_on));
        }
	}

    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if(RTMPGetKeyParameter("VOW_RX_En", tmpbuf, 128, buffer,TRUE) && (strlen(tmpbuf) > 0))
	{

        pAd->vow_rx_time_cfg.rx_time_en =  simple_strtol(tmpbuf, 0, 10) != 0 ? TRUE : FALSE;

	    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_RX_En --> %d\n", 
                                    pAd->vow_rx_time_cfg.rx_time_en));
	}

    /* for gorup setting */
    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if(RTMPGetKeyParameter("VOW_Group_Min_Rate", tmpbuf, 128, buffer,TRUE) && (strlen(tmpbuf) > 0))
	{
		for (i = 0, ptok = rstrtok(tmpbuf,";"); ptok; ptok = rstrtok(NULL,";"), i++)
        {
			if (i >= pAd->ApCfg.BssidNum)
				break;

	        pAd->vow_bss_cfg[i].min_rate =  (UINT16)simple_strtol(ptok, 0, 10);
            pAd->vow_bss_cfg[i].min_rate_token = vow_convert_rate_token(pAd, VOW_MIN, i);

		    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS(%d) VOW_Group_Min_Rate --> %d\n", 
                                    i, pAd->vow_bss_cfg[i].min_rate));
        }
	}

    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if(RTMPGetKeyParameter("VOW_Group_Max_Rate", tmpbuf, 128, buffer,TRUE) && (strlen(tmpbuf) > 0))
	{
		for (i = 0, ptok = rstrtok(tmpbuf,";"); ptok; ptok = rstrtok(NULL,";"), i++)
        {
			if (i >= pAd->ApCfg.BssidNum)
				break;

	        pAd->vow_bss_cfg[i].max_rate =  (UINT16)simple_strtol(ptok, 0, 10);
            pAd->vow_bss_cfg[i].max_rate_token = vow_convert_rate_token(pAd, VOW_MAX, i);

		    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS(%d) VOW_Group_Max_Rate --> %d\n", 
                                    i, pAd->vow_bss_cfg[i].max_rate));
        }
	}

    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if(RTMPGetKeyParameter("VOW_Group_Min_Ratio", tmpbuf, 128, buffer,TRUE) && (strlen(tmpbuf) > 0))
	{
		for (i = 0, ptok = rstrtok(tmpbuf,";"); ptok; ptok = rstrtok(NULL,";"), i++)
        {
			if (i >= pAd->ApCfg.BssidNum)
				break;

	        pAd->vow_bss_cfg[i].min_airtime_ratio =  (UINT8)simple_strtol(ptok, 0, 10);
            pAd->vow_bss_cfg[i].min_airtime_token = vow_convert_airtime_token(pAd, VOW_MIN, i);

		    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS(%d) VOW_Group_Min_Ratio --> %d\n", 
                                    i, pAd->vow_bss_cfg[i].min_airtime_ratio));
        }
	}

    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if(RTMPGetKeyParameter("VOW_Group_Max_Ratio", tmpbuf, 128, buffer,TRUE) && (strlen(tmpbuf) > 0))
	{
		for (i = 0, ptok = rstrtok(tmpbuf,";"); ptok; ptok = rstrtok(NULL,";"), i++)
        {
			if (i >= pAd->ApCfg.BssidNum)
				break;

	        pAd->vow_bss_cfg[i].max_airtime_ratio =  (UINT8)simple_strtol(ptok, 0, 10);
            pAd->vow_bss_cfg[i].max_airtime_token = vow_convert_airtime_token(pAd, VOW_MAX, i);

		    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS(%d) VOW_Group_Max_Ratio --> %d\n", 
                                    i, pAd->vow_bss_cfg[i].max_airtime_ratio));
        }
	}

    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if(RTMPGetKeyParameter("VOW_Refill_Period", tmpbuf, 128, buffer,TRUE) && (strlen(tmpbuf) > 0))
	{

        pAd->vow_cfg.refill_period =  (UINT8)simple_strtol(tmpbuf, 0, 10);

	    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_Refill_Period --> %d\n", 
                                    pAd->vow_cfg.refill_period));
	}

    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if(RTMPGetKeyParameter("VOW_Group_Min_Rate_Bucket_Size", tmpbuf, 128, buffer,TRUE) && (strlen(tmpbuf) > 0))
	{
		for (i = 0, ptok = rstrtok(tmpbuf,";"); ptok; ptok = rstrtok(NULL,";"), i++)
        {
			if (i >= pAd->ApCfg.BssidNum)
				break;

	        pAd->vow_bss_cfg[i].min_ratebucket_size =  (UINT16)simple_strtol(ptok, 0, 10);

		    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS(%d) VOW_Group_Min_Rate_Bucket_Size --> %d\n", 
                                    i, pAd->vow_bss_cfg[i].min_ratebucket_size));
        }
	}

    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if(RTMPGetKeyParameter("VOW_Group_Max_Rate_Bucket_Size", tmpbuf, 128, buffer,TRUE) && (strlen(tmpbuf) > 0))
	{
		for (i = 0, ptok = rstrtok(tmpbuf,";"); ptok; ptok = rstrtok(NULL,";"), i++)
        {
			if (i >= pAd->ApCfg.BssidNum)
				break;

	        pAd->vow_bss_cfg[i].max_ratebucket_size =  (UINT16)simple_strtol(ptok, 0, 10);

		    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS(%d) VOW_Group_Max_Rate_Bucket_Size --> %d\n", 
                                    i, pAd->vow_bss_cfg[i].max_ratebucket_size));
        }
	}

    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if(RTMPGetKeyParameter("VOW_Group_Min_Airtime_Bucket_Size", tmpbuf, 128, buffer,TRUE) && (strlen(tmpbuf) > 0))
	{
		for (i = 0, ptok = rstrtok(tmpbuf,";"); ptok; ptok = rstrtok(NULL,";"), i++)
        {
			if (i >= pAd->ApCfg.BssidNum)
				break;

	        pAd->vow_bss_cfg[i].min_airtimebucket_size =  (UINT8)simple_strtol(ptok, 0, 10);

		    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS(%d) VOW_Group_Min_Airtime_Bucket_Size --> %d\n", 
                                    i, pAd->vow_bss_cfg[i].min_airtimebucket_size));
        }
	}

    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if(RTMPGetKeyParameter("VOW_Group_Max_Airtime_Bucket_Size", tmpbuf, 128, buffer,TRUE) && (strlen(tmpbuf) > 0))
	{
		for (i = 0, ptok = rstrtok(tmpbuf,";"); ptok; ptok = rstrtok(NULL,";"), i++)
        {
			if (i >= pAd->ApCfg.BssidNum)
				break;

	        pAd->vow_bss_cfg[i].max_airtimebucket_size = (UINT8)simple_strtol(ptok, 0, 10);

		    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS(%d) VOW_Group_Max_Airtime_Bucket_Size --> %d\n", 
                                    i, pAd->vow_bss_cfg[i].max_airtimebucket_size));
        }
	}

    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if(RTMPGetKeyParameter("VOW_Group_Backlog", tmpbuf, 128, buffer,TRUE) && (strlen(tmpbuf) > 0))
	{
		for (i = 0, ptok = rstrtok(tmpbuf,";"); ptok; ptok = rstrtok(NULL,";"), i++)
        {
			if (i >= pAd->ApCfg.BssidNum)
				break;

	        pAd->vow_bss_cfg[i].max_backlog_size = (UINT16)simple_strtol(ptok, 0, 10);

		    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS(%d) VOW_Group_Backlog --> %d\n", 
                                    i, pAd->vow_bss_cfg[i].max_backlog_size));
        }
	}

    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if(RTMPGetKeyParameter("VOW_Group_Max_Wait_Time", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0))
	{
		for (i = 0, ptok = rstrtok(tmpbuf,";"); ptok; ptok = rstrtok(NULL,";"), i++)
        {
			if (i >= pAd->ApCfg.BssidNum)
				break;

	        pAd->vow_bss_cfg[i].max_wait_time = (UINT8)simple_strtol(ptok, 0, 10);

		    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS(%d) VOW_Group_Max_Wait_Time --> %d\n", 
                                    i, pAd->vow_bss_cfg[i].max_wait_time));
        }
	}

    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if(RTMPGetKeyParameter("VOW_Group_DWRR_Quantum", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0))
	{
		for (i = 0, ptok = rstrtok(tmpbuf,";"); ptok; ptok = rstrtok(NULL,";"), i++)
        {
			if (i >= pAd->ApCfg.BssidNum)
				break;

	        pAd->vow_bss_cfg[i].dwrr_quantum =  (UINT8)simple_strtol(ptok, 0, 10);

		    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BSS(%d) VOW_Group_DWRR_Quantum --> %d\n", 
                                    i, pAd->vow_bss_cfg[i].dwrr_quantum));
        }
	}

    /* for stations */
    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if(RTMPGetKeyParameter("VOW_Sta_VO_DWRR_Quantum", tmpbuf, 128, buffer,TRUE) && (strlen(tmpbuf) > 0))
	{

        for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++)
        {
            pAd->vow_sta_cfg[i].dwrr_quantum[WMM_AC_VO] = (UINT8)simple_strtol(tmpbuf, 0, 10);
        }

	    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_Sta_VO_DWRR_Quantum --> %d\n", 
                                    (UINT8)simple_strtol(tmpbuf, 0, 10)));
	}

    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if(RTMPGetKeyParameter("VOW_Sta_VI_DWRR_Quantum", tmpbuf, 128, buffer,TRUE) && (strlen(tmpbuf) > 0))
	{

        for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++)
        {
            pAd->vow_sta_cfg[i].dwrr_quantum[WMM_AC_VI] = (UINT8)simple_strtol(tmpbuf, 0, 10);
        }

	    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_Sta_VI_DWRR_Quantum --> %d\n", 
                                    (UINT8)simple_strtol(tmpbuf, 0, 10)));
	}

    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if(RTMPGetKeyParameter("VOW_Sta_BE_DWRR_Quantum", tmpbuf, 128, buffer,TRUE) && (strlen(tmpbuf) > 0))
	{

        for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++)
        {
            pAd->vow_sta_cfg[i].dwrr_quantum[WMM_AC_BE] = (UINT8)simple_strtol(tmpbuf, 0, 10);
        }

	    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_Sta_BE_DWRR_Quantum --> %d\n", 
                                    (UINT8)simple_strtol(tmpbuf, 0, 10)));
	}

    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if(RTMPGetKeyParameter("VOW_Sta_BK_DWRR_Quantum", tmpbuf, 128, buffer,TRUE) && (strlen(tmpbuf) > 0))
	{

        for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++)
        {
            pAd->vow_sta_cfg[i].dwrr_quantum[WMM_AC_BK] = (UINT8)simple_strtol(tmpbuf, 0, 10);
        }

	    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_Sta_BK_DWRR_Quantum --> %d\n", 
                                    (UINT8)simple_strtol(tmpbuf, 0, 10)));
	}

    /* for group/stations control */
    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if(RTMPGetKeyParameter("VOW_WMM_Search_Rule_Band0", tmpbuf, 128, buffer,TRUE) && (strlen(tmpbuf) > 0))
	{

        pAd->vow_cfg.dbdc0_search_rule =  simple_strtol(tmpbuf, 0, 10) ? 1 : 0;

	    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_WMM_Search_Rule_Band0 --> %d\n", 
                                    pAd->vow_cfg.dbdc0_search_rule));
	}

    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if(RTMPGetKeyParameter("VOW_WMM_Search_Rule_Band1", tmpbuf, 128, buffer,TRUE) && (strlen(tmpbuf) > 0))
	{

        pAd->vow_cfg.dbdc1_search_rule =  simple_strtol(tmpbuf, 0, 10) ? 1 : 0;

	    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_WMM_Search_Rule_Band1 --> %d\n", 
                                    pAd->vow_cfg.dbdc1_search_rule));
	}

    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if(RTMPGetKeyParameter("VOW_Sta_DWRR_Max_Wait_Time", tmpbuf, 128, buffer,TRUE) && (strlen(tmpbuf) > 0))
	{

        pAd->vow_cfg.sta_max_wait_time =  (UINT8)simple_strtol(tmpbuf, 0, 10);

	    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_Sta_DWRR_Max_Wait_Time --> %d\n", 
                                    pAd->vow_cfg.sta_max_wait_time));
	}

    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if(RTMPGetKeyParameter("VOW_Group_DWRR_Max_Wait_Time", tmpbuf, 128, buffer,TRUE) && (strlen(tmpbuf) > 0))
	{

        pAd->vow_cfg.group_max_wait_time =  (UINT8)simple_strtol(tmpbuf, 0, 10);

	    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_Group_DWRR_Max_Wait_Time --> %d\n", 
                                    pAd->vow_cfg.group_max_wait_time));
	}

	/* Weigthed Airtime Fairness - Enable/Disable*/
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if (RTMPGetKeyParameter("VOW_WATF_Enable", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE) && (strlen(tmpbuf) > 0))
	{
		pAd->vow_watf_en =  (UINT8)simple_strtol(tmpbuf, 0, 10);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_WATF_Enable --> %d\n", pAd->vow_watf_en));
	}

	if(pAd->vow_watf_en) 
	{
		/* Weigthed Airtime Fairness - Different DWRR quantum value*/
		NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

		if (RTMPGetKeyParameter("VOW_WATF_Q_LV0", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE) && (strlen(tmpbuf) > 0))
		{
			pAd->vow_watf_q_lv0 = (UINT8)simple_strtol(tmpbuf, 0, 10);
			pAd->vow_cfg.vow_sta_dwrr_quantum[0] = pAd->vow_watf_q_lv0;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VOW_WATF_Q_LV0 --> %d\n", pAd->vow_watf_q_lv0));
		}
		else
			pAd->vow_watf_q_lv0 = 4;

		NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

		if (RTMPGetKeyParameter("VOW_WATF_Q_LV1", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE) && (strlen(tmpbuf) > 0))
		{
			pAd->vow_watf_q_lv1 = (UINT8)simple_strtol(tmpbuf, 0, 10);
			pAd->vow_cfg.vow_sta_dwrr_quantum[1] = pAd->vow_watf_q_lv1;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VOW_WATF_Q_LV1 --> %d\n", pAd->vow_watf_q_lv1));
		}
		else
			pAd->vow_watf_q_lv1 = 8;

		NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
		
		if (RTMPGetKeyParameter("VOW_WATF_Q_LV2", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE) && (strlen(tmpbuf) > 0))
		{
			pAd->vow_watf_q_lv2 = (UINT8)simple_strtol(tmpbuf, 0, 10);
			pAd->vow_cfg.vow_sta_dwrr_quantum[2] = pAd->vow_watf_q_lv2;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VOW_WATF_Q_LV2 --> %d\n", pAd->vow_watf_q_lv2));
		}
		else
			pAd->vow_watf_q_lv2 = 12;

		NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
		
		if (RTMPGetKeyParameter("VOW_WATF_Q_LV3", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE) && (strlen(tmpbuf) > 0))
		{
			pAd->vow_watf_q_lv3 = (UINT8)simple_strtol(tmpbuf, 0, 10);
			pAd->vow_cfg.vow_sta_dwrr_quantum[3] = pAd->vow_watf_q_lv3;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VOW_WATF_Q_LV3 --> %d\n", pAd->vow_watf_q_lv3));
		}
		else
			pAd->vow_watf_q_lv3 = 16;


		/* Weigthed Airtime Fairness - Different DWRR quantum MAC address list*/
		NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
		os_alloc_mem(NULL, (UCHAR **)&pwatf_string, sizeof(32));
		os_alloc_mem(NULL, (UCHAR **)&tmp, sizeof(32));
		for(i=0; i<VOW_WATF_LEVEL_NUM; i++)
		{	
			sprintf(pwatf_string,"VOW_WATF_MAC_LV%d",i);
			if (RTMPGetKeyParameter(pwatf_string, tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE) && (pAd->vow_watf_en))
			{
				for (j=0, macptr = rstrtok(tmpbuf,","); macptr; macptr = rstrtok(NULL,","), j++) 
				{
					if (strlen(macptr) != 17)  /* Mac address acceptable format 01:02:03:04:05:06 length 17*/
						continue;

					sprintf(tmp,"%d-%s",i,macptr);
					set_vow_watf_add_entry(pAd,tmp);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d-%s",i,macptr));
				}
			}
		}
		if (pwatf_string != NULL)
			os_free_mem(pwatf_string);
		if (tmp != NULL)
			os_free_mem(tmp);
	}

	/* fast round robin */
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if (RTMPGetKeyParameter("VOW_STA_FRR_QUANTUM", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE) && (strlen(tmpbuf) > 0))
	{
		pAd->vow_sta_frr_quantum =  (UINT8)simple_strtol(tmpbuf, 0, 10);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VOW_STA_FRR_QUANTUM --> %d\n", pAd->vow_sta_frr_quantum));
	}

}
#endif /* CONFIG_AP_SUPPORT */
#endif  /*  VOW_SUPPORT */

#ifdef RED_SUPPORT
void rtmp_read_red_parms_from_file(IN	PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: begin --> \n", __FUNCTION__));

    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
    /* for enable/disable */
	if(RTMPGetKeyParameter("RED_Enable", tmpbuf, 128, buffer,TRUE) && (strlen(tmpbuf) > 0))
	{
	    pAd->red_en =  simple_strtol(tmpbuf, 0, 10) != 0 ? TRUE : FALSE;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RED_Enable --> %d\n", pAd->red_en));
	}
}
#endif  /*  RED_SUPPORT */

void rtmp_read_cp_parms_from_file(IN	PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: begin --> \n", __FUNCTION__));

    NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
    /* for enable/disable */
	if(RTMPGetKeyParameter("CP_SUPPORT", tmpbuf, 128, buffer,TRUE) && (strlen(tmpbuf) > 0))
	{
	    pAd->cp_support =  simple_strtol(tmpbuf, 0, 10);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CP_SUPPORT --> %d\n", pAd->cp_support));
	}
}

#ifdef SINGLE_SKU_V2
// TODO: shiang-usw, for MT76x0 series, currently cannot use this function!
NDIS_STATUS RTMPSetSingleSKUParameters(RTMP_ADAPTER *pAd)
{

	NDIS_STATUS ret;
#ifdef RTMP_INTERNAL_TX_ALC
	if (pAd->TxPowerCtrl.bInternalTxALC != TRUE)
#endif /* RTMP_INTERNAL_TX_ALC */
	{
	}

	ret = MtSingleSkuLoadParam(pAd);
	return ret;
}

#if defined(MT_MAC) && defined(TXBF_SUPPORT)
NDIS_STATUS RTMPSetBfBackOffParameters(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS ret;

    ret = MtBfBackOffLoadTable(pAd);
	return ret;
}
#endif /* defined(MT_MAC) && defined(TXBF_SUPPORT) */

NDIS_STATUS RTMPResetSingleSKUParameters(RTMP_ADAPTER *pAd)
{
	MtSingleSkuUnloadParam(pAd);
	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS RTMPResetBfBackOffTable(RTMP_ADAPTER *pAd)
{	
#ifdef MT_MAC
#ifdef TXBF_SUPPORT
	MtBfBackOffUnloadTable(pAd);
#endif /* TXBF_SUPPORT */
#endif /* MT_MAC */

	return NDIS_STATUS_SUCCESS;
}

UCHAR GetSkuChannelBasePwr(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR 			channel)
{
	CH_POWER *ch, *ch_temp;
	UCHAR start_ch;
	UCHAR base_pwr = pAd->DefaultTargetPwr;
	UINT8 i, j;

	DlListForEachSafe(ch, ch_temp, &pAd->SingleSkuPwrList, CH_POWER, List)
	{
		start_ch = ch->StartChannel;

		if ( channel >= start_ch )
		{
			for ( j = 0; j < ch->num; j++ )
			{
				if ( channel == ch->Channel[j] )
				{
					for ( i= 0 ; i < SINGLE_SKU_TABLE_CCK_LENGTH ; i++ )
					{
						if ( base_pwr > ch->PwrCCK[i] )
							base_pwr = ch->PwrCCK[i];
					}

					for ( i= 0 ; i < SINGLE_SKU_TABLE_OFDM_LENGTH ; i++ )
					{
						if ( base_pwr > ch->PwrOFDM[i] )
							base_pwr = ch->PwrOFDM[i];
					}

					for ( i= 0 ; i < SINGLE_SKU_TABLE_HT_LENGTH ; i++ )
					{
						if ( base_pwr > ch->PwrHT20[i] )
							base_pwr = ch->PwrHT20[i];
					}

					if (pAd->CommonCfg.BBPCurrentBW == BW_40)
					{
						for ( i= 0 ; i < SINGLE_SKU_TABLE_HT_LENGTH ; i++ )
						{
							if ( ch->PwrHT40[i] == 0 )
								break;

							if ( base_pwr > ch->PwrHT40[i] )
								base_pwr = ch->PwrHT40[i];
						}
					}
					break;
				}
			}
		}
	}

	return base_pwr;

}

#define	SKU_PHYMODE_CCK_1M_2M				0
#define	SKU_PHYMODE_CCK_5M_11M				1
#define	SKU_PHYMODE_OFDM_6M_9M				2
#define	SKU_PHYMODE_OFDM_12M_18M			3
#define	SKU_PHYMODE_OFDM_24M_36M			4
#define	SKU_PHYMODE_OFDM_48M_54M			5
#define	SKU_PHYMODE_HT_MCS0_MCS1			6
#define	SKU_PHYMODE_HT_MCS2_MCS3			7
#define	SKU_PHYMODE_HT_MCS4_MCS5			8
#define	SKU_PHYMODE_HT_MCS6_MCS7			9
#define	SKU_PHYMODE_HT_MCS8_MCS9			10
#define	SKU_PHYMODE_HT_MCS10_MCS11			11
#define	SKU_PHYMODE_HT_MCS12_MCS13			12
#define	SKU_PHYMODE_HT_MCS14_MCS15			13
#define	SKU_PHYMODE_STBC_MCS0_MCS1			14
#define	SKU_PHYMODE_STBC_MCS2_MCS3			15
#define	SKU_PHYMODE_STBC_MCS4_MCS5			16
#define	SKU_PHYMODE_STBC_MCS6_MCS7			17


VOID InitSkuRateDiffTable(
	IN PRTMP_ADAPTER 	pAd )
{
	USHORT		i, value;
	CHAR		BasePwr, Pwr;

	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 4, value);
	BasePwr = (value >> 8) & 0xFF;
	BasePwr = (BasePwr > 0x1F ) ? BasePwr - 0x40: BasePwr;

	for ( i = 0 ; i < 9; i++ )
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + i*2, value);
		Pwr = value & 0xFF ;
		Pwr = (Pwr > 0x1F ) ? Pwr - 0x40: Pwr;
		pAd->SingleSkuRatePwrDiff[i *2] = Pwr - BasePwr;
		Pwr = (value >> 8) & 0xFF;
		Pwr = (Pwr > 0x1F ) ? Pwr - 0x40: Pwr;
		pAd->SingleSkuRatePwrDiff[i *2 + 1] = Pwr - BasePwr;
	}
}


#ifdef RLT_MAC
INT32 GetSkuPAModePwr(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR			PAMode)
{
	INT32 pa_mode_pwr = 0;

	switch ( PAMode )
	{
		case SKU_PHYMODE_CCK_1M_2M:
			pa_mode_pwr = RF_PA_MODE_CCK_1M;
			break;
		case SKU_PHYMODE_CCK_5M_11M:
			pa_mode_pwr = RF_PA_MODE_CCK_5M;
			break;
		case SKU_PHYMODE_OFDM_6M_9M:
			pa_mode_pwr = RF_PA_MODE_OFDM_6M;
			break;
		case SKU_PHYMODE_OFDM_12M_18M:
			pa_mode_pwr = RF_PA_MODE_OFDM_12M;
			break;
		case SKU_PHYMODE_OFDM_24M_36M:
			pa_mode_pwr = RF_PA_MODE_OFDM_24M;
			break;
		case SKU_PHYMODE_OFDM_48M_54M:
			pa_mode_pwr = RF_PA_MODE_OFDM_48M;
			break;
		case SKU_PHYMODE_HT_MCS0_MCS1:
		case SKU_PHYMODE_STBC_MCS0_MCS1:
			pa_mode_pwr = RF_PA_MODE_HT_MCS0;
			break;
		case SKU_PHYMODE_HT_MCS2_MCS3:
		case SKU_PHYMODE_STBC_MCS2_MCS3:
			pa_mode_pwr = RF_PA_MODE_HT_MCS2;
			break;
		case SKU_PHYMODE_HT_MCS4_MCS5:
		case SKU_PHYMODE_STBC_MCS4_MCS5:
			pa_mode_pwr = RF_PA_MODE_HT_MCS4;
			break;
		case SKU_PHYMODE_HT_MCS6_MCS7:
		case SKU_PHYMODE_STBC_MCS6_MCS7:
			pa_mode_pwr = RF_PA_MODE_HT_MCS6;
			break;
		case SKU_PHYMODE_HT_MCS8_MCS9:
			pa_mode_pwr = RF_PA_MODE_HT_MCS8;
			break;
		case SKU_PHYMODE_HT_MCS10_MCS11:
			pa_mode_pwr = RF_PA_MODE_HT_MCS10;
			break;
		case SKU_PHYMODE_HT_MCS12_MCS13:
			pa_mode_pwr = RF_PA_MODE_HT_MCS12;
			break;
		case SKU_PHYMODE_HT_MCS14_MCS15:
			pa_mode_pwr = RF_PA_MODE_HT_MCS14;
			break;
		default:
			break;
	}

	return pa_mode_pwr;
}


UCHAR GetSkuRatePwr(
	IN PRTMP_ADAPTER 	pAd,
	IN CHAR 				phymode,
	IN UCHAR 			channel,
	IN UCHAR			bw)
{
	UINT8 i;
	CH_POWER *ch, *ch_temp;
	UCHAR start_ch;
	UCHAR rate_pwr = pAd->DefaultTargetPwr;
	UCHAR max_pwr;
	INT32 pwr_diff;

	DlListForEachSafe(ch, ch_temp, &pAd->SingleSkuPwrList, CH_POWER, List)
	{
		start_ch = ch->StartChannel;

		if ( channel >= start_ch )
		{
			for ( i = 0; i < ch->num; i++ )
			{
				if ( channel == ch->Channel[i] )
				{
					switch ( phymode )
					{
						case SKU_PHYMODE_CCK_1M_2M:
							rate_pwr = ch->PwrCCK[0];
							break;
						case SKU_PHYMODE_CCK_5M_11M:
							rate_pwr = ch->PwrCCK[2];
							break;
						case SKU_PHYMODE_OFDM_6M_9M:
							rate_pwr = ch->PwrOFDM[0];
							break;
						case SKU_PHYMODE_OFDM_12M_18M:
							rate_pwr = ch->PwrOFDM[2];
							break;
						case SKU_PHYMODE_OFDM_24M_36M:
							rate_pwr = ch->PwrOFDM[4];
							break;
						case SKU_PHYMODE_OFDM_48M_54M:
							rate_pwr = ch->PwrOFDM[6];
							break;
						case SKU_PHYMODE_HT_MCS0_MCS1:
						case SKU_PHYMODE_STBC_MCS0_MCS1:
							if ( bw == BW_20 )
								rate_pwr = ch->PwrHT20[0];
							else if ( bw == BW_40 )
								rate_pwr = ch->PwrHT40[0];
							break;
						case SKU_PHYMODE_HT_MCS2_MCS3:
						case SKU_PHYMODE_STBC_MCS2_MCS3:
							if ( bw == BW_20 )
								rate_pwr = ch->PwrHT20[2];
							else if ( bw == BW_40 )
								rate_pwr = ch->PwrHT40[2];
							break;
						case SKU_PHYMODE_HT_MCS4_MCS5:
						case SKU_PHYMODE_STBC_MCS4_MCS5:
							if ( bw == BW_20 )
								rate_pwr = ch->PwrHT20[4];
							else if ( bw == BW_40 )
								rate_pwr = ch->PwrHT40[4];
							break;
						case SKU_PHYMODE_HT_MCS6_MCS7:
						case SKU_PHYMODE_STBC_MCS6_MCS7:
							if ( bw == BW_20 )
								rate_pwr = ch->PwrHT20[6];
							else if ( bw == BW_40 )
								rate_pwr = ch->PwrHT40[6];
							break;
						case SKU_PHYMODE_HT_MCS8_MCS9:
							if ( bw == BW_20 )
								rate_pwr = ch->PwrHT20[8];
							else if ( bw == BW_40 )
								rate_pwr = ch->PwrHT40[8];
							break;
						case SKU_PHYMODE_HT_MCS10_MCS11:
							if ( bw == BW_20 )
								rate_pwr = ch->PwrHT20[10];
							else if ( bw == BW_40 )
								rate_pwr = ch->PwrHT40[10];
							break;
						case SKU_PHYMODE_HT_MCS12_MCS13:
							if ( bw == BW_20 )
								rate_pwr = ch->PwrHT20[12];
							else if ( bw == BW_40 )
								rate_pwr = ch->PwrHT40[12];
							break;
						case SKU_PHYMODE_HT_MCS14_MCS15:
							if ( bw == BW_20 )
								rate_pwr = ch->PwrHT20[14];
							else if ( bw == BW_40 )
								rate_pwr = ch->PwrHT40[14];
							break;
						default:
							break;
					}
					break;
				}
			}
		}
	}

	pwr_diff = GetSkuPAModePwr(pAd, phymode) + (pAd->SingleSkuRatePwrDiff[phymode] << 12) + 2048;
	pwr_diff = pwr_diff >> 12;
	max_pwr = pAd->DefaultTargetPwr + pwr_diff;

	if ( rate_pwr > max_pwr )
		rate_pwr = max_pwr;

	return rate_pwr;

}


VOID UpdateSkuRatePwr(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR 			channel,
	IN UCHAR			bw,
	IN CHAR				base_pwr)
{
	INT32	sku_rate_pwr;
	INT32	rate_pwr;
	INT32	mcs_digital_pwr, pa_mode_pwr, diff_pwr;
	UINT32	data, Adata, Gdata;
	UCHAR 	BBPR4, BBPR178;
	UCHAR	i;
	CHAR	rate_table[18];

	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("channel = %d, bw = %d\n", channel, bw));

	for ( i = 0 ; i < 18; i++ )
	{
		sku_rate_pwr = GetSkuRatePwr(pAd, i, channel, bw);
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("sku_rate_pwr = %d", sku_rate_pwr));
		sku_rate_pwr = sku_rate_pwr << 12;			// sku_rate_power * 4096
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tsku_rate_pwr = %d\n", sku_rate_pwr));

		if ( i < SKU_PHYMODE_CCK_5M_11M )
		{
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPR4);
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R178, &BBPR178);
			if ( BBPR4 & 0x20 )
			{
				if ( BBPR178 == 0 )
				{
					mcs_digital_pwr = 9830;		// 8192 * 1.2
				}
				else
				{
					mcs_digital_pwr = 18022;		// 8192 * 2.2
				}
			}
			else
			{
				if ( BBPR178 == 0 )
				{
					mcs_digital_pwr = 24576;		// 8192 * 3
				}
				else
				{
					mcs_digital_pwr = 819;			/// 8192 * 0.1
				}

			}
		}
		else
		{
			mcs_digital_pwr = 0;
		}

		pa_mode_pwr = GetSkuPAModePwr(pAd, i);

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("base_pwr = %d", base_pwr));
		rate_pwr = base_pwr << 12;
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t base_pwr = %d\n", rate_pwr));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("mcs_digital_pwr = %d\n", mcs_digital_pwr));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("pa_mode_pwr = %d\n", pa_mode_pwr));
		rate_pwr = rate_pwr + mcs_digital_pwr + pa_mode_pwr;
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("rate_pwr = %d\n", rate_pwr));
		diff_pwr = sku_rate_pwr - rate_pwr;
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("diff_pwr = %d", diff_pwr));
		diff_pwr = diff_pwr >> 12;
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tdiff_pwr = %d\n", diff_pwr));

		rate_table[i] = diff_pwr -1;
	}

	for ( i = 0 ; i < 5; i++ )
	{
		data = 0;
		Adata = 0;
		Gdata = 0;

		data = (rate_table[i*4] & 0x3F )+ ((rate_table[i*4 + 1] &0x3F) << 8);
		Adata = ((rate_table[i*4] + pAd->chipCap.Apwrdelta ) & 0x3F )+ ( ((rate_table[i*4 + 1] + pAd->chipCap.Apwrdelta) & 0x3F) << 8);
		Gdata = ((rate_table[i*4] + pAd->chipCap.Gpwrdelta ) & 0x3F ) + ( ((rate_table[i*4 + 1] + pAd->chipCap.Gpwrdelta) & 0x3F) << 8);

		if ( i != 4 )
		{
			data |= ((rate_table[i*4 + 2] &0x3F) << 16 )+ ((rate_table[i*4 + 3] & 0x3F) << 24);
			Adata |= ( ((rate_table[i*4 + 2] + pAd->chipCap.Apwrdelta ) & 0x3F) << 16) + ( ((rate_table[i*4 + 3] + pAd->chipCap.Apwrdelta) & 0x3F) << 24);
			Gdata |= ( ((rate_table[i*4 + 2] + pAd->chipCap.Gpwrdelta ) & 0x3F) << 16) + ( ((rate_table[i*4 + 3] + pAd->chipCap.Gpwrdelta) & 0x3F) << 24);
		}

		/* For 20M/40M Power Delta issue */
		pAd->Tx20MPwrCfgABand[i] = data;
		pAd->Tx20MPwrCfgGBand[i] = data;
		pAd->Tx40MPwrCfgABand[i] = Adata;
		pAd->Tx40MPwrCfgGBand[i] = Gdata;

		RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + i*4, data);

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("20MHz BW, 2.4G band-%08x,  Adata = %08x,  Gdata = %08x \n", data, Adata, Gdata));
	}

	/* Extra set MAC registers to compensate Tx power if any */
	RTMP_CHIP_ASIC_EXTRA_POWER_OVER_MAC(pAd);

}

#endif /*RTL_MAC*/

#endif /* SINGLE_SKU_V2 */


INT32 ralinkrate[] = {
/* CCK */
2, 4, 11, 22,
/* OFDM */
12, 18, 24, 36, 48, 72, 96, 108,
/* 20MHz, 800ns GI, MCS: 0 ~ 15 */
13, 26, 39, 52, 78, 104, 117, 130, 26, 52, 78, 104, 156, 208, 234, 260,
/* 20MHz, 800ns GI, MCS: 16 ~ 23 */
39, 78, 117, 156, 234, 312, 351, 390,
/* 40MHz, 800ns GI, MCS: 0 ~ 15 */
27, 54, 81, 108, 162, 216, 243, 270, 54, 108, 162, 216, 324, 432, 486, 540,
/* 40MHz, 800ns GI, MCS: 16 ~ 23 */
81, 162, 243, 324, 486, 648, 729, 810,
/* 20MHz, 400ns GI, MCS: 0 ~ 15 */
14, 29, 43, 57, 87, 115, 130, 144, 29, 59, 87, 115, 173, 230, 260, 288,
/* 20MHz, 400ns GI, MCS: 16 ~ 23 */
43, 87, 130, 173, 260, 317, 390, 433,
/* 40MHz, 400ns GI, MCS: 0 ~ 15 */
30, 60, 90, 120, 180, 240, 270, 300, 60, 120, 180, 240, 360, 480, 540, 600,
/* 40MHz, 400ns GI, MCS: 16 ~ 23 */
90, 180, 270, 360, 540, 720, 810, 900};

UINT32 RT_RateSize = sizeof(ralinkrate);
