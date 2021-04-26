/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	dabs_qos.c
*/

#include "rt_config.h"
#include "mcu/mt_cmd.h"

#ifdef DABS_QOS

#include <net/ip.h>

static const UINT8 ac_queue_to_up[WMM_NUM_OF_AC] = {
	1 /* AC_BK */, 0 /* AC_BE */, 5 /* AC_VI */, 7 /* AC_VO */
};

UINT8  up_to_ac_mapping[NUM_OF_8021D_COS] = {
	WMM_AC_BE /* UP=0 */,   WMM_AC_BK /* UP=1 */,   WMM_AC_BK /* UP=2 */,   WMM_AC_BE /* UP=3 */,
	WMM_AC_VI /* UP=4 */,   WMM_AC_VI /* UP=5 */,   WMM_AC_VO /* UP=6 */,   WMM_AC_VO /* UP=7 */
};

struct qos_param_rec qos_param_table[MAX_QOS_PARAM_TBL];

#define RTMP_SET_PACKET_QOS_IDX(_p, _idx) (*(UINT8 *)&PACKET_CB(_p, 32) = _idx)
#define RTMP_GET_PACKET_QOS_IDX(_p)                (*(UINT8 *)&PACKET_CB(_p, 32))

unsigned short search_qos_param_tbl_idx_by_5_tuple(PRTMP_ADAPTER pAd, struct sk_buff *skb)
{
	struct iphdr *hdr = ip_hdr(skb);
	unsigned short idx;
	unsigned short the_idx = MAX_QOS_PARAM_TBL;
	struct qos_param_rec *prec;

	for (idx = 0; idx < MAX_QOS_PARAM_TBL; idx++) {
		prec = &qos_param_table[idx];

		if ((prec->ip_src == 0) || (prec->ip_dest == 0))
			continue;
		if ((prec->ip_src != hdr->saddr) || (prec->ip_dest != hdr->daddr))
			continue;
		if (prec->protocol != hdr->protocol)
			continue;

		if (prec->protocol == IPPROTO_TCP) {
			if ((prec->sport > 0) && (prec->dport > 0)) {
				struct tcphdr *tcph = (struct tcphdr *)((UINT8*)hdr) + 20;
				if ((prec->sport != tcph->source) ||
					(prec->dport != tcph->dest))
					continue;
			}
		}
		else if (prec->protocol == IPPROTO_UDP) {
			if ((prec->sport > 0) && (prec->dport > 0)) {
				struct udphdr *udph = (struct udphdr *)(((UINT8*)hdr) + 20);
				UINT8* pdata = ((UINT8*)hdr) + 20;

				if ((prec->sport != udph->source) ||
					(prec->dport != udph->dest)) {
					if (pAd->dabs_qos_op & DABS_DBG_PRN_LV1)
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("[%d]dest_ip=%u.%u.%u.%u=port=(%u,%u)!=(%u,%u),\
						(%02X:%02X:%02X%02X)\n",
						idx, NIPQUAD(hdr->daddr),
					 	ntohs(prec->sport), ntohs(prec->dport),
						ntohs(udph->source), ntohs(udph->dest),
						pdata[2], pdata[3],pdata[4],pdata[5]));
					continue;
				}	
			}	
		}

		the_idx = idx;
		break;
	}

	return the_idx;
}

unsigned short search_qos_param_tbl_idx_by_wlan_idx(
	unsigned short wlan_idx,
	unsigned short proto,
	unsigned short dport)
{
	unsigned short idx;
	unsigned short the_idx = MAX_QOS_PARAM_TBL;
	struct qos_param_rec *prec;

	for (idx = 0; idx < MAX_QOS_PARAM_TBL; idx++) {
		prec = &qos_param_table[idx];
		if (prec->valid == FALSE)
			continue;

		if ((prec->protocol != proto) && (prec->protocol != 0))
			continue;

		if ((prec->dport != dport) && (prec->dport != 0))
			continue;

		if (prec->wlan_idx == wlan_idx) {
			the_idx = idx;
			break;
		}
	}

	return the_idx;
}
bool set_qos_param_tbl_wlan_idx_by_idx(unsigned short idx, unsigned short wlan_idx)
{
	bool ret = true;
	struct qos_param_rec *ptarget_qos_param;

	if (idx >= MAX_QOS_PARAM_TBL)
		return false;

	ptarget_qos_param = &qos_param_table[idx];

	ptarget_qos_param->wlan_idx = wlan_idx;
	return ret;
}

bool set_qos_param_tbl_ipaddr_by_idx(
	unsigned short idx, 
	struct qos_param_rec *pqos_param
)
{
	bool ret = true;
	struct qos_param_rec *ptarget_qos_param;
	if (idx >= MAX_QOS_PARAM_TBL)
		return false;

	ptarget_qos_param = &qos_param_table[idx];

	ptarget_qos_param->ip_src = pqos_param->ip_src;
	ptarget_qos_param->ip_dest = pqos_param->ip_dest;
	ptarget_qos_param->protocol = pqos_param->protocol;
	ptarget_qos_param->sport = pqos_param->sport;
	ptarget_qos_param->dport = pqos_param->dport;

	return ret;
}

bool set_qos_param_tbl_qos_by_idx(unsigned short idx, struct qos_param_rec *pqos_param)
{
	bool ret = true;
	struct qos_param_rec *ptarget_qos_param;
	if (idx >= MAX_QOS_PARAM_TBL)
		return false;

	ptarget_qos_param = &qos_param_table[idx];

	ptarget_qos_param->priority = ac_queue_to_up[pqos_param->priority];
	ptarget_qos_param->delay_bound = pqos_param->delay_bound;
	ptarget_qos_param->delay_req = pqos_param->delay_req;
	ptarget_qos_param->delay_weight = pqos_param->delay_weight;
	ptarget_qos_param->bw_req = pqos_param->bw_req;
	ptarget_qos_param->data_rate = pqos_param->data_rate;
	ptarget_qos_param->dir = pqos_param->dir;
	ptarget_qos_param->drop_thres = pqos_param->drop_thres;
	ptarget_qos_param->app_type = pqos_param->app_type;
/*
	if (ptarget_qos_param->delay_req <= 10)
		ptarget_qos_param->priority = ac_queue_to_up[3];
	else if (ptarget_qos_param->delay_req <= 50)
		ptarget_qos_param->priority = ac_queue_to_up[2];
	else if (ptarget_qos_param->delay_req <= 500)
		ptarget_qos_param->priority = ac_queue_to_up[1];
	else
		ptarget_qos_param->priority = ac_queue_to_up[0];
*/	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s:Set qos: prio:%u,delay_bound:%u,delay_req:%u,weight=%u,data_rate:%u,dir:%u,dropth:%u,app_type:%u\n", __func__,
		ptarget_qos_param->priority,
		ptarget_qos_param->delay_bound,
		ptarget_qos_param->delay_req,
		ptarget_qos_param->delay_weight,
		ptarget_qos_param->data_rate,
		ptarget_qos_param->dir,
		ptarget_qos_param->drop_thres,
		ptarget_qos_param->app_type
		));

	return ret;
}

bool set_qos_param_tbl_by_idx(unsigned short idx, struct qos_param_rec *pqos_param)
{
	bool ret = true;

	if (idx >= MAX_QOS_PARAM_TBL)
		return false;

	memcpy(&qos_param_table[idx], pqos_param, sizeof(struct qos_param_rec));

	return ret;
}

bool enable_qos_param_tbl_by_idx(unsigned short idx)
{
	bool ret = true;

	if (idx >= MAX_QOS_PARAM_TBL)
		return false;

	qos_param_table[idx].valid = true;

	return ret;
}

bool disable_qos_param_tbl_by_idx(unsigned short idx)
{
	bool ret = true;

	if (idx >= MAX_QOS_PARAM_TBL)
		return false;

	qos_param_table[idx].valid = false;

	return ret;
}

bool set_qos_param_to_fw(PRTMP_ADAPTER pAd, unsigned short idx)
{
	bool ret = true;
	MURU_QOS_SETTING qos_setting = {0};
	struct qos_param_rec *pqos_param;

	if (idx >= MAX_QOS_PARAM_TBL)
		return false;

	pqos_param = &qos_param_table[idx]; 

	if (pqos_param->valid == false)
		return false;

	pqos_param = &qos_param_table[idx]; 
	qos_setting.u2WlanIdx = pqos_param->wlan_idx;
	qos_setting.u1AC = up_to_ac_mapping[pqos_param->priority];
	qos_setting.u2DelayBound = pqos_param->delay_bound;
	qos_setting.u2DelayReq = pqos_param->delay_req;
	qos_setting.u1DelayWeight = pqos_param->delay_weight;
	qos_setting.u2DataRate = pqos_param->data_rate;
	qos_setting.u2BWReq = pqos_param->bw_req;
	qos_setting.u1Dir = pqos_param->dir;
	qos_setting.u2DropThres = pqos_param->drop_thres;

	if (SendQoSCmd(pAd, QOS_CMD_PARAM_SETTING, &qos_setting) == false)
		ret = false;

	return ret;
}

INT set_dabs_qos_param(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	UINT32 cmd = 3, rv, ret = 1, idx = 0;
	UINT32 param[12] = {0};
	UINT32 ip_src[4] = {0}, ip_dest[4] = {0};
	MURU_QOS_SETTING qos_setting = {0};
	struct qos_param_rec target_qos_param, *pqos_param;

	if (arg) {
		rv = sscanf(arg, "%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u", &cmd, &param[0], &param[1], &param[2],
			&param[3], &param[4], &param[5], &param[6], &param[7], &param[8], &param[9], &param[10]);

		if (rv == 0) {
			ret = 0;
			goto error;
		}
		switch (cmd) {
		case 0:
			rv = sscanf(arg, "%u-%u-%u.%u.%u.%u-%u.%u.%u.%u-%u-%u-%u",
				&cmd, &param[0],
				&ip_src[0], &ip_src[1], &ip_src[2], &ip_src[3],
				&ip_dest[0], &ip_dest[1], &ip_dest[2], &ip_dest[3],
				&param[3], &param[4], &param[5]);

			param[1] = (ip_src[3]<<24) | (ip_src[2] << 16) | (ip_src[1] << 8)
				| (ip_src[0] << 0);
			param[2] = (ip_dest[3]<<24) | (ip_dest[2] << 16) | (ip_dest[1] << 8)
				| (ip_dest[0] << 0);

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
				("ip_src=%u.%u.%u.%u, ip_dest=%u.%u.%u.%u\n",
				NIPQUAD(param[1]), NIPQUAD(param[2])));

			target_qos_param.ip_src = param[1];
			target_qos_param.ip_dest = param[2];
			target_qos_param.protocol = param[3];
			target_qos_param.sport = htons(param[4]);
			target_qos_param.dport = htons(param[5]);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
				("ip_src=%u.%u.%u.%u, ip_dest=%u.%u.%u.%u,proto=%u,sp=%u,dp=%u\n",
				NIPQUAD(target_qos_param.ip_src),
				NIPQUAD(target_qos_param.ip_dest),
				target_qos_param.protocol,
				ntohs(target_qos_param.sport),
				ntohs(target_qos_param.dport)));


			set_qos_param_tbl_ipaddr_by_idx(param[0], &target_qos_param);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: set qos_param ipaddr for idx:%u\n", __func__, param[0]));
			break;
		case 1:
			target_qos_param.priority = param[2]; /* up to ac by set_qos_param_tbl_qos_by_idx */
			target_qos_param.delay_bound = param[3];
			target_qos_param.delay_req = param[4];
			target_qos_param.delay_weight = param[5];
			target_qos_param.data_rate = param[6];
			target_qos_param.bw_req = param[7];
			target_qos_param.dir = param[8];
			target_qos_param.drop_thres = param[9];
			target_qos_param.app_type = param[10];
			set_qos_param_tbl_qos_by_idx(param[0], &target_qos_param);
			if (param[1] > 0)
				set_qos_param_tbl_wlan_idx_by_idx(param[0], param[1]);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: set qos_param qos related for idx:%u\n", __func__, param[0]));
			if ((target_qos_param.delay_req > 0) && (target_qos_param.delay_req < 65535)) {
				if (pAd->pbc_bound[PBC_AC_BK] == PBC_WMM_UP_DEFAULT_BK)
				{
					pAd->pbc_bound[PBC_AC_VO] = 1200;
					pAd->pbc_bound[PBC_AC_VI] = 4000;
					pAd->pbc_bound[PBC_AC_BE] = 1200;
					pAd->pbc_bound[PBC_AC_BK] = 1000;

					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("%s: set pbc_ubound=[%u,%u,%u,%u]\n", __func__,
						pAd->pbc_bound[PBC_AC_BK], pAd->pbc_bound[PBC_AC_BE],
						pAd->pbc_bound[PBC_AC_VI], pAd->pbc_bound[PBC_AC_VO]));
				}
			}
			break;
		case 2:
			set_qos_param_tbl_wlan_idx_by_idx(param[0], param[1]);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: set qos_param wlan:%u related for idx:%u\n", __func__, param[1], param[0]));
			break;
		case 3:
			if (param[1] != 0) {
				enable_qos_param_tbl_by_idx(param[0]);
				if (set_qos_param_to_fw(pAd, param[0]) == FALSE)
				{
					ret = 0;
					disable_qos_param_tbl_by_idx(param[0]);
				}
			}	
			else
				disable_qos_param_tbl_by_idx(param[0]);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: qos_param tbl[%d] valid:%u\n", __func__, param[0],param[1]));
			break;
		case 4:
			for (idx = 0; idx < MAX_QOS_PARAM_TBL; idx++) {
				pqos_param = &qos_param_table[idx];
				if (pqos_param->ip_dest == 0)
					continue;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
("%s: dump qos_param[%d]: ip_src=%u.%u.%u.%u,ip_dest=%u.%u.%u.%u,proto=%u,sport=%u,dport=%u\n", __func__,
				idx,
			NIPQUAD(pqos_param->ip_src),
			NIPQUAD(pqos_param->ip_dest),
			pqos_param->protocol,
			ntohs(pqos_param->sport),
			ntohs(pqos_param->dport)
			));

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: dump qos_param[%d]: valid:%u,prio:%u,delay_bound:%u,delay_req:%u,delay_weight:%u,data_rate:%u,bw_req:%u,dir=%u,dropth=%u,app_type:%u\n", __func__,
			param[0],
			pqos_param->valid,
			pqos_param->priority,
			pqos_param->delay_bound,
			pqos_param->delay_req,
			pqos_param->delay_weight,
			pqos_param->data_rate,
			pqos_param->bw_req,
			pqos_param->dir,
			pqos_param->drop_thres,
			pqos_param->app_type
			));
			}	
			SendQoSCmd(pAd, QOS_CMD_PARAM_DUMP, &qos_setting);
			break;
		case 5:
			pAd->dabs_qos_op = param[0];
			if (pAd->dabs_qos_op &DABS_DBG_DLY_TIME)
				net_enable_timestamp();
			else
	                        net_disable_timestamp();
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s:dabs_qos_op:%u\n", __func__, param[0]));
			break;
		case 6:
			ret = SendQoSCmd(pAd, QOS_CMD_PARAM_RESET, &qos_setting);
			memset(&qos_param_table[0], 0, sizeof(struct qos_param_rec)*MAX_QOS_PARAM_TBL);
			pAd->pbc_bound[PBC_AC_BE] = PBC_WMM_UP_DEFAULT_BE;
			pAd->pbc_bound[PBC_AC_BK] = PBC_WMM_UP_DEFAULT_BK;
			pAd->pbc_bound[PBC_AC_VO] = PBC_WMM_UP_DEFAULT_VO;
			pAd->pbc_bound[PBC_AC_VI] = PBC_WMM_UP_DEFAULT_VI;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: clean up table ret=%u\n",__func__, ret));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: reset pbc_ubound=[%u,%u,%u,%u]\n", __func__,
				pAd->pbc_bound[PBC_AC_BK], pAd->pbc_bound[PBC_AC_BE],
				pAd->pbc_bound[PBC_AC_VI], pAd->pbc_bound[PBC_AC_VO]));

			break;
		case 7:
			for (idx = 0; idx < MAX_QOS_PARAM_TBL; idx++) {
				pqos_param = &qos_param_table[idx];
				if (pqos_param->tot_pkt_cnt > 0) {
					unsigned int avg_dly;
					avg_dly = div64_ul(pqos_param->tot_pkt_dly, pqos_param->tot_pkt_cnt);	
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("qos[%d]:dly_req:%u,avg_dly:%u(max=%u)[%llu/%u]\n", idx,
						pqos_param->delay_req,
						avg_dly,
						pqos_param->max_pkt_dly,
						pqos_param->tot_pkt_dly,
						pqos_param->tot_pkt_cnt));
					pqos_param->tot_pkt_dly = 0;
					pqos_param->tot_pkt_cnt = 0;
					pqos_param->max_pkt_dly = 0;
				}
			}
			SendQoSCmd(pAd, QOS_CMD_RESULT_DUMP, &qos_setting);
			break;
		default:
			ret = 0;
			goto error;
		}
	}

error:

	if (ret == 0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,(
			"iwpriv ra0 set dabs_qos=0-[idx]-[ip_src]-[ip_dest]-[proto]-[sport]-[dport] (set ip by idx)\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,(
			"iwpriv ra0 set dabs_qos=1-[idx]-[wlanid]-[AC]-[DlyBound]-[DlyReq]-[DlyWt]-[Rate]-[BWReq]-[Dir]-[DropTh] (set qos by idx)\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,(
			"iwpriv ra0 set dabs_qos=4 (dump table)\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,(
			"iwpriv ra0 set dabs_qos=5-[dabs_qos_op(bit2:] (set dabs operation)\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,(
			"iwpriv ra0 set dabs_qos=6 (clean up table)\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,(
			"iwpriv ra0 set dabs_qos=7 (dump host delay)\n"));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:\n", __func__));


	return ret;
}

void dabs_host_delay(PRTMP_ADAPTER pAd,  NDIS_PACKET *pkt)
{
	UINT16 qos_idx;

	if (pkt == NULL)
		return;

	qos_idx = RTMP_GET_PACKET_QOS_IDX(pkt);

	if (qos_idx < MAX_QOS_PARAM_TBL) {
		struct qos_param_rec *prec;
		prec = &qos_param_table[qos_idx];
		if (prec->valid == TRUE) {
			ktime_t delta_kt = net_timedelta(RTPKT_TO_OSPKT(pkt)->tstamp);
			UINT32 delta_us = 0;
			delta_us = ktime_to_us(delta_kt);

			prec->tot_pkt_cnt++;
			prec->tot_pkt_dly += delta_us;

			if (delta_us > prec->max_pkt_dly) {
				prec->max_pkt_dly = delta_us;
			}

			if (pAd->dabs_qos_op & DABS_DBG_PRN_LV2) {
				prec->some_pkt_dly += delta_us;
				if ((prec->tot_pkt_cnt % 1000) == 999) {
					prec->avg_pkt_dly =  prec->some_pkt_dly/1000;
					prec->some_pkt_dly = 0;
				}
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("qos[%d] deltaus=%u\n",qos_idx, delta_us));
			}
		}
	}

	return;
}

void dabs_active_qos_by_ipaddr(PRTMP_ADAPTER pAd,  NDIS_PACKET *pkt)
{
	UINT32 idx = MAX_QOS_PARAM_TBL;
	UINT8 *pData = GET_OS_PKT_DATAPTR(pkt);
	UINT32 ip_dest, ip_src, proto, sport = 0,dport = 0;

	pData += (MAT_ETHER_HDR_LEN);
	ip_src = *((UINT32*)(pData + 12));
	ip_dest = *((UINT32*)(pData + 16));
	proto = *((UINT8*)(pData + 9));

	if ((ip_src != 0) && (ip_dest != 0) && ((ip_dest & 0x0FF) != 0x0FF)) {
		if (RTMP_GET_PACKET_WCID(pkt) < (MAX_LEN_OF_MAC_TABLE - 16)) {
			idx = search_qos_param_tbl_idx_by_5_tuple(pAd, RTPKT_TO_OSPKT(pkt));
			if (pAd->dabs_qos_op & DABS_DBG_PRN_LV2) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("WCID%u,hit=%u src=%u.%u.%u.%u,dest=%u.%u.%u.%u, proto=%u,sp=%u,dp=%u\n",
					RTMP_GET_PACKET_WCID(pkt), idx,
					NIPQUAD(ip_src), NIPQUAD(ip_dest), proto, sport, dport));
			}
		}
	}

	RTMP_SET_PACKET_QOS_IDX(pkt, MAX_QOS_PARAM_TBL);

	if (idx < MAX_QOS_PARAM_TBL) {
		struct qos_param_rec *prec;
		prec = &qos_param_table[idx];

		if (prec->valid == FALSE) {
			set_qos_param_tbl_wlan_idx_by_idx(idx, RTMP_GET_PACKET_WCID(pkt));
			if (pAd->dabs_qos_op & DABS_DBG_PRN_LV1) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("Set QTBL for STA%u,idx=%u,src=%u.%u.%u.%u,dest=%u.%u.%u.%u, proto=%u,sp=%u,dp=%u\n",
					RTMP_GET_PACKET_WCID(pkt), idx,
					NIPQUAD(ip_src), NIPQUAD(ip_dest), proto, htons(sport), htons(dport)));
			}
#if defined(VOW_SUPPORT) && defined(VOW_DVT)
			pAd->vow_sta_ac[RTMP_GET_PACKET_WCID(pkt)] = up_to_ac_mapping[prec->priority];
#endif
			enable_qos_param_tbl_by_idx(idx);

			if (pAd->dabs_qos_op & DABS_SET_QOS_PARAM) {
				if (set_qos_param_to_fw(pAd, idx) == FALSE)
					disable_qos_param_tbl_by_idx(idx);
			}
		} else {
			RTMP_SET_PACKET_QOS_IDX(pkt, idx);
		}
	}

	return;
}
#endif /* DABS_QOS */
