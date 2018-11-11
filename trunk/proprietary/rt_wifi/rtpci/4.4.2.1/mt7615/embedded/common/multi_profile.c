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
	multi_profile.c
*/

#ifdef MULTI_PROFILE
#include "rt_config.h"

/*Local definition*/
#define FIRST_AP_2G_PROFILE_PATH	"/etc/Wireless/RT2860/RT2860AP.dat"
#define FIRST_AP_5G_PROFILE_PATH	"/etc/Wireless/iNIC/iNIC_ap.dat"
#define FIRST_AP_MERGE_PROFILE_PATH "/etc/Wireless/.mt7615_dbdc.dat"
#ifdef RT_CFG80211_SUPPORT
#define FIRST_AP_5G_DEVNAME "wlanx"
#else
#if defined(BB_SOC) && !defined(MULTI_INF_SUPPORT)
#define FIRST_AP_5G_DEVNAME "ra"
#else
#define FIRST_AP_5G_DEVNAME "rax"
#endif
#endif
#define SECOND_AP_2G_PROFILE_PATH	"/etc/Wireless/iNIC/iNIC_ap_2G.dat"
#define SECOND_AP_5G_PROFILE_PATH	"/etc/Wireless/iNIC/iNIC_ap_5G.dat"
#define SECOND_AP_MERGE_PROFILE_PATH "/etc/Wireless/RT2860/iNIC_ap_dbdc.dat"
#ifdef RT_CFG80211_SUPPORT
#define SECOND_AP_5G_DEVNAME "wlany"
#else
#if defined(BB_SOC) && !defined(MULTI_INF_SUPPORT)
#define SECOND_AP_5G_DEVNAME "ra"
#else
#define SECOND_AP_5G_DEVNAME "ray"
#endif
#endif
#define THIRD_AP_2G_PROFILE_PATH	"/etc/Wireless/WIFI3/RT2870AP_2G.dat"
#define THIRD_AP_5G_PROFILE_PATH	"/etc/Wireless/WIFI3/RT2870AP_5G.dat"
#define THIRD_AP_MERGE_PROFILE_PATH "/etc/Wireless/WIFI3/RT2870AP_dbdc.dat"
#ifdef RT_CFG80211_SUPPORT
#define THIRD_AP_5G_DEVNAME "wlanz"
#else
#define THIRD_AP_5G_DEVNAME "raz"
#endif


#define TEMP_STR_SIZE 256

struct mpf_data {
	UCHAR enable;
	UCHAR specific_dname;
	UCHAR pf1_num;
	UCHAR pf2_num;
	UCHAR total_num;
};

struct mpf_table {
	UCHAR *profile_2g;
	UCHAR *profile_5g;
	UCHAR *merge;
	UCHAR *prefix;
};

static struct mpf_table mtb[] = {
	{ FIRST_AP_2G_PROFILE_PATH, FIRST_AP_5G_PROFILE_PATH, FIRST_AP_MERGE_PROFILE_PATH, FIRST_AP_5G_DEVNAME },
	{ SECOND_AP_2G_PROFILE_PATH, SECOND_AP_5G_PROFILE_PATH, SECOND_AP_MERGE_PROFILE_PATH, SECOND_AP_5G_DEVNAME },
	{ THIRD_AP_2G_PROFILE_PATH, THIRD_AP_5G_PROFILE_PATH, THIRD_AP_MERGE_PROFILE_PATH, THIRD_AP_5G_DEVNAME },
};

#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
static INT multi_profile_merge_wsc(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final);
#endif /*MBSS_SUPPORT*/
#endif /*CONFIG_AP_SUPPORT*/
/*Local function body*/
/*
*
*/
static UCHAR *multi_profile_fname_get(struct _RTMP_ADAPTER *pAd, UCHAR profile_id)
{
	UCHAR *src = NULL;
	INT card_idx = 0;

#if defined(MT_FIRST_CARD) || defined(MT_SECOND_CARD) || defined(MT_THIRD_CARD)
	card_idx = get_dev_config_idx(pAd);
#endif /* MT_FIRST_CARD || MT_SECOND_CARD || MT_THIRD_CARD */

	if(profile_id == 2){
		src = mtb[card_idx].merge;
	}else{
		src = (profile_id == 0) ? mtb[card_idx].profile_2g : mtb[card_idx].profile_5g;
	}

	return src;
}

/*
* open & read profile 
*/
static INT multi_profile_read(CHAR *fname,CHAR *buf)
{
	INT retval = NDIS_STATUS_FAILURE;
	RTMP_OS_FD_EXT srcf;

	if(!fname)
		return retval;

	srcf = os_file_open(fname,O_RDONLY,0);
	if (srcf.Status)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Open file \"%s\" failed!\n", fname));
		return retval;
	}
	
	if (buf) {
		os_zero_mem(buf, MAX_INI_BUFFER_SIZE);
		retval =os_file_read(srcf, buf, MAX_INI_BUFFER_SIZE - 1);
		if (retval > 0)
		{
			retval = NDIS_STATUS_SUCCESS;
		}
		else
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Read file \"%s\" failed(errCode=%d)!\n", fname, retval));
	} else
		retval = NDIS_STATUS_FAILURE;

	if (os_file_close(srcf) != 0)
	{
		retval = NDIS_STATUS_FAILURE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Close file \"%s\" failed(errCode=%d)!\n", fname, retval));
	}
	return retval;
}

/*
* write merge profile for check
*/
static INT multi_profile_write(CHAR *fname,CHAR *buf)
{
	INT retval = NDIS_STATUS_FAILURE;	
	RTMP_OS_FD_EXT srcf;

	if(!fname)
		return retval;

	srcf = os_file_open(fname, O_WRONLY|O_CREAT,0);

	if (srcf.Status){
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Open file \"%s\" failed!\n", fname));
		return retval;
	}

	if (buf) {
		retval =os_file_write(srcf, buf, strlen(buf));
		if (retval > 0)
		{
			retval = NDIS_STATUS_SUCCESS;
		}
		else
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Write file \"%s\" failed(errCode=%d)!\n", fname, retval));
	} else
		retval = NDIS_STATUS_FAILURE;

	if (os_file_close(srcf) != 0)
	{
		retval = NDIS_STATUS_FAILURE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Close file \"%s\" failed(errCode=%d)!\n", fname, retval));
	}

	return retval;
}

/*
* replace function
*/
INT multi_profile_replace(CHAR *cha, CHAR *rep,CHAR *value)
{
	CHAR *token = NULL;
	while ( (token = strstr(value,cha)) != NULL){
		strncpy(token,rep,strlen(rep));
	}
	return NDIS_STATUS_SUCCESS;
}

/*
* Separate
*/
static INT multi_profile_merge_separate(
	UCHAR *parm,
	UCHAR *buf1,
	UCHAR *buf2,
	UCHAR *final)
{
	CHAR tmpbuf[TEMP_STR_SIZE]= "";
	CHAR tmpbuf2[TEMP_STR_SIZE]= "";
	CHAR value[TEMP_STR_SIZE]="";

	if (!buf1 || !buf2) 
		return NDIS_STATUS_FAILURE;

	if(RTMPGetKeyParameter(parm, tmpbuf, TEMP_STR_SIZE, buf1, TRUE) !=TRUE) {
		return NDIS_STATUS_SUCCESS;
	}

	if(RTMPGetKeyParameter(parm, tmpbuf2, TEMP_STR_SIZE, buf2, TRUE) != TRUE){
		return NDIS_STATUS_SUCCESS;
	}
	snprintf(value,sizeof(value),"%s;%s",tmpbuf,tmpbuf2);
	RTMPSetKeyParameter(parm,value, TEMP_STR_SIZE,final, TRUE);	 
	return NDIS_STATUS_SUCCESS;
}

#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
/*
*perband
*/
static INT multi_profile_merge_perband(
	struct mpf_data *data,
	UCHAR *parm,
	UCHAR *buf1,
	UCHAR *buf2,
	UCHAR *final)
{
	CHAR tmpbuf1[TEMP_STR_SIZE]= "";
	CHAR tmpbuf2[TEMP_STR_SIZE]= "";
	CHAR value[TEMP_STR_SIZE]="";
	UCHAR i, j;
	CHAR *tmpbuf;
	CHAR *macptr;

	if (!buf1 || !buf2) 
		return NDIS_STATUS_FAILURE;

	if(RTMPGetKeyParameter(parm, tmpbuf1, TEMP_STR_SIZE, buf1, TRUE) !=TRUE) {
		return NDIS_STATUS_SUCCESS;
	}

	if(RTMPGetKeyParameter(parm, tmpbuf2, TEMP_STR_SIZE, buf2, TRUE) != TRUE) {
		return NDIS_STATUS_SUCCESS;
	}
	os_zero_mem(value,sizeof(value));

	/*check number of perband parameter mode*/
	for (i = 0, macptr = rstrtok(tmpbuf1,";"); macptr; macptr = rstrtok(NULL,";"), i++){
		/*do nothing*/
	}
	for (j = 0, macptr = rstrtok(tmpbuf2,";"); macptr; macptr = rstrtok(NULL,";"), j++){
		/*do nothing*/
	}
	
	if(i > 1 || j > 1) {
		multi_profile_merge_separate(parm,buf1,buf2,final);
	} else {
		for(i = 0; i < data->total_num; i++) {
			tmpbuf = (i < data->pf1_num) ? tmpbuf1 : tmpbuf2;
			snprintf(value,sizeof(value),"%s%s;",value,tmpbuf);
		}
		RTMPSetKeyParameter(parm,value, TEMP_STR_SIZE,final, TRUE);
	}	
	
	return NDIS_STATUS_SUCCESS;
}


/*
* MACAddress$Idx
*/
static INT multi_profile_merge_mac_address(
	struct mpf_data *mpf,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	CHAR tmpbuf[25]="";
	CHAR tok_str[25]="";
	UCHAR i=0;
	UCHAR j=0;

	/* set file parameter to portcfg*/
	if (RTMPGetKeyParameter("MacAddress", tmpbuf, 25, buf2, TRUE)){
		snprintf(tok_str, sizeof(tok_str), "MacAddress%d", mpf->pf1_num);
		RTMPSetKeyParameter(tok_str, tmpbuf, 25, final,TRUE);
	}
	for (i = 1; i <= mpf->pf2_num ; i++){
		snprintf(tok_str, sizeof(tok_str), "MacAddress%d", i);
		if(RTMPGetKeyParameter(tok_str, tmpbuf, 25, buf2, TRUE)){
			j = i + mpf->pf1_num;
			snprintf(tok_str, sizeof(tok_str), "MacAddress%d",j);
			RTMPSetKeyParameter(tok_str, tmpbuf, 25, final,TRUE);
		}
	}
	return NDIS_STATUS_SUCCESS;
}

/*
* For increase from 1
*/
static INT multi_profile_merge_increase(
	struct mpf_data *mpf,
	UCHAR start_idx,
	CHAR *parm,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	CHAR tmpbuf[TEMP_STR_SIZE]="";
	CHAR tok_str[25]="";
	UCHAR i=0;
	UCHAR j=0;
	UCHAR k=0;

	/* set file parameter to portcfg*/
	for (i = 0; i < mpf->pf2_num; i++){
			k = start_idx + i;
		snprintf(tok_str, sizeof(tok_str), "%s%d",parm, k);
		if(RTMPGetKeyParameter(tok_str, tmpbuf, TEMP_STR_SIZE, buf2, TRUE)){
			j = k + mpf->pf1_num;
			snprintf(tok_str, sizeof(tok_str), "%s%d",parm,j);
			RTMPSetKeyParameter(tok_str, tmpbuf, TEMP_STR_SIZE, final,TRUE);
		}
	}
	return NDIS_STATUS_SUCCESS;
}

/*
* Key%dType  & Key%dStr%
*/
static INT multi_profile_merge_keytype(
	struct mpf_data *mpf,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	CHAR tmpbuf[TEMP_STR_SIZE]="";
	CHAR tok_str[25]="";
	UCHAR i=0;
	UCHAR j=0;
	UCHAR k=0;

	for(k=1; k <=4; k++){
		snprintf(tok_str, sizeof(tok_str), "Key%dType", k);
		multi_profile_merge_separate(tok_str,buf1,buf2,final);
	}
	/* set file parameter to keytype*/	
	for (i = 1; i <= mpf->pf2_num; i++){		
		j = i + mpf->pf1_num;
		for(k=1; k <=4; k++){
			snprintf(tok_str, sizeof(tok_str), "Key%dStr%d", k,i);
			if(RTMPGetKeyParameter(tok_str, tmpbuf, TEMP_STR_SIZE, buf2, TRUE)){
				snprintf(tok_str, sizeof(tok_str), "Key%dStr%d", k,j);
				RTMPSetKeyParameter(tok_str, tmpbuf, TEMP_STR_SIZE, final,TRUE);
			}
		}
	}
	return NDIS_STATUS_SUCCESS;
}

/*
* Security 
*/
static INT multi_profile_merge_security(
	struct mpf_data *mpf,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*IEEE8021X*/
	multi_profile_merge_separate("IEEE8021X",buf1,buf2,final);
	/*PreAuth*/	
	multi_profile_merge_separate("PreAuth",buf1,buf2,final);
	/*AuthMode*/	
	multi_profile_merge_separate("AuthMode",buf1,buf2,final);
	/*EncrypType*/	
	multi_profile_merge_separate("EncrypType",buf1,buf2,final);
	/*RekeyMethod*/	
	multi_profile_merge_separate("RekeyMethod",buf1,buf2,final);
	/*RekeyInterval*/	
	multi_profile_merge_separate("RekeyInterval",buf1,buf2,final);
	/*PMKCachePeriod*/	
	multi_profile_merge_separate("PMKCachePeriod",buf1,buf2,final);
	/*WPAPSK*/
	multi_profile_merge_increase(mpf,1,"WPAPSK",buf1,buf2,final);
	/*DefaultKeyID*/	
	multi_profile_merge_separate("DefaultKeyID",buf1,buf2,final);
	/*KeyType & KeyStr*/	
	multi_profile_merge_keytype(mpf,buf1,buf2,final);
	/*AccessPolicy */
	multi_profile_merge_increase(mpf,0,"AccessPolicy",buf1,buf2,final);
	/*AccessControlList*/
	multi_profile_merge_increase(mpf,0,"AccessControlList",buf1,buf2,final);
	/*RADIUS_Server*/
	multi_profile_merge_separate("RADIUS_Server",buf1,buf2,final);
	/*RADIUS_Port*/
	multi_profile_merge_separate("RADIUS_Port",buf1,buf2,final);
	/*RADIUS_Key*/
	multi_profile_merge_separate("RADIUS_Key",buf1,buf2,final);
	/*RADIUS Key%d*/
	multi_profile_merge_increase(mpf,1,"RADIUS_Key",buf1,buf2,final);


	/*PMFMFPC*/
	multi_profile_merge_separate("PMFMFPC",buf1,buf2,final);
	/*PMFMFPR*/
	multi_profile_merge_separate("PMFMFPR",buf1,buf2,final);
	/*PMFSHA256*/
	multi_profile_merge_separate("PMFSHA256",buf1,buf2,final);

	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
// TODO: related to multi_profile_merge_apedca
static INT multi_profile_merge_default_edca(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	UCHAR i;
	CHAR aifs[32]="";
	CHAR cwmin[32]="";
	CHAR cwmax[32]="";
	CHAR txop[32]="";
	CHAR acm[32]="";
	CHAR *buf = NULL;
	CHAR value[256]="";
	UCHAR idx;
	CHAR tok_str[25]="";

	for(i=0;i<2;i++){
		buf = (i==0) ? buf1 : buf2;
		os_zero_mem(value,sizeof(value));
		/*APAifsn*/
		if(RTMPGetKeyParameter("APAifsn", aifs, sizeof(aifs), buf, FALSE) != TRUE) {
			return NDIS_STATUS_FAILURE;
		}
		multi_profile_replace(";",",",aifs);
		/*APCwmin*/
		if(RTMPGetKeyParameter("APCwmin", cwmin, sizeof(cwmin), buf, FALSE) != TRUE) {
			return NDIS_STATUS_FAILURE;
		}
		multi_profile_replace(";",",",cwmin);
		/*APCwmax*/
		if(RTMPGetKeyParameter("APCwmax", cwmax, sizeof(cwmax), buf, FALSE) != TRUE) {
			return NDIS_STATUS_FAILURE;
		}
		multi_profile_replace(";",",",cwmax);
		/*APTxop*/
		if(RTMPGetKeyParameter("APTxop", txop, sizeof(txop), buf, FALSE) != TRUE) {
			return NDIS_STATUS_FAILURE;
		}
		multi_profile_replace(";",",",txop);
		/*APACM*/
		if(RTMPGetKeyParameter("APACM", acm, sizeof(acm), buf, FALSE) != TRUE) {
			return NDIS_STATUS_FAILURE;
		}
		multi_profile_replace(";",",",acm);
		/*merge*/
		snprintf(value,sizeof(value),"1;%s;%s;%s;%s;%s",aifs,cwmin,cwmax,txop,acm);
		/*set*/
		snprintf(tok_str,sizeof(tok_str),"APEdca%d",i);
		RTMPSetKeyParameter(tok_str, value, sizeof(value), final,TRUE);
	}
	os_zero_mem(value,sizeof(value));
	for(i=0;i<data->total_num;i++){
		idx = (i < data->pf1_num) ? 0 : 1;
		snprintf(value,sizeof(value),"%s%d;",value,idx);
	}
	RTMPSetKeyParameter("EdcaIdx",value, sizeof(value),final,TRUE);
	return NDIS_STATUS_SUCCESS;
}

/*
* apedca
*/
static INT multi_profile_merge_apedca(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	CHAR tmpbuf[TEMP_STR_SIZE]="";
	CHAR tmpbuf2[TEMP_STR_SIZE]="";
	UCHAR edca_idx =0;
	CHAR *macptr;
	UCHAR i;
	UCHAR j;
	UCHAR edca_own[4] = { 0,0,0,0 };
	CHAR tok_str[25]="";

	/*for seach 2.4G band EdcaIdx*/
	if(RTMPGetKeyParameter("EdcaIdx", tmpbuf, TEMP_STR_SIZE, buf1, FALSE) != TRUE) {
		/*default EDCA parameters*/
		return multi_profile_merge_default_edca(data,buf1,buf2,final);
	}

	for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++){
		edca_idx = simple_strtol(macptr, 0, 10);
		if(edca_idx < 4){
			edca_own[edca_idx] = (0x10 | edca_idx);
		}
	}

	/*for seach 5G band EdcaIdx*/
	if(RTMPGetKeyParameter("EdcaIdx", tmpbuf2, TEMP_STR_SIZE, buf2, FALSE) != TRUE) {
		return NDIS_STATUS_SUCCESS;
	}

	for (i = 0, macptr = rstrtok(tmpbuf2,";"); macptr; macptr = rstrtok(NULL,";"), i++){
		edca_idx = simple_strtol(macptr, 0, 10);
		if(edca_idx >= 4){
			snprintf(tmpbuf,sizeof(tmpbuf),"%s;%d",tmpbuf,0);
			continue;
		}
		for(j=0; j < 4; j++){
			if(edca_own[j] == 0){
				break;
			}
		}
		if(j < 4)
			edca_own[j] = (0x20 | edca_idx);
		snprintf(tmpbuf,sizeof(tmpbuf),"%s;%d",tmpbuf,j);
	}

	RTMPSetKeyParameter("EdcaIdx", tmpbuf, TEMP_STR_SIZE,final,TRUE);
	/*merge  ApEdca%*/
	for(i = 0; i < 4; i++) {
		if (edca_own[i] & 0x10) {
			macptr = buf1;
		}else if (edca_own[i] & 0x20) {
			macptr = buf2;
		}else
			continue;

		j = (edca_own[i] & 0x3);
		snprintf(tok_str, sizeof(tok_str), "APEdca%d",j);
		if(RTMPGetKeyParameter(tok_str, tmpbuf, TEMP_STR_SIZE, macptr, TRUE)) {
			snprintf(tok_str, sizeof(tok_str), "APEdca%d",i);
			RTMPSetKeyParameter(tok_str, tmpbuf, TEMP_STR_SIZE, final,TRUE);
		}
	}
	return NDIS_STATUS_SUCCESS;
}

/*
* bssedca
*/
static INT multi_profile_merge_bssedca(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	UCHAR i;
	CHAR aifs[32]="";
	CHAR cwmin[32]="";
	CHAR cwmax[32]="";
	CHAR txop[32]="";
	CHAR acm[32]="";
	CHAR *buf = NULL;
	CHAR value[256]="";
	CHAR tok_str[25]="";

	for(i=0;i<data->total_num;i++){
		buf = (i < data->pf1_num) ? buf1 : buf2;
		os_zero_mem(value,sizeof(value));
		/*BSSAifsn*/
		if(RTMPGetKeyParameter("BSSAifsn", aifs, sizeof(aifs), buf, FALSE) != TRUE) {
			return NDIS_STATUS_FAILURE;
		}
		multi_profile_replace(";",",",aifs);
		/*BSSCwmin*/
		if(RTMPGetKeyParameter("BSSCwmin", cwmin, sizeof(cwmin), buf, FALSE) != TRUE) {
			return NDIS_STATUS_FAILURE;
		}
		multi_profile_replace(";",",",cwmin);
		/*BSSCwmax*/
		if(RTMPGetKeyParameter("BSSCwmax", cwmax, sizeof(cwmax), buf, FALSE) != TRUE) {
			return NDIS_STATUS_FAILURE;
		}
		multi_profile_replace(";",",",cwmax);
		/*BSSTxop*/
		if(RTMPGetKeyParameter("BSSTxop", txop, sizeof(txop), buf, FALSE) != TRUE) {
			return NDIS_STATUS_FAILURE;
		}
		multi_profile_replace(";",",",txop);
		/*BSSACM*/
		if(RTMPGetKeyParameter("BSSACM", acm, sizeof(acm), buf, FALSE) != TRUE) {
			return NDIS_STATUS_FAILURE;
		}
		multi_profile_replace(";",",",acm);
		/*merge*/
		snprintf(value,sizeof(value),"%s;%s;%s;%s;%s",aifs,cwmin,cwmax,txop,acm);
		/*set*/
		snprintf(tok_str,sizeof(tok_str),"BSSEdca%d",i);
		RTMPSetKeyParameter(tok_str, value, sizeof(value), final,TRUE);
	}
	return NDIS_STATUS_SUCCESS;
}

/*
* Channel
*/
static INT multi_profile_merge_channel(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	UCHAR ch1[32]="";
	UCHAR ch2[32]="";
	UCHAR value[256]="";
	UCHAR i;
	UCHAR *ch;
	if(RTMPGetKeyParameter("Channel", ch1, sizeof(ch1), buf1, TRUE) != TRUE) {
		return NDIS_STATUS_FAILURE;
	}
	if(RTMPGetKeyParameter("Channel", ch2, sizeof(ch2), buf2, TRUE) != TRUE) {
		return NDIS_STATUS_FAILURE;
	}
	os_zero_mem(value,sizeof(value));

	for(i=0;i<data->total_num;i++){
		ch = (i < data->pf1_num) ? ch1 : ch2;
		snprintf(value,sizeof(value),"%s%s;",value,ch);
	}
	RTMPSetKeyParameter("Channel",value, sizeof(value),final,TRUE);
	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
static INT multi_profile_merge_wireless_mode(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	UCHAR wmode1[32]="";
	UCHAR wmode2[32]="";
	UCHAR value[256]="";
	CHAR *macptr;
	UCHAR i;
	UCHAR *ch;

	if(RTMPGetKeyParameter("WirelessMode", wmode1, sizeof(wmode1), buf1, TRUE) != TRUE) {
		return NDIS_STATUS_FAILURE;
	}
	/*check number of wireless mode*/
	for (i = 0, macptr = rstrtok(wmode1,";"); macptr; macptr = rstrtok(NULL,";"), i++){
		/*do nothing*/
	}
	if(i > 1 ) {
		multi_profile_merge_separate("WirelessMode",buf1,buf2,final);
	}else{
		if(RTMPGetKeyParameter("WirelessMode", wmode2, sizeof(wmode2), buf2, TRUE) != TRUE) {
			return NDIS_STATUS_FAILURE;
		}
		os_zero_mem(value,sizeof(value));
		for(i=0;i<data->total_num;i++){
			ch = (i < data->pf1_num) ? wmode1 : wmode2;
			snprintf(value,sizeof(value),"%s%s;",value,ch);
		}
		RTMPSetKeyParameter("WirelessMode",value, sizeof(value),final,TRUE);
	}
	return NDIS_STATUS_SUCCESS;
}

/*
* VOW_BW_Ctrl
*/
static INT multi_profile_merge_vow_bw_ctrl(
	struct mpf_data *data,
	CHAR *parm,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	UCHAR group1[128]="";
	UCHAR group2[128]="";
	UCHAR value[128]="";
	UCHAR i;
	UCHAR *ptok = NULL;
	
	if(RTMPGetKeyParameter(parm, group1, sizeof(group1), buf1, TRUE) != TRUE) {
		return NDIS_STATUS_FAILURE;
	}
	if(RTMPGetKeyParameter(parm, group2, sizeof(group2), buf2, TRUE) != TRUE) {
		return NDIS_STATUS_FAILURE;
	}

	os_zero_mem(value,sizeof(value));
	for (i = 0, ptok = rstrtok(group1,";"); ptok; ptok = rstrtok(NULL,";"), i++)
    {
		if (i >= data->pf1_num)
			break;
		
		snprintf(value,sizeof(value),"%s%s;",value,ptok);
    }
	for (i = 0, ptok = rstrtok(group2,";"); ptok; ptok = rstrtok(NULL,";"), i++)
    {
		if (i >= data->pf2_num)
			break;
		
		snprintf(value,sizeof(value),"%s%s;",value,ptok);
    }

	RTMPSetKeyParameter(parm, value, sizeof(value),final,TRUE);
	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
static INT multi_profile_merge_ack_policy(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	UCHAR i;
	UCHAR idx;
	CHAR tmpbuf[32];
	CHAR tok_str[25]="";

	/*read 2.4G profile*/
	if(RTMPGetKeyParameter("AckPolicy", tmpbuf, 32, buf1, TRUE))
	{
		for (i = 0 ; i < data->pf1_num ; i++){
			snprintf(tok_str,sizeof(tok_str),"APAckPolicy%d",i);
			RTMPSetKeyParameter(tok_str,tmpbuf, sizeof(tmpbuf),final,TRUE);
		}
	}
	/*read 5G profile*/
	if(RTMPGetKeyParameter("AckPolicy", tmpbuf, 32, buf2, TRUE))
	{
		for (i = 0 ; i < data->pf2_num ; i++){
			idx = i + data->pf1_num;
			snprintf(tok_str,sizeof(tok_str),"APAckPolicy%d",idx);
			RTMPSetKeyParameter(tok_str,tmpbuf, sizeof(tmpbuf),final,TRUE);
		}
	}
	return NDIS_STATUS_SUCCESS;
}

/*
* mbss related merge function
*/
static INT multi_profile_merge_mbss(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*merge MACAddress*/
	multi_profile_merge_mac_address(data,buf1,buf2,final);
	/*merge SSID*/
	multi_profile_merge_increase(data,1,"SSID",buf1,buf2,final);
	/*merge DLSCapable*/
	multi_profile_merge_separate("DLSCapable",buf1,buf2,final);
	/*merge WirelessMode*/
	multi_profile_merge_wireless_mode(data,buf1,buf2,final);
	/*merge Channel*/
	multi_profile_merge_channel(data,buf1,buf2,final);
	/*merge AutoChannelSkipList*/
	multi_profile_merge_separate("AutoChannelSkipList",buf1,buf2,final);
	/*merge ACSCheckTime*/
	multi_profile_merge_separate("ACSCheckTime",buf1,buf2,final);	
	/*merge security*/
	multi_profile_merge_security(data,buf1,buf2,final);
	/*merge WmmCapable*/
	multi_profile_merge_separate("WmmCapable",buf1,buf2,final);
	/*merge NoForwarding*/
	multi_profile_merge_separate("NoForwarding",buf1,buf2,final);
	/*merge StationKeepAlive*/
	multi_profile_merge_perband(data,"StationKeepAlive",buf1,buf2,final);
	/*merge HideSSID*/
	multi_profile_merge_separate("HideSSID",buf1,buf2,final);
	/*merge HT_EXTCHA*/
	multi_profile_merge_perband(data,"HT_EXTCHA",buf1,buf2,final);
	/*merge HT_TxStream*/
	multi_profile_merge_perband(data,"HT_TxStream",buf1,buf2,final);
	/*merge HT_RxStream*/
	multi_profile_merge_perband(data,"HT_RxStream",buf1,buf2,final);
	/*merge HT_MCS*/
	multi_profile_merge_separate("HT_MCS",buf1,buf2,final);
	/*merge HT_BW*/
	multi_profile_merge_perband(data,"HT_BW",buf1,buf2,final);
	/*merge HT_STBC*/
	multi_profile_merge_perband(data,"HT_STBC",buf1,buf2,final);
	/*merge HT_LDPC*/
	multi_profile_merge_perband(data,"HT_LDPC",buf1,buf2,final);
	/*merge VHT_STBC*/
	multi_profile_merge_separate("VHT_STBC",buf1,buf2,final);
	/*merge VHT_LDPC*/
	multi_profile_merge_separate("VHT_LDPC",buf1,buf2,final);
	/*merge MbssMaxStaNum*/
	multi_profile_merge_separate("MbssMaxStaNum",buf1,buf2,final);
	/*merge APSDCapable*/
	multi_profile_merge_separate("APSDCapable",buf1,buf2,final);
	/*merge DscpQosMapping*/
#ifdef DSCP_QOS_MAP_SUPPORT
	multi_profile_merge_separate("DscpQosMapEnable",buf1,buf2,final);
	multi_profile_merge_separate("DscpQosMap",buf1,buf2,final);
#endif
	/*merge APEdcaIdx*/
	multi_profile_merge_apedca(data,buf1,buf2,final);
	/*merge BSSEdcaIdx*/
	multi_profile_merge_bssedca(data,buf1,buf2,final);
	/*merge AckPolicy*/
	multi_profile_merge_ack_policy(data,buf1,buf2,final);
	{
	    INT Status = FALSE;
	    CHAR value[TEMP_STR_SIZE]="";
            CHAR tmpbuf[TEMP_STR_SIZE]="";

	    /*merge CountryRegionABand*/
	    Status = RTMPGetKeyParameter("CountryRegionABand", tmpbuf, TEMP_STR_SIZE, buf2, TRUE);
	    if (Status == TRUE) {
	        snprintf(value,sizeof(value),"%s",tmpbuf);
	        RTMPSetKeyParameter("CountryRegionABand", value, TEMP_STR_SIZE,final, TRUE);
	    }
	}
	/*merge VOW BW_Ctrl related profile*/
	multi_profile_merge_vow_bw_ctrl(data,"VOW_Group_Min_Rate",buf1,buf2,final);
	multi_profile_merge_vow_bw_ctrl(data,"VOW_Group_Max_Rate",buf1,buf2,final);
	multi_profile_merge_vow_bw_ctrl(data,"VOW_Group_Min_Ratio",buf1,buf2,final);
	multi_profile_merge_vow_bw_ctrl(data,"VOW_Group_Max_Ratio",buf1,buf2,final);
	multi_profile_merge_vow_bw_ctrl(data,"VOW_Airtime_Ctrl_En",buf1,buf2,final);
	multi_profile_merge_vow_bw_ctrl(data,"VOW_Rate_Ctrl_En",buf1,buf2,final);
	multi_profile_merge_vow_bw_ctrl(data,"VOW_Group_Min_Rate_Bucket_Size",buf1,buf2,final);
	multi_profile_merge_vow_bw_ctrl(data,"VOW_Group_Max_Rate_Bucket_Size",buf1,buf2,final);
	multi_profile_merge_vow_bw_ctrl(data,"VOW_Group_Min_Airtime_Bucket_Size",buf1,buf2,final);
	multi_profile_merge_vow_bw_ctrl(data,"VOW_Group_Max_Airtime_Bucket_Size",buf1,buf2,final);
	multi_profile_merge_vow_bw_ctrl(data,"VOW_Group_Backlog",buf1,buf2,final);
	multi_profile_merge_vow_bw_ctrl(data,"VOW_Group_Max_Wait_Time",buf1,buf2,final);
	multi_profile_merge_vow_bw_ctrl(data,"VOW_Group_DWRR_Quantum",buf1,buf2,final);

	multi_profile_merge_wsc(data,buf1,buf2,final);
	return NDIS_STATUS_SUCCESS;
}

/*
* wsc related merge function
*/
static INT multi_profile_merge_wsc(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
#if defined(WSC_V2_SUPPORT) || defined(WSC_AP_SUPPORT)
	UCHAR WscConMode[32]="";
	UCHAR WscConMode2[32]="";
	UCHAR value[256]="";
	INT	i;
	CHAR *macptr;
#endif

#ifdef WSC_V2_SUPPORT
	/*merge WscV2Support*/
	{
		/*merge WscConfMode*/
		if (RTMPGetKeyParameter("WscV2Support", WscConMode, sizeof(WscConMode), buf1, TRUE) != TRUE) 
		{
			goto label_wsc_v2_done;
		}
		for (i = 0, macptr = rstrtok(WscConMode,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			/*do nothing*/
		}
		RTMPGetKeyParameter("WscV2Support", WscConMode, sizeof(WscConMode), buf1, TRUE) ;
		if (data->pf1_num > i)
		{//need to append default value
			INT append_cnt = data->pf1_num - i;
			INT loop_cnt = 0;
			while (append_cnt) 
			{
				snprintf(WscConMode,sizeof(WscConMode),"%s; ",WscConMode);
				append_cnt--;
				loop_cnt++;
			}
		}
		else if (data->pf1_num < i)
		{
			goto label_wsc_v2_done;
		}

		if (RTMPGetKeyParameter("WscV2Support", WscConMode2, sizeof(WscConMode2), buf2, TRUE) != TRUE) 
		{
			goto label_wsc_v2_done;
		}
		for (i = 0, macptr = rstrtok(WscConMode2,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			/*do nothing*/
		}
		RTMPGetKeyParameter("WscV2Support", WscConMode2, sizeof(WscConMode2), buf2, TRUE) ;
		if (data->pf2_num > i)
		{//need to append default value
			INT append_cnt = data->pf2_num - i;
			INT loop_cnt = 0;
			while (append_cnt) 
			{
				snprintf(WscConMode2,sizeof(WscConMode2),"%s; ",WscConMode2);
				append_cnt--;
				loop_cnt++;
			}
		} 
		else if (data->pf2_num < i)
		{
			goto label_wsc_v2_done;
		}
		snprintf(value,sizeof(value),"%s;%s",WscConMode,WscConMode2);
		RTMPSetKeyParameter("WscV2Support",value, sizeof(value),final,TRUE);
	}
label_wsc_v2_done:
#endif /*WSC_V2_SUPPORT*/

#ifdef WSC_AP_SUPPORT
	{
		/*merge WscConfMode*/
		if (RTMPGetKeyParameter("WscConfMode", WscConMode, sizeof(WscConMode), buf1, TRUE) != TRUE) 
		{
			goto label_WscConfMode_done;
		}
		for (i = 0, macptr = rstrtok(WscConMode,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			/*do nothing*/
		}
		RTMPGetKeyParameter("WscConfMode", WscConMode, sizeof(WscConMode), buf1, TRUE) ;
		if (data->pf1_num > i)
		{//need to append default value
			INT append_cnt = data->pf1_num - i;
			INT loop_cnt = 0;
			while (append_cnt) 
			{
				snprintf(WscConMode,sizeof(WscConMode),"%s; ",WscConMode);
				append_cnt--;
				loop_cnt++;
			}
		} 
		else if (data->pf1_num < i)
		{
			goto label_WscConfMode_done;
		}
		if (RTMPGetKeyParameter("WscConfMode", WscConMode2, sizeof(WscConMode2), buf2, TRUE) != TRUE) 
		{
			goto label_WscConfMode_done;
		}
		for (i = 0, macptr = rstrtok(WscConMode2,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			/*do nothing*/
		}
		RTMPGetKeyParameter("WscConfMode", WscConMode2, sizeof(WscConMode2), buf2, TRUE) ;
		if (data->pf2_num > i)
		{//need to append default value
			INT append_cnt = data->pf2_num - i;
			INT loop_cnt = 0;
			while (append_cnt) 
			{
				snprintf(WscConMode2,sizeof(WscConMode2),"%s; ",WscConMode2);
				append_cnt--;
				loop_cnt++;
			}
		}
		else if (data->pf2_num < i)
		{
			goto label_WscConfMode_done;
		}
		snprintf(value,sizeof(value),"%s;%s",WscConMode,WscConMode2);
		RTMPSetKeyParameter("WscConfMode",value, sizeof(value),final,TRUE);
label_WscConfMode_done:

		/*merge WscConfStatus*/
		if (RTMPGetKeyParameter("WscConfStatus", WscConMode, sizeof(WscConMode), buf1, TRUE) != TRUE) 
		{
			goto label_WscConfStatus_done;
		}
		for (i = 0, macptr = rstrtok(WscConMode,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			/*do nothing*/
		}
		RTMPGetKeyParameter("WscConfStatus", WscConMode, sizeof(WscConMode), buf1, TRUE) ;
		if (data->pf1_num > i)
		{//need to append default value
			INT append_cnt = data->pf1_num - i;
			INT loop_cnt = 0;
			while (append_cnt) 
			{
				snprintf(WscConMode,sizeof(WscConMode),"%s; ",WscConMode);
				append_cnt--;
				loop_cnt++;
			}
		}
		else if (data->pf1_num < i)
		{
			goto label_WscConfStatus_done;
		}
	
		if (RTMPGetKeyParameter("WscConfStatus", WscConMode2, sizeof(WscConMode2), buf2, TRUE) != TRUE) 
		{
			goto label_WscConfStatus_done;
		}
		for (i = 0, macptr = rstrtok(WscConMode2,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			/*do nothing*/
		}
		RTMPGetKeyParameter("WscConfStatus", WscConMode2, sizeof(WscConMode2), buf2, TRUE) ;
		if (data->pf2_num > i)
		{//need to append default value
			INT append_cnt = data->pf2_num - i;
			INT loop_cnt = 0;
			while (append_cnt) 
			{
				snprintf(WscConMode2,sizeof(WscConMode2),"%s; ",WscConMode2);
				append_cnt--;
				loop_cnt++;
			}
		}
		else if (data->pf2_num < i)
		{
			goto label_WscConfStatus_done;
		}
		snprintf(value,sizeof(value),"%s;%s",WscConMode,WscConMode2);
		RTMPSetKeyParameter("WscConfStatus",value, sizeof(value),final,TRUE);
	}
label_WscConfStatus_done:
#endif /*WSC_AP_SUPPORT*/

	return NDIS_STATUS_SUCCESS;
}
#endif /*MBSS_SUPPORT*/


#ifdef APCLI_SUPPORT
/*
* apcli related merge function
*/
static INT multi_profile_merge_apcli(
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	INT status = FALSE;
	CHAR tmpbuf[TEMP_STR_SIZE]= "";
	CHAR value[TEMP_STR_SIZE]="";
#ifdef MAC_REPEATER_SUPPORT
	/*MACRepeaterEn, use profile 1*/
	status = RTMPGetKeyParameter("MACRepeaterEn", tmpbuf, TEMP_STR_SIZE, buf1, TRUE);
	if (status == TRUE) {
		snprintf(value,sizeof(value),"%s",tmpbuf);
		RTMPSetKeyParameter("MACRepeaterEn", value, TEMP_STR_SIZE,final, TRUE);
	}
	/*MACRepeaterOuiMode, use profile 1*/
	status = RTMPGetKeyParameter("MACRepeaterOuiMode", tmpbuf, TEMP_STR_SIZE, buf1, TRUE);
	if (status == TRUE) {
		snprintf(value,sizeof(value),"%s",tmpbuf);
		RTMPSetKeyParameter("MACRepeaterOuiMode", value, TEMP_STR_SIZE,final, TRUE);
	}
#endif

	/*merge ApCliEnable*/
	multi_profile_merge_separate("ApCliEnable",buf1,buf2,final);
	/*merge ApCliSsid*/
	multi_profile_merge_separate("ApCliSsid",buf1,buf2,final);
	/*merge ApCliWirelessMode*/
	multi_profile_merge_separate("ApCliWirelessMode",buf1,buf2,final);
	/*merge ApCliBssid*/
	multi_profile_merge_separate("ApCliBssid",buf1,buf2,final);
	/*merge ApCliAuthMode*/
	multi_profile_merge_separate("ApCliAuthMode",buf1,buf2,final);
	/*merge ApCliEncrypType*/
	multi_profile_merge_separate("ApCliEncrypType",buf1,buf2,final);
	{
		/*merge apcli0 ApCliWPAPSK*/
		status = RTMPGetKeyParameter("ApCliWPAPSK", tmpbuf, TEMP_STR_SIZE, buf1, TRUE);
		if (status == TRUE) {
			snprintf(value,sizeof(value),"%s",tmpbuf);
			RTMPSetKeyParameter("ApCliWPAPSK", value, TEMP_STR_SIZE,final, TRUE);
		}
		/*tansfer apcli1 ApCliWPAPSK to ApCliWPAPSK1*/
		status = RTMPGetKeyParameter("ApCliWPAPSK", tmpbuf, TEMP_STR_SIZE, buf2, TRUE);
		if (status == TRUE) {
			snprintf(value,sizeof(value),"%s",tmpbuf);
			RTMPSetKeyParameter("ApCliWPAPSK1", value, TEMP_STR_SIZE, final, TRUE);
		}
	}
	/*merge ApCliDefaultKeyID*/
	multi_profile_merge_separate("ApCliDefaultKeyID",buf1,buf2,final);
	/*merge ApCliKey1Type*/
	multi_profile_merge_separate("ApCliKey1Type",buf1,buf2,final);
	/*merge ApCliKey2Type*/
	multi_profile_merge_separate("ApCliKey2Type",buf1,buf2,final);
	/*merge ApCliKey3Type*/
	multi_profile_merge_separate("ApCliKey3Type",buf1,buf2,final);
	/*merge ApCliKey4Type*/
	multi_profile_merge_separate("ApCliKey4Type",buf1,buf2,final);
	{
		/*merge apcli0 ApCliKey1Str*/
		status = RTMPGetKeyParameter("ApCliKey1Str", tmpbuf, TEMP_STR_SIZE, buf1, TRUE);
		if (status == TRUE) {
			snprintf(value,sizeof(value),"%s",tmpbuf);
			RTMPSetKeyParameter("ApCliKey1Str", value, TEMP_STR_SIZE,final, TRUE);
		}
		/*tansfer apcli1 ApCliKey1Str to ApCliKey1Str1*/
		status = RTMPGetKeyParameter("ApCliKey1Str", tmpbuf, TEMP_STR_SIZE, buf2, TRUE);
		if (status == TRUE) {
			snprintf(value,sizeof(value),"%s",tmpbuf);
			RTMPSetKeyParameter("ApCliKey1Str1", value, TEMP_STR_SIZE, final, TRUE);
		}
		/*merge apcli0 ApCliKey2Str*/
		status = RTMPGetKeyParameter("ApCliKey2Str", tmpbuf, TEMP_STR_SIZE, buf1, TRUE);
		if (status == TRUE) {
			snprintf(value,sizeof(value),"%s",tmpbuf);
			RTMPSetKeyParameter("ApCliKey2Str", value, TEMP_STR_SIZE,final, TRUE);
		}
		/*tansfer apcli1 ApCliKey2Str to ApCliKey2Str1*/
		status = RTMPGetKeyParameter("ApCliKey2Str", tmpbuf, TEMP_STR_SIZE, buf2, TRUE);
		if (status == TRUE) {
			snprintf(value,sizeof(value),"%s",tmpbuf);
			RTMPSetKeyParameter("ApCliKey2Str1", value, TEMP_STR_SIZE, final, TRUE);
		}
		/*merge apcli0 ApCliKey3Str*/
		status = RTMPGetKeyParameter("ApCliKey3Str", tmpbuf, TEMP_STR_SIZE, buf1, TRUE);
		if (status == TRUE) {
			snprintf(value,sizeof(value),"%s",tmpbuf);
			RTMPSetKeyParameter("ApCliKey3Str", value, TEMP_STR_SIZE,final, TRUE);
		}
		/*tansfer apcli1 ApCliKey3Str to ApCliKey3Str1*/
		status = RTMPGetKeyParameter("ApCliKey3Str", tmpbuf, TEMP_STR_SIZE, buf2, TRUE);
		if (status == TRUE) {
			snprintf(value,sizeof(value),"%s",tmpbuf);
			RTMPSetKeyParameter("ApCliKey3Str1", value, TEMP_STR_SIZE, final, TRUE);
		}
		/*merge apcli0 ApCliKey4Str*/
		status = RTMPGetKeyParameter("ApCliKey4Str", tmpbuf, TEMP_STR_SIZE, buf1, TRUE);
		if (status == TRUE) {
			snprintf(value,sizeof(value),"%s",tmpbuf);
			RTMPSetKeyParameter("ApCliKey4Str", value, TEMP_STR_SIZE,final, TRUE);
		}
		/*tansfer apcli1 ApCliKey4Str to ApCliKey4Str1*/
		status = RTMPGetKeyParameter("ApCliKey4Str", tmpbuf, TEMP_STR_SIZE, buf2, TRUE);
		if (status == TRUE) {
			snprintf(value,sizeof(value),"%s",tmpbuf);
			RTMPSetKeyParameter("ApCliKey4Str1", value, TEMP_STR_SIZE, final,TRUE);
		}
	}

	/*merge ApCliTxMode*/
	multi_profile_merge_separate("ApCliTxMode",buf1,buf2,final);
	/*merge ApCliTxMcs*/
	multi_profile_merge_separate("ApCliTxMcs",buf1,buf2,final);
	
#ifdef WSC_AP_SUPPORT
	/*merge ApCli_Wsc4digitPinCode*/
	multi_profile_merge_separate("ApCli_Wsc4digitPinCode",buf1,buf2,final);
    /*merge ApCliWscScanMode*/
    multi_profile_merge_separate("ApCliWscScanMode",buf1,buf2,final);
#endif /*WSC_AP_SUPPORT*/
#ifdef UAPSD_SUPPORT
	/*merge ApCliAPSDCapable*/
	multi_profile_merge_separate("ApCliAPSDCapable",buf1,buf2,final);
#endif /*UAPSD_SUPPORT*/

	/*merge ApCliPMFMFPC*/
	multi_profile_merge_separate("ApCliPMFMFPC",buf1,buf2,final);
	/*merge ApCliPMFMFPR*/
	multi_profile_merge_separate("ApCliPMFMFPR",buf1,buf2,final);
	/*merge ApCliPMFSHA256*/
	multi_profile_merge_separate("ApCliPMFSHA256",buf1,buf2,final);
	return NDIS_STATUS_SUCCESS;
}
#endif /*APCLI_SUPPORT*/

#ifdef BAND_STEERING
static INT multi_profile_merge_bandsteering(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	//multi_profile_merge_separate("BndStrgBssIdx",buf1,buf2,final);
	//return NDIS_STATUS_SUCCESS;

	CHAR tmpbuf[TEMP_STR_SIZE]= "";
	CHAR tmpbuf2[TEMP_STR_SIZE]= "";
	CHAR value[TEMP_STR_SIZE]="";
	RTMP_STRING *macptr = NULL;
	int i=0;

	if (!buf1 || !buf2) 
		return NDIS_STATUS_FAILURE;

	if(RTMPGetKeyParameter("BndStrgBssIdx", tmpbuf, TEMP_STR_SIZE, buf1, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			if(i == data->pf1_num)
				break;
			
			if(i == 0)
				snprintf((value + strlen(value)),sizeof(value),"%s",macptr);
			else
				snprintf((value + strlen(value)),sizeof(value),";%s",macptr);
		}
		if(i < data->pf1_num)
		{
			for(; i < data->pf1_num; i++){
				if(i == 0)
					snprintf((value + strlen(value)),sizeof(value),"%s","1");
				else
					snprintf((value + strlen(value)),sizeof(value),";%s","0");
			}
		}	
	}else{
		for(i = 0; i < data->pf1_num; i++)
		{
			if(i==0)
				snprintf((value + strlen(value)),sizeof(value),"%s","1");
			else
				snprintf((value + strlen(value)),sizeof(value),";%s","0");
		}
	}
	
	if(RTMPGetKeyParameter("BndStrgBssIdx", tmpbuf2, TEMP_STR_SIZE, buf2, TRUE)){
		for (i = 0, macptr = rstrtok(tmpbuf2,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			if(i == data->pf2_num)
				break;
			snprintf((value + strlen(value)),sizeof(value),";%s",macptr);
		}
		if(i < data->pf2_num)
		{
			for(; i < data->pf2_num; i++){
				if(i == 0)
					snprintf((value + strlen(value)),sizeof(value),";%s","1");
				else
					snprintf((value + strlen(value)),sizeof(value),";%s","0");
			}
		}
	}else{
		for(i = 0; i < data->pf2_num; i++)
		{
			if(i==0)
				snprintf((value + strlen(value)),sizeof(value),";%s","1");
			else
				snprintf((value + strlen(value)),sizeof(value),";%s","0");
		}
	}
	RTMPSetKeyParameter("BndStrgBssIdx",value, TEMP_STR_SIZE,final, TRUE);	 
	return NDIS_STATUS_SUCCESS;
}
#endif

/*
* BssidNum
*/
static INT multi_profile_merge_bssidnum(struct mpf_data *data,CHAR *buf1, CHAR *buf2, CHAR *final)
{
	CHAR tmpbuf[25]="";
	UCHAR num1=0;
	UCHAR num2=0;
	UCHAR total;

	if(RTMPGetKeyParameter("BssidNum", tmpbuf, 25, buf1, TRUE)){
		num1 = (UCHAR) simple_strtol(tmpbuf, 0, 10);
	}
	if(RTMPGetKeyParameter("BssidNum", tmpbuf, 25, buf2, TRUE)){
		num2 = (UCHAR) simple_strtol(tmpbuf, 0, 10);
	}
	total = num1 + num2;
	snprintf(tmpbuf,sizeof(tmpbuf),"%d",total);
	RTMPSetKeyParameter("BssidNum",tmpbuf, 25,final, TRUE);
	/*assign bss number*/
	data->pf1_num = num1;
	data->pf2_num = num2;
	data->total_num = total;
	return NDIS_STATUS_SUCCESS;
}
#endif /*CONFIG_AP_SUPPORT*/

/*
 * protections: including HT_PROTECT / RTS_THRESHOLD
 */
static INT multi_profile_merge_protection(
		CHAR *buf1, CHAR *buf2, CHAR *final)
{
	/*RTSPktThreshold*/
	multi_profile_merge_separate("RTSPktThreshold", buf1, buf2, final);
	/*RTSThreshold*/
	multi_profile_merge_separate("RTSThreshold", buf1, buf2, final);
	/*HT_PRORTECT*/
	multi_profile_merge_separate("HT_PROTECT", buf1, buf2, final);

	return NDIS_STATUS_SUCCESS;
}

static INT multi_profile_merge_frag(
		CHAR *buf1, CHAR *buf2, CHAR *final)
{
	/*Fragment Threshold*/
	multi_profile_merge_separate("FragThreshold", buf1, buf2, final);

	return NDIS_STATUS_SUCCESS;
}

/*
* merge 5G only related
*/
static INT multi_profile_merge_5g_only(CHAR *buf1, CHAR *buf2, CHAR *final)
{
	CHAR tmpbuf[64]="";
	UCHAR len = sizeof(tmpbuf);

	/*merge VHT_BW*/
	if(RTMPGetKeyParameter("VHT_BW", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("VHT_BW",tmpbuf, len,final, TRUE);
	}
	/*merge VHT_SGI*/
	if(RTMPGetKeyParameter("VHT_SGI", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("VHT_SGI",tmpbuf, len,final, TRUE);
	}
	/*merge VHT_BW_SIGNAL*/
	if(RTMPGetKeyParameter("VHT_BW_SIGNAL", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("VHT_BW_SIGNAL",tmpbuf, len,final, TRUE);
	}
	/*merge VHT_Sec80_Channel*/
	if(RTMPGetKeyParameter("VHT_Sec80_Channel", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("VHT_Sec80_Channel",tmpbuf, len,final, TRUE);
	}
	/*MUTxRxEnable*/
	if(RTMPGetKeyParameter("MUTxRxEnable", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("MUTxRxEnable",tmpbuf, len,final, TRUE);
	}
	/*IEEE80211H*/
	if(RTMPGetKeyParameter("IEEE80211H", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("IEEE80211H",tmpbuf, len,final, TRUE);
	}
    /*DFS related params is 5G only, use profile 2*/
#ifdef MT_DFS_SUPPORT    
    /*DfsEnable*/
    if (RTMPGetKeyParameter("DfsEnable", tmpbuf, len, buf2, TRUE) == TRUE) {
		RTMPSetKeyParameter("DfsEnable", tmpbuf, len,final, TRUE);
	}
#endif
	/*CSPeriod*/
	if (RTMPGetKeyParameter("CSPeriod", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("CSPeriod", tmpbuf, len,final, TRUE);
	}
    /*RDRegion*/
    if (RTMPGetKeyParameter("RDRegion", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("RDRegion", tmpbuf, len,final, TRUE);
	}
	return NDIS_STATUS_SUCCESS;
}

#ifdef DEFAULT_5G_PROFILE
/*
* merge 2G only related
*/
static INT multi_profile_merge_2g_only(CHAR *buf1, CHAR *buf2, CHAR *final)
{
	CHAR tmpbuf[64]="";
	UCHAR len = sizeof(tmpbuf);

	/*merge CountryRegion*/
	if(RTMPGetKeyParameter("CountryRegion", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("CountryRegion",tmpbuf, len,final, TRUE);
	}
	/*merge DisableOLBC*/
	if(RTMPGetKeyParameter("DisableOLBC", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("DisableOLBC",tmpbuf, len,final, TRUE);
	}
	/*merge G_BAND_256QAM*/
	if(RTMPGetKeyParameter("G_BAND_256QAM", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("G_BAND_256QAM",tmpbuf, len,final, TRUE);
	}
	/*merge HT_BSSCoexistence*/
	if(RTMPGetKeyParameter("HT_BSSCoexistence", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("HT_BSSCoexistence",tmpbuf, len,final, TRUE);
	}
	/*BGProtection*/
	if(RTMPGetKeyParameter("BGProtection", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("BGProtection",tmpbuf, len,final, TRUE);
	}
	/*TxPreamble*/
	if(RTMPGetKeyParameter("TxPreamble", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("TxPreamble",tmpbuf, len,final, TRUE);
	}
	
	return NDIS_STATUS_SUCCESS;
}
/*
* merge global setting only related
*/
static INT multi_profile_merge_global_setting_only(CHAR *buf1, CHAR *buf2, CHAR *final)
{
	CHAR tmpbuf[64]="";
	UCHAR len = sizeof(tmpbuf);

	/*merge CountryCode*/
	if(RTMPGetKeyParameter("CountryCode", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("CountryCode",tmpbuf, len,final, TRUE);
	}
	/*merge NoForwardingBTNBSSID*/
	if(RTMPGetKeyParameter("NoForwardingBTNBSSID", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("NoForwardingBTNBSSID",tmpbuf, len,final, TRUE);
	}
	/*merge GreenAP*/
	if(RTMPGetKeyParameter("GreenAP", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("GreenAP",tmpbuf, len,final, TRUE);
	}
	/*DBDC_MODE*/
	if(RTMPGetKeyParameter("DBDC_MODE", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("DBDC_MODE","1", 1,final, TRUE);
	}
	if(RTMPGetKeyParameter("DBDC_MODE", tmpbuf, len, buf1, TRUE) == TRUE){
		RTMPSetKeyParameter("DBDC_MODE","1", 1,final, TRUE);
	}
	/*IcapMode*/
	if(RTMPGetKeyParameter("IcapMode", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("IcapMode",tmpbuf, len,final, TRUE);
	}
	/*CarrierDetect*/
	if(RTMPGetKeyParameter("CarrierDetect", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("CarrierDetect",tmpbuf, len,final, TRUE);
	}
	/*DebugFlags*/
	if(RTMPGetKeyParameter("DebugFlags", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("DebugFlags",tmpbuf, len,final, TRUE);
	}
	/*E2pAccessMode*/
	if(RTMPGetKeyParameter("E2pAccessMode", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("E2pAccessMode",tmpbuf, len,final, TRUE);
	}
	/*EfuseBufferMode*/
	if(RTMPGetKeyParameter("EfuseBufferMode", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("EfuseBufferMode",tmpbuf, len,final, TRUE);
	}
	/*WCNTest*/
	if(RTMPGetKeyParameter("WCNTest", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("WCNTest",tmpbuf, len,final, TRUE);
	}
	/*AutoChannelSelect*/
	if(RTMPGetKeyParameter("AutoChannelSelect", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("AutoChannelSelect",tmpbuf, len,final, TRUE);
	}
	/*HT_RDG*/
	if(RTMPGetKeyParameter("HT_RDG", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("HT_RDG",tmpbuf, len,final, TRUE);
	}
	/*HT_OpMode*/
	if(RTMPGetKeyParameter("HT_OpMode", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("HT_OpMode",tmpbuf, len,final, TRUE);
	}
	/*HT_MpduDensity*/
	if(RTMPGetKeyParameter("HT_MpduDensity", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("HT_MpduDensity",tmpbuf, len,final, TRUE);
	}
	/*HT_BADecline*/
	if(RTMPGetKeyParameter("HT_BADecline", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("HT_BADecline",tmpbuf, len,final, TRUE);
	}
	/*HT_AutoBA*/
	if(RTMPGetKeyParameter("HT_AutoBA", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("HT_AutoBA",tmpbuf, len,final, TRUE);
	}
	/*HT_BAWinSize*/
	if(RTMPGetKeyParameter("HT_BAWinSize", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("HT_BAWinSize",tmpbuf, len,final, TRUE);
	}
	/*HT_AMSDU*/
	if(RTMPGetKeyParameter("HT_AMSDU", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("HT_AMSDU",tmpbuf, len,final, TRUE);
	}
	/*BeaconPeriod*/
	if(RTMPGetKeyParameter("BeaconPeriod", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("BeaconPeriod",tmpbuf, len,final, TRUE);
	}
	/*DtimPeriod*/
	if(RTMPGetKeyParameter("DtimPeriod", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("DtimPeriod",tmpbuf, len,final, TRUE);
	}
	/*TxBurst*/
	if(RTMPGetKeyParameter("TxBurst", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("TxBurst",tmpbuf, len,final, TRUE);
	}
	/*PktAggregate*/
	if(RTMPGetKeyParameter("PktAggregate", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("PktAggregate",tmpbuf, len,final, TRUE);
	}
	/*VOW_WATF_Enable*/
	if(RTMPGetKeyParameter("VOW_WATF_Enable", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("VOW_WATF_Enable",tmpbuf, len,final, TRUE);
	}
	/*VOW_WATF_MAC_LV1*/
	if(RTMPGetKeyParameter("VOW_WATF_MAC_LV1", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("VOW_WATF_MAC_LV1",tmpbuf, len,final, TRUE);
	}
	/*VOW_WATF_MAC_LV2*/
	if(RTMPGetKeyParameter("VOW_WATF_MAC_LV2", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("VOW_WATF_MAC_LV2",tmpbuf, len,final, TRUE);
	}
	/*VOW_WATF_MAC_LV3*/
	if(RTMPGetKeyParameter("VOW_WATF_MAC_LV3", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("VOW_WATF_MAC_LV3",tmpbuf, len,final, TRUE);
	}
	/*VOW_WATF_MAC_LV4*/
	if(RTMPGetKeyParameter("VOW_WATF_MAC_LV4", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("VOW_WATF_MAC_LV4",tmpbuf, len,final, TRUE);
	}
	/*VOW_Airtime_Fairness_En*/
	if(RTMPGetKeyParameter("VOW_Airtime_Fairness_En", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("VOW_Airtime_Fairness_En",tmpbuf, len,final, TRUE);
	}
	/*VOW_BW_Ctrl*/
	if(RTMPGetKeyParameter("VOW_BW_Ctrl", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("VOW_BW_Ctrl",tmpbuf, len,final, TRUE);
	}
    /* VOW_RX_En */
	if(RTMPGetKeyParameter("VOW_RX_En", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("VOW_RX_En",tmpbuf, len,final, TRUE);
	}
	/*RED_Enable*/
	if(RTMPGetKeyParameter("RED_Enable", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("RED_Enable",tmpbuf, len,final, TRUE);
	}

	return NDIS_STATUS_SUCCESS;
}
#endif

#ifdef IGMP_SNOOP_SUPPORT
/*
* merge igmp related
*/
static INT multi_profile_merge_igmp(CHAR *buf1, CHAR *buf2, CHAR *final)
{
	/*merge VHT_BW*/
	multi_profile_merge_separate("IgmpSnEnable",buf1,buf2,final);
	return NDIS_STATUS_SUCCESS;
}
#endif /* IGMP_SNOOP_SUPPORT */

static INT multi_profile_merge_edcca(CHAR *buf1, CHAR *buf2, CHAR *final)
{
	/*merge EDCCA*/
	multi_profile_merge_separate("EDCCAEnable",buf1,buf2,final);
	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
static INT multi_profile_merge_dbdc_mode(CHAR *buf1, CHAR *buf2, CHAR *final)
{
#ifdef DEFAULT_5G_PROFILE
	UCHAR tmpbuf[25]="";
    UCHAR dbdc_mode=0;
    /*check dbdc mode is enable*/
    if(RTMPGetKeyParameter("DBDC_MODE", tmpbuf, 25, buf1, TRUE)){
        dbdc_mode = (UCHAR) simple_strtol(tmpbuf, 0, 10);
        if(!dbdc_mode){         
            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("DBDC_MODE is not enable! Not need to merge.\n"));
            goto buf2_check;
        }
        return NDIS_STATUS_SUCCESS;
	
    }
buf2_check: 
    if(RTMPGetKeyParameter("DBDC_MODE", tmpbuf, 25, buf2, TRUE)){
        dbdc_mode = (UCHAR) simple_strtol(tmpbuf, 0, 10);
        if(!dbdc_mode){
            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("DBDC_MODE is not enable! Not need to merge.\n"));
            return NDIS_STATUS_FAILURE;
        }
    }
    return NDIS_STATUS_SUCCESS;
#else
	UCHAR tmpbuf[25]="";
	UCHAR dbdc_mode=0;
	/*check dbdc mode is enable*/
	if(RTMPGetKeyParameter("DBDC_MODE", tmpbuf, 25, buf1, TRUE)){
		dbdc_mode = (UCHAR) simple_strtol(tmpbuf, 0, 10);
		if(!dbdc_mode){			
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("DBDC_MODE is not enable! Not need to merge.\n"));
			return NDIS_STATUS_FAILURE;
		}
	}
	return NDIS_STATUS_SUCCESS;
#endif
}


#ifdef TXBF_SUPPORT
/*
*  TXBF merge function for multiple profile mode 
*/
static INT multi_profile_merge_txbf(
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
    CHAR tmpbuf[64]="";
	UCHAR len = sizeof(tmpbuf);

	/*merge ETxBfEnCond*/
	if(RTMPGetKeyParameter("ETxBfEnCond", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("ETxBfEnCond",tmpbuf, len,final, TRUE);
	}
    /*merge ITxBfEn*/
    if(RTMPGetKeyParameter("ITxBfEn", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("ITxBfEn",tmpbuf, len,final, TRUE);
	}

    return NDIS_STATUS_SUCCESS;
}

#ifdef TXBF_BY_CHANNEL
/*
*  TXBF by channel merge function for multiple profile mode 
*/
static INT multi_profile_merge_bandnobf(
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
    CHAR tmpbuf[64]="";
	UCHAR len = sizeof(tmpbuf);

	/*merge BandNoBf*/
	if (RTMPGetKeyParameter("BandNoBf", tmpbuf, len, buf2, TRUE) == TRUE){
		RTMPSetKeyParameter("BandNoBf",tmpbuf, len,final, TRUE);
	}

    return NDIS_STATUS_SUCCESS;
}
#endif /* TXBF_BY_CHANNEL */

#endif /*TXBF_SUPPORT*/

/*
*  TXPOWER merge function for multiple profile mode 
*/
static INT multi_profile_merge_txpwr(
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{

#ifdef SINGLE_SKU_V2
	/*merge SKUenable*/
    multi_profile_merge_separate("SKUenable",buf1,buf2,final);
#endif /* SINGLE_SKU_V2 */

    /*merge PERCENTAGEenable*/
    multi_profile_merge_separate("PERCENTAGEenable",buf1,buf2,final);

    /*merge BFBACKOFFenable*/
    multi_profile_merge_separate("BFBACKOFFenable",buf1,buf2,final);

    /*merge TxPower*/
    multi_profile_merge_separate("TxPower",buf1,buf2,final);

#ifdef TX_POWER_CONTROL_SUPPORT
	/*merge Tx Power Boost Table*/
	multi_profile_merge_separate("PowerUpCckOfdm",buf1,buf2,final);
	multi_profile_merge_separate("PowerUpHT20",buf1,buf2,final);
	multi_profile_merge_separate("PowerUpHT40",buf1,buf2,final);
	multi_profile_merge_separate("PowerUpVHT20",buf1,buf2,final);
	multi_profile_merge_separate("PowerUpVHT40",buf1,buf2,final);
	multi_profile_merge_separate("PowerUpVHT80",buf1,buf2,final);
	multi_profile_merge_separate("PowerUpVHT160",buf1,buf2,final);
#endif /* TX_POWER_CONTROL_SUPPORT */	

    return NDIS_STATUS_SUCCESS;
}

#ifdef LINK_TEST_SUPPORT
/*
*  LinkTestSupport merge function for multiple profile mode 
*/
static INT multi_profile_merge_linktest(
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{

    /*merge LinkTestSupport*/
    multi_profile_merge_separate("LinkTestSupport",buf1,buf2,final);

    return NDIS_STATUS_SUCCESS;
}
#endif /* LINK_TEST_SUPPORT */

#ifdef MWDS
static INT multi_profile_merge_mwds(
        CHAR *buf1,
        CHAR *buf2,
        CHAR *final)
{
        /*merge ApMWDS*/
     multi_profile_merge_separate("ApMWDS",buf1,buf2,final);

        /*merge ApCliMWDS*/
     multi_profile_merge_separate("ApCliMWDS",buf1,buf2,final);

     return NDIS_STATUS_SUCCESS;
}
#endif

#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11R_FT_SUPPORT
static INT multi_profile_merge_ft(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
    /*merge FtMdId*/
	multi_profile_merge_increase(data,1,"FtMdId",buf1,buf2,final);

	/*merge FtSupport */
	multi_profile_merge_separate("FtSupport",buf1,buf2,final);

	return NDIS_STATUS_SUCCESS;
	
}
#endif
#endif
#ifdef DOT11K_RRM_SUPPORT
static INT multi_profile_merge_rrm(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*merge FtSupport */
	multi_profile_merge_separate("RRMEnable",buf1,buf2,final);

	return NDIS_STATUS_SUCCESS;
	
}
#endif
#ifdef WH_EZ_SETUP
#ifdef CONFIG_AP_SUPPORT
static INT multi_profile_merge_ez_setup(
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{

	/*merge EzEnable*/
	multi_profile_merge_separate("EzEnable",buf1,buf2,final);

	/*merge EzConfStatus*/
	multi_profile_merge_separate("EzConfStatus",buf1,buf2,final);

	/*merge EzGroupID*/
	multi_profile_merge_separate("EzGroupID",buf1,buf2,final);

	/*merge EzGenGroupID*/
	multi_profile_merge_separate("EzGenGroupID",buf1,buf2,final);

	/*merge EzOpenGroupID*/
	multi_profile_merge_separate("EzOpenGroupID",buf1,buf2,final);

	/*merge EzPushBW */
	multi_profile_merge_separate("EzPushBW",buf1,buf2,final);

	/*merge ApCliEzEnable*/
	multi_profile_merge_separate("ApCliEzEnable",buf1,buf2,final);

	/*merge ApCliEzConfStatus*/
	multi_profile_merge_separate("ApCliEzConfStatus",buf1,buf2,final);

	/*merge ApCliEzGroupID*/
	multi_profile_merge_separate("ApCliEzGroupID",buf1,buf2,final);

	/*merge ApCliEzGenGroupID*/
	multi_profile_merge_separate("ApCliEzGenGroupID",buf1,buf2,final);

	/*merge ApCliEzOpenGroupID*/
	multi_profile_merge_separate("ApCliEzOpenGroupID",buf1,buf2,final);

	/*merge ApCliEzRssiThreshold*/
	multi_profile_merge_separate("ApCliEzRssiThreshold",buf1,buf2,final);

	/*merge ApCliHideSSID*/
	multi_profile_merge_separate("ApCliHideSSID",buf1,buf2,final);

	/*merge EzEnable*/
	multi_profile_merge_separate("EzDefaultSsid",buf1,buf2,final);

	/*merge EzEnable*/
	multi_profile_merge_separate("EzDefaultPmk",buf1,buf2,final);

	/*merge EzEnable*/
	multi_profile_merge_separate("EzDefaultPmkValid",buf1,buf2,final);

	return NDIS_STATUS_SUCCESS;

}
#endif /*CONFIG_AP_SUPPORT */
#endif /* WH_EZ_SETUP */

/*
* set second profile and merge it.
*/
static INT multi_profile_merge(
	struct _RTMP_ADAPTER *ad,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	INT retval = NDIS_STATUS_FAILURE;
	struct mpf_data *data = NULL;
	if(multi_profile_merge_dbdc_mode(buf1,buf2,final) != NDIS_STATUS_SUCCESS)
		return retval;
	/*create mpf_data*/
	os_alloc_mem(ad, (UCHAR **)&data, sizeof(struct mpf_data));

	if(!data)
		return retval;

	ad->multi_pf_ctrl = data;
	/*first copy buf1 to final*/
	os_move_mem(final,buf1,MAX_INI_BUFFER_SIZE);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(ad){
		/*merge BssidNum*/
		if(multi_profile_merge_bssidnum(data,buf1,buf2,final) != NDIS_STATUS_SUCCESS)
			return retval;
#ifdef MBSS_SUPPORT
		if(multi_profile_merge_mbss(data,buf1,buf2,final) != NDIS_STATUS_SUCCESS)
			return retval;
#endif /*MBSS_SUPPORT*/

#ifdef APCLI_SUPPORT
		if(multi_profile_merge_apcli(buf1,buf2,final) != NDIS_STATUS_SUCCESS)
			return retval;
#endif /*APCLI_SUPPORT*/

#ifdef BAND_STEERING
		if(multi_profile_merge_bandsteering(data,buf1,buf2,final) != NDIS_STATUS_SUCCESS)
			return retval;
#endif /* BAND_STEERING */
	}
#endif /*CONFIG_AP_SUPPORT*/
	if (multi_profile_merge_protection(buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

	if (multi_profile_merge_frag(buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#ifdef DEFAULT_5G_PROFILE
	if(multi_profile_merge_5g_only(buf2,buf1,final) != NDIS_STATUS_SUCCESS)
		return retval;

#ifdef TXBF_SUPPORT
    if (multi_profile_merge_txbf(buf2,buf1,final) != NDIS_STATUS_SUCCESS)
        return retval;
	
#ifdef TXBF_BY_CHANNEL
	if (multi_profile_merge_bandnobf(buf2,buf1,final) != NDIS_STATUS_SUCCESS)
		return retval;
#endif /* TXBF_BY_CHANNEL */

#endif /*TXBF_SUPPORT*/
	if(multi_profile_merge_2g_only(buf1,buf2,final) != NDIS_STATUS_SUCCESS)
		return retval;
	/* will remove global setting from 2G profile after UI 5G default is ready */
	if(multi_profile_merge_global_setting_only(buf1,buf2,final) != NDIS_STATUS_SUCCESS)
		return retval;
#else
	if(multi_profile_merge_5g_only(buf1,buf2,final) != NDIS_STATUS_SUCCESS)
		return retval;

#ifdef TXBF_SUPPORT
    if (multi_profile_merge_txbf(buf1,buf2,final) != NDIS_STATUS_SUCCESS)
        return retval;
#endif /*TXBF_SUPPORT*/
#endif

    if (multi_profile_merge_txpwr(buf1,buf2,final) != NDIS_STATUS_SUCCESS)
        return retval;

#ifdef LINK_TEST_SUPPORT
    if (multi_profile_merge_linktest(buf1,buf2,final) != NDIS_STATUS_SUCCESS)
        return retval;
#endif /* LINK_TEST_SUPPORT */

#ifdef IGMP_SNOOP_SUPPORT
	if(multi_profile_merge_igmp(buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#endif /* IGMP_SNOOP_SUPPORT */

	if(multi_profile_merge_edcca(buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

#ifdef MWDS
        if(multi_profile_merge_mwds(buf1, buf2, final) != NDIS_STATUS_SUCCESS){
                return retval;
        }
#endif

#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11R_FT_SUPPORT
	if(multi_profile_merge_ft(data,buf1,buf2,final) != NDIS_STATUS_SUCCESS)
			return retval;
#endif
#endif
#ifdef DOT11K_RRM_SUPPORT
	if(multi_profile_merge_rrm(data,buf1,buf2,final) != NDIS_STATUS_SUCCESS)
			return retval;
#endif
#ifdef WH_EZ_SETUP
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(ad){
			/*merge easy setup */
			if(multi_profile_merge_ez_setup(buf1,buf2,final) != NDIS_STATUS_SUCCESS)
			{
			
				EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF,(" multi_profile_merge_ez_setup Failed !!\n"));
				return retval;
			}
		}
#endif /*CONFIG_AP_SUPPORT*/
#endif /* WH_EZ_SETUP */


	data->enable = TRUE;
	/*adjust specific device name*/
	data->specific_dname = TRUE;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (
		"multi-profile merge success, en:%d,pf1_num:%d,pf2_num:%d,total:%d\n",
		data->enable,
		data->pf1_num,
		data->pf2_num,
		data->total_num));
	return NDIS_STATUS_SUCCESS;
}


/*Global function body*/
INT multi_profile_check(struct _RTMP_ADAPTER *ad, CHAR *final)
{
	ULONG buf_size = MAX_INI_BUFFER_SIZE;
	CHAR *buf1 = NULL;
	CHAR *buf2 = NULL;
	INT retval = NDIS_STATUS_FAILURE;
	UCHAR *fname = NULL;
	/*open first profile file*/
	os_alloc_mem(ad, (UCHAR **)&buf1, buf_size);
	if(!buf1)
		goto end;
#ifdef DEFAULT_5G_PROFILE
	fname = multi_profile_fname_get(ad,1);
#else
	fname = multi_profile_fname_get(ad,0);
#endif
	if(multi_profile_read(fname,buf1) != NDIS_STATUS_SUCCESS){
		goto end1;
	}

	/*open second profile file*/		
	os_alloc_mem(ad, (UCHAR **)&buf2, buf_size);

	if(!buf2)
		goto end1;
#ifdef DEFAULT_5G_PROFILE
	fname = multi_profile_fname_get(ad,0);
#else
	fname = multi_profile_fname_get(ad,1);
#endif
	if(multi_profile_read(fname,buf2) != NDIS_STATUS_SUCCESS){
		goto end2;
	}
	/*merge it*/	
	if(multi_profile_merge(ad,buf1,buf2,final) != NDIS_STATUS_SUCCESS){
		goto end2;
	}	
	fname = multi_profile_fname_get(ad,2);
	multi_profile_write(fname,final);
	retval = NDIS_STATUS_SUCCESS;
end2:
	os_free_mem(buf2);
end1:
	os_free_mem(buf1);
end:
	return retval;
}

/*
*
*/
INT	multi_profile_devname_req(struct _RTMP_ADAPTER *ad, UCHAR *final_name, UCHAR *ifidx)
{
	INT card_idx = 0;
	UCHAR *dev_name;
	struct mpf_data *data;
#if defined(MT_FIRST_CARD) || defined(MT_SECOND_CARD) || defined(MT_THIRD_CARD)
	card_idx = get_dev_config_idx(ad);
#endif /* MT_FIRST_CARD || MT_SECOND_CARD || MT_THIRD_CARD */

	if(!ad->multi_pf_ctrl)
		return NDIS_STATUS_SUCCESS;

	data = (struct mpf_data*) ad->multi_pf_ctrl;

	if(!data->enable || !data->specific_dname)
		return NDIS_STATUS_SUCCESS;

	if(*ifidx >= data->pf1_num) {
		dev_name = mtb[card_idx].prefix;
		snprintf(final_name,32,"%s",dev_name);
		*ifidx -= data->pf1_num;
	}
	return NDIS_STATUS_SUCCESS;
}

#ifdef DSCP_QOS_MAP_SUPPORT
INT multi_profile_get_bss_num(struct _RTMP_ADAPTER *ad, UINT8 profile_num)
{
	struct mpf_data *data;
	
	if(!ad->multi_pf_ctrl)
		return 0;
	
	data = (struct mpf_data*) ad->multi_pf_ctrl;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,("MultiProfile profile1 BssNum %d for profile2 BssNum %d \n",data->pf1_num,data->pf2_num));
	if(profile_num == 1)
		return data->pf1_num;
	else
		return data->pf2_num;
}
#endif
INT	multi_profile_apcli_devname_req(struct _RTMP_ADAPTER *ad, UCHAR *final_name, INT *ifidx)
{
	INT card_idx = 0;
	UCHAR *dev_name;
	struct mpf_data *data;
#if defined(MT_FIRST_CARD) || defined(MT_SECOND_CARD) || defined(MT_THIRD_CARD)
	card_idx = get_dev_config_idx(ad);
#endif /* MT_FIRST_CARD || MT_SECOND_CARD || MT_THIRD_CARD */

	if(!ad->multi_pf_ctrl)
		return NDIS_STATUS_SUCCESS;

	data = (struct mpf_data*) ad->multi_pf_ctrl;

	if(!data->enable || !data->specific_dname)
		return NDIS_STATUS_SUCCESS;

	if(*ifidx == 1) {
		/* apcli1 is 2.4G, name is apclix0*/
		dev_name = mtb[card_idx].prefix;
#if defined(BB_SOC) && !defined(MULTI_INF_SUPPORT)
		snprintf(final_name,32,"%s","apcli");
#else
		snprintf(final_name,32,"%s%c",final_name,dev_name[2]);
#endif
	}
	return NDIS_STATUS_SUCCESS;
}
/*
*
*/
VOID multi_profile_exit(struct _RTMP_ADAPTER *ad)
{
	if(ad->multi_pf_ctrl)
		os_free_mem(ad->multi_pf_ctrl);
	ad->multi_pf_ctrl = NULL;
}

/*
*
*/
UCHAR is_multi_profile_enable(struct _RTMP_ADAPTER *ad)
{
	struct mpf_data *data;
	if(!ad->multi_pf_ctrl)
		return FALSE;

	data = (struct mpf_data*) ad->multi_pf_ctrl;

	return data->enable;
}

/*
*
*/
UCHAR multi_profile_get_pf1_num(struct _RTMP_ADAPTER *ad)
{
	struct mpf_data *data;
	if(!ad->multi_pf_ctrl)
		return 0;

	data = (struct mpf_data*) ad->multi_pf_ctrl;

	return data->pf1_num;
}

/*
*
*/
UCHAR multi_profile_get_pf2_num(struct _RTMP_ADAPTER *ad)
{
	struct mpf_data *data;
	if(!ad->multi_pf_ctrl)
		return 0;

	data = (struct mpf_data*) ad->multi_pf_ctrl;

	return data->pf2_num;
}
#endif /*MULTI_PROFILE*/
