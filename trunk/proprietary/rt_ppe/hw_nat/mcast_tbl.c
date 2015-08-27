#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>

#include "frame_engine.h"
#include "mcast_tbl.h"

extern spinlock_t ppe_foe_lock;

static int32_t mcast_entry_get(uint16_t vlan_id, uint8_t *dst_mac)
{
	int i;

	for(i=0; i<MAX_MCAST_ENTRY; i++) {
		if ((GET_PPE_MCAST_H(i)->mc_vid == vlan_id) &&
		     GET_PPE_MCAST_L(i)->mc_mac_addr[3] == dst_mac[2] &&
		     GET_PPE_MCAST_L(i)->mc_mac_addr[2] == dst_mac[3] &&
		     GET_PPE_MCAST_L(i)->mc_mac_addr[1] == dst_mac[4] &&
		     GET_PPE_MCAST_L(i)->mc_mac_addr[0] == dst_mac[5]) {
			if (GET_PPE_MCAST_H(i)->mc_mpre_sel == 0) {
				if (dst_mac[0] == 0x1 && dst_mac[1] == 0x00)
					return i;
			} else if (GET_PPE_MCAST_H(i)->mc_mpre_sel == 1) {
				if (dst_mac[0] == 0x33 && dst_mac[1] == 0x33)
					return i;
			}
		}
	}

	return -1;
}

#if defined (CONFIG_RA_HW_NAT_QDMA)
/* must be protected by spinlock ppe_foe_lock */
int foe_mcast_entry_qid(uint16_t vlan_id, uint8_t *dst_mac, uint8_t mc_qos_qid)
{
	int entry_num, ret;
	ppe_mcast_h *mcast_h;

	NAT_DEBUG("%s: vid=%d, mac=%x:%x:%x:%x:%x:%x, mc_qos_qid=%d\n",
		__FUNCTION__, vlan_id, dst_mac[0],dst_mac[1],dst_mac[2],dst_mac[3],dst_mac[4],dst_mac[5],
		mc_qos_qid);

	ret = 1;

	/* update exist entry */
	if ((entry_num = mcast_entry_get(vlan_id, dst_mac)) >= 0) {
		mcast_h = GET_PPE_MCAST_H(entry_num);
		mcast_h->mc_qos_qid = mc_qos_qid;
		ret = 0;
	}

	return ret;
}
#endif

/*
  mc_px_en: enable multicast to port x
  mc_px_qos_en: enable QoS for multicast to port x

  - multicast port0 map to PDMA
  - multicast port1 map to GMAC1
  - multicast port2 map to GMAC2
  - multicast port3 map to QDMA
*/
int foe_mcast_entry_ins(uint16_t vlan_id, uint8_t *dst_mac, uint8_t mc_px_en, uint8_t mc_px_qos_en, uint8_t mc_qos_qid)
{
	int i = 0;
	int entry_num, ret;
	uint32_t mc_mpre_sel;
	ppe_mcast_h *mcast_h;
	ppe_mcast_l *mcast_l;

	NAT_DEBUG("%s: vid=%d, mac=%x:%x:%x:%x:%x:%x, mc_px_en=%x, mc_px_qos_en=%x, mc_qos_qid=%d\n",
		__FUNCTION__, vlan_id, dst_mac[0],dst_mac[1],dst_mac[2],dst_mac[3],dst_mac[4],dst_mac[5],
		mc_px_en, mc_px_qos_en, mc_qos_qid);

	ret = 1;

	if (dst_mac[0] == 0x1 && dst_mac[1] == 0x00)
		mc_mpre_sel = 0;
	else if (dst_mac[0] == 0x33 && dst_mac[1] == 0x33)
		mc_mpre_sel = 1;
	else
		return ret;

	spin_lock_bh(&ppe_foe_lock);

	/* update exist entry */
	if ((entry_num = mcast_entry_get(vlan_id, dst_mac)) >= 0) {
		mcast_h = GET_PPE_MCAST_H(entry_num);
		mcast_l = GET_PPE_MCAST_L(entry_num);
		mcast_h->mc_mpre_sel = mc_mpre_sel;
		mcast_h->mc_px_en |= mc_px_en;
#if defined (CONFIG_RA_HW_NAT_QDMA)
		/* raeth must alloc FQ pool to handle PPE -> FQ */
		mcast_h->mc_px_qos_en |= mc_px_qos_en;
		mcast_h->mc_qos_qid = mc_qos_qid;
#endif
		ret = 0;
	} else {
		/* create new entry */
		for (i=0;i<MAX_MCAST_ENTRY;i++) {
			mcast_h = GET_PPE_MCAST_H(i);
			mcast_l = GET_PPE_MCAST_L(i);
			if (mcast_h->valid == 0) {
				mcast_h->mc_vid = vlan_id;
				mcast_h->mc_mpre_sel = mc_mpre_sel;
				mcast_h->mc_px_en = mc_px_en;
#if defined (CONFIG_RA_HW_NAT_QDMA)
				/* raeth must alloc FQ pool to handle PPE -> FQ */
				mcast_h->mc_px_qos_en = mc_px_qos_en;
				mcast_h->mc_qos_qid = mc_qos_qid;
#endif
				mcast_l->mc_mac_addr[3] = dst_mac[2];
				mcast_l->mc_mac_addr[2] = dst_mac[3];
				mcast_l->mc_mac_addr[1] = dst_mac[4];
				mcast_l->mc_mac_addr[0] = dst_mac[5];
				mcast_h->valid = 1;
				ret = 0;
				break;
			}
		}
	}

	spin_unlock_bh(&ppe_foe_lock);

	if (ret)
		NAT_PRINT("HNAT: Multicast Table is FULL!!\n");

	return ret;
}

/*
 * Return:
 *	    0: entry found
 *	    1: entry not found
 */
int foe_mcast_entry_del(uint16_t vlan_id, uint8_t *dst_mac, uint8_t mc_px_en, uint8_t mc_px_qos_en)
{
	int entry_num, ret;
	ppe_mcast_h *mcast_h;
	ppe_mcast_l *mcast_l;

	NAT_DEBUG("%s: vid=%d, mac=%x:%x:%x:%x:%x:%x, mc_px_en=%x, mc_px_qos_en=%x\n",
		__FUNCTION__, vlan_id, dst_mac[0],dst_mac[1],dst_mac[2],dst_mac[3],dst_mac[4],dst_mac[5],
		mc_px_en, mc_px_qos_en);

	ret = 1;

	spin_lock_bh(&ppe_foe_lock);
	if ((entry_num = mcast_entry_get(vlan_id, dst_mac)) >= 0) {
		mcast_h = GET_PPE_MCAST_H(entry_num);
		mcast_l = GET_PPE_MCAST_L(entry_num);
		mcast_h->mc_px_en &= ~mc_px_en;
		mcast_h->mc_px_qos_en &= ~mc_px_qos_en;
		
		if (mcast_h->mc_px_en == 0 && mcast_h->mc_px_qos_en == 0) {
			mcast_h->valid = 0;
			mcast_h->mc_vid = 0;
			mcast_h->mc_qos_qid = 0;
			memset(&mcast_l->mc_mac_addr, 0, 4);
		}
		ret = 0;
	}
	spin_unlock_bh(&ppe_foe_lock);

	return ret;
}

void foe_mcast_entry_dump(void)
{
	int i;
	ppe_mcast_h *mcast_h;
	ppe_mcast_l *mcast_l;

	printk("MAC               |  VID | Ports | QPorts | QID | IPv6\n");
//	       "xx:xx:00:00:00:00   4094    ----     ----     0      0"
	for(i=0;i<MAX_MCAST_ENTRY;i++) {
		mcast_h = GET_PPE_MCAST_H(i);
		mcast_l = GET_PPE_MCAST_L(i);
		printk("xx:xx:%02x:%02x:%02x:%02x   %4d    %c%c%c%c     %c%c%c%c    %2d      %d\n",
			mcast_l->mc_mac_addr[3], mcast_l->mc_mac_addr[2],
			mcast_l->mc_mac_addr[1], mcast_l->mc_mac_addr[0],
			mcast_h->mc_vid,
			(mcast_h->mc_px_en & 0x08)?'1':'-',
			(mcast_h->mc_px_en & 0x04)?'1':'-',
			(mcast_h->mc_px_en & 0x02)?'1':'-',
			(mcast_h->mc_px_en & 0x01)?'1':'-',
			(mcast_h->mc_px_qos_en & 0x08)?'1':'-',
			(mcast_h->mc_px_qos_en & 0x04)?'1':'-',
			(mcast_h->mc_px_qos_en & 0x02)?'1':'-',
			(mcast_h->mc_px_qos_en & 0x01)?'1':'-',
			mcast_h->mc_qos_qid,
			mcast_h->mc_mpre_sel);
	}
}

void foe_mcast_entry_del_all(void)
{
	int i;
	ppe_mcast_h *mcast_h;
	ppe_mcast_l *mcast_l;

	for(i=0;i<MAX_MCAST_ENTRY;i++) {
		mcast_h = GET_PPE_MCAST_H(i);
		mcast_l = GET_PPE_MCAST_L(i);
		mcast_h->mc_px_en = 0;
		mcast_h->mc_px_qos_en = 0;
		mcast_h->valid = 0;
		mcast_h->mc_vid = 0;
		mcast_h->mc_qos_qid = 0;
		mcast_h->mc_mpre_sel = 0;
		memset(&mcast_l->mc_mac_addr, 0, 4);
	}
}
