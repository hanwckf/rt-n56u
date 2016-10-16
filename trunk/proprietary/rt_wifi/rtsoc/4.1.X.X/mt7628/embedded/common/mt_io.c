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
	mt_io.c
*/
#include	"rt_config.h"

#if defined(MT7603) || defined(MT7628) || defined(MT7636)
UINT32 mt_dft_mac_cr_range[] = {
	0x50022000, 0x50022000, 0xc84,	 /* USB Controller */
	0x50029000, 0x50029000, 0x210,	 /* USB DMA */
	0x800c006c, 0x800c006c, 0x100,	/* PSE Client */
	0x60000000, 0x20000, 0x200, /* WF_CFG */
	0x60100000, 0x21000, 0x200, /* WF_TRB */
	0x60110000, 0x21200, 0x200, /* WF_AGG */
	0x60120000, 0x21400, 0x200, /* WF_ARB */
	0x60130000, 0x21600, 0x200, /* WF_TMAC */
	0x60140000, 0x21800, 0x200, /* WF_RMAC */
	0x60150000, 0x21A00, 0x200, /* WF_SEC */
	0x60160000, 0x21C00, 0x200, /* WF_DMA */
	0x60170000, 0x21E00, 0x200, /* WF_CFGOFF */
	0x60180000, 0x22000, 0x1000, /* WF_PF */
	0x60190000, 0x23000, 0x200, /* WF_WTBLOFF */
	0x601A0000, 0x23200, 0x200, /* WF_ETBF */
	
	0x60300000, 0x24000, 0x400, /* WF_LPON */
	0x60310000, 0x24400, 0x200, /* WF_INT */
	0x60320000, 0x28000, 0x4000, /* WF_WTBLON */
	0x60330000, 0x2C000, 0x200, /* WF_MIB */
	0x60400000, 0x2D000, 0x200, /* WF_AON */

	0x80020000, 0x00000, 0x2000, /* TOP_CFG */	
	0x80000000, 0x02000, 0x2000, /* MCU_CFG */
	0x50000000, 0x04000, 0x4000, /* PDMA_CFG */
#if defined(MT7603_FPGA) || defined(MT7628_FPGA)  || defined(MT7636_FPGA)
	0xA0000000, 0x80000, 0x10000, /* PSE_CFG after remap 2 */
#else
	0xA0000000, 0x08000, 0x8000, /* PSE_CFG */
#endif /* MT7603_FPGA */
	0x60200000, 0x10000, 0x10000, /* WF_PHY */
	
	0x0, 0x0, 0x0,
};
#endif /* defined(MT7603) || defined(MT7628) || defined(MT7636) */


#ifdef MT7615
UINT32 mt7615_mac_cr_range[] = {
	0x820d0000, 0x20000, 0x200, /* WF_AON */
	0x820f0000, 0x21000, 0x200, /* WF_CFG */
	0x820f0800, 0x21200, 0x200, /* WF_AGG */
	0x820f1000, 0x21400, 0x200, /* WF_ARB */
	0x820f2000, 0x21600, 0x200, /* WF_TMAC */
	0x820f3000, 0x21800, 0x200, /* WF_RMAC */
	0x820f4000, 0x21A00, 0x200, /* WF_SEC */
	0x820f5000, 0x21C00, 0x200, /* WF_DMA */
	0x820f6000, 0x21E00, 0x200, /* WF_CFGOFF */
	0x820f7000, 0x22000, 0x1000, /* WF_PF */
	0x820f7200, 0x23000, 0x200, /* WF_WTBLOFF */
	0x820f8000, 0x23200, 0x200, /* WF_ETBF */
	
	0x820f9000, 0x24000, 0x400, /* WF_LPON */
	0x820f9800, 0x24400, 0x200, /* WF_INT */
	0x820fa000, 0x28000, 0x4000, /* WF_WTBLON */
	0x820fb000, 0x2C000, 0x200, /* WF_MIB */
	0x820fc000, 0x2D000, 0x200, /* WF_AON */
	0x820fd000, 0x2D000, 0x200, /* WF_AON */
	0x820fe000, 0x2D000, 0x200, /* WF_AON */
	0x820e0000, 0x2D000, 0x200, /* WF_AON */
	
	0x80020000, 0x00000, 0x2000, /* TOP_CFG */	
	0x80000000, 0x02000, 0x2000, /* MCU_CFG */
	0x50000000, 0x04000, 0x4000, /* PDMA_CFG */
#if defined(MT7603_FPGA) || defined(MT7628_FPGA)  || defined(MT7636_FPGA)
	0xA0000000, 0x80000, 0x10000, /* PSE_CFG after remap 2 */
#else
	0xA0000000, 0x08000, 0x8000, /* PSE_CFG */
#endif /* MT7603_FPGA */
	0x60200000, 0x10000, 0x10000, /* WF_PHY */
	
	0x0, 0x0, 0x0,
};
#endif /* MT7615 */


BOOLEAN mt_mac_cr_range_mapping(RTMP_ADAPTER *pAd, UINT32 *mac_addr)
{
	UINT32 mac_addr_hif = *mac_addr;
	INT idx = 0;
	BOOLEAN IsFound = 0;
	UINT32 *mac_cr_range = NULL;

#if defined(MT7603) || defined(MT7628) || defined(MT7636)
	if (IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT76x6(pAd))
		mac_cr_range = &mt_dft_mac_cr_range[0];
#endif /* defined(MT7603) || defined(MT7628) || defined(MT7636) */

#ifdef MT7615
	if (IS_MT7615(pAd))
		mac_cr_range = &mt7615_mac_cr_range[0];
#endif /* MT7615 */

	if (!mac_cr_range)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): NotSupported Chip for this function!\n", __FUNCTION__));
		return IsFound;
	}

	if (mac_addr_hif > 0x4ffff)
	{
		do
		{
			if (mac_addr_hif >= mac_cr_range[idx] && 
				mac_addr_hif < (mac_cr_range[idx] + mac_cr_range[idx + 2]))
			{
				mac_addr_hif -= mac_cr_range[idx];
				mac_addr_hif += mac_cr_range[idx + 1];
				IsFound = 1;
				break;
			}
			idx += 3;
		} while (mac_cr_range[idx] != 0);
	} 
	else
	{
		IsFound = 1;
	}

	*mac_addr = mac_addr_hif;

	return IsFound;
}


UINT32 mt_physical_addr_map(RTMP_ADAPTER *pAd, UINT32 addr)
{
	UINT32 global_addr = 0x0, idx = 1;
	UINT32 wtbl_2_base = pAd->chipCap.WtblPseAddr;

	if (addr < 0x2000)
		global_addr = 0x80020000 + addr;
	else if ((addr >= 0x2000) && (addr < 0x4000))
		global_addr = 0x80000000 + addr - 0x2000;
	else if ((addr >= 0x4000) && (addr < 0x8000))
		global_addr = 0x50000000 + addr - 0x4000;
	else if ((addr >= 0x8000) && (addr < 0x10000))
		global_addr = 0xa0000000 + addr - 0x8000;
	else if ((addr >= 0x10000) && (addr < 0x20000))
		global_addr = 0x60200000 + addr - 0x10000;
	else if ((addr >= 0x20000) && (addr < 0x40000))
	{
		UINT32 *mac_cr_range = NULL;

#if defined(MT7603) || defined(MT7628) || defined(MT7636)
		if (IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT7636(pAd))
			mac_cr_range = &mt_dft_mac_cr_range[0];
#endif /* defined(MT7603) || defined(MT7628) || defined(MT7636) */

#ifdef MT7615
		if (IS_MT7615(pAd))
			mac_cr_range = &mt7615_mac_cr_range[0];
#endif /* MT7615 */

		if (!mac_cr_range)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): NotSupported Chip for this function!\n", __FUNCTION__));
			return global_addr;
		}

		do {
			if ((addr >= mac_cr_range[idx]) && (addr < (mac_cr_range[idx]+mac_cr_range[idx+1]))) {
				global_addr = mac_cr_range[idx-1]+(addr-mac_cr_range[idx]);
				break;
			}
			idx += 3;
		} while (mac_cr_range[idx] != 0);

		if (mac_cr_range[idx] == 0)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("unknow addr range = %x\n", addr));
		}
	}
	else if ((addr >= 0x40000) && (addr < 0x80000)) //WTBL Address
	{
		global_addr = wtbl_2_base + addr - 0x40000;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("==>global_addr1=0x%x\n", global_addr));
	}
	else if ((addr >= 0xc0000) && (addr < 0xc0100)) //PSE Client
	{
		global_addr = 0x800c0000 + addr - 0xc0000;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("==>global_addr2=0x%x\n", global_addr));
	}
	else
	{
		global_addr = addr;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("==>global_addr3=0x%x\n", global_addr));
	}

	return global_addr;
}


