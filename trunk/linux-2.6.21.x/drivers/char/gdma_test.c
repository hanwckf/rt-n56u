/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************
 *
 */
#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/pci.h>
#include <asm/uaccess.h>

#include <linux/ralink_gdma.h>

#define PATTERN_LEN      1024
#define PATTERN_BOUNDARY 1052 //for unalignment memcpy test
#define SHOW_INTERVAL	 1000

/*
 * Notes: 
 * If you want to test GDMA, please use "DEBUG mode in channel allocation mode"
 * to use different channel for each run. 
 */

/* 
 * CONCURRENT: All channels run at the same time (forever) 
 * CHAIN: * periodical use channel 0~7->0~7....0~7 
 *	    please modify GdmaMem2Mem()
 *	    {....  
 *		Entry.ChMask=1;
 *		...
 *	    } 
 *
 *	    Entry.NextUnMaskCh= (Entry.ChNum+1) % MAX_GDMA_CHANNEL;
 *
 * POLL: GDMA polling mode, start another gdma when current gdma done.
 * FULL_TEST: Execute all test combination (48 pairs) using polling
 *
 */
//#define CONCURRENT	    1
//#define CHAIN		    1
#define POLL		    1
//#define GDMA_FULL_TEST    1


unsigned char *Src;
unsigned char *Dst[MAX_GDMA_CHANNEL];

dma_addr_t Src_phy;
dma_addr_t Dst_phy[MAX_GDMA_CHANNEL];

unsigned char DoneBit[MAX_GDMA_CHANNEL];

void check_result( uint8_t *src, uint8_t *dst, uint32_t len)
{
	printk("src=%p dst=%p len=%x\n", src, dst, len);

	if(memcmp(src, dst, len)==0){
		printk("check .. ok\n");
	}else{
		printk("check .. fail\n");
	}
}

void Done(uint32_t Ch)
{
	int i=0;
	static int count[MAX_GDMA_CHANNEL];
	
	if((++count[Ch] % SHOW_INTERVAL)==0) {
	    printk("Ch=%d is still alive\n", Ch);
	}

	for(i=0;i<PATTERN_LEN;i++) {
		if(Dst[Ch][i]!=Src[i]) {
			printk("***********<<WARNNING!!!>>*********\n");
			printk("Ch=%d Check fail (Dst=%x Src=%x)\n",Ch, Dst[Ch][i], Src[i]);
			printk("***********************************\n");
			return;
		}
	}
	
	DoneBit[Ch]=1;

#if defined (CHAIN)
	GdmaMem2Mem(Src_phy, Dst_phy[Ch], sizeof(Src), Done);

	if(Ch==7) {
		GdmaUnMaskChannel(0);
	}
#elif defined (CONCURRENT)
	GdmaMem2Mem(Src_phy, Dst_phy[Ch], sizeof(Src), Done);
#endif


}

int poll_mode(void *unused) 
{

	while(1) {
	    GdmaMem2Mem(Src_phy, Dst_phy[0], PATTERN_LEN, NULL);
	    Done(0);
	    GdmaMem2Mem(Src_phy, Dst_phy[1], PATTERN_LEN, NULL);
	    Done(1);
	    GdmaMem2Mem(Src_phy, Dst_phy[2], PATTERN_LEN, NULL);
	    Done(2);
	    GdmaMem2Mem(Src_phy, Dst_phy[3], PATTERN_LEN, NULL);
	    Done(3);
	    GdmaMem2Mem(Src_phy, Dst_phy[4], PATTERN_LEN, NULL);
	    Done(4);
	    GdmaMem2Mem(Src_phy, Dst_phy[5], PATTERN_LEN, NULL);
	    Done(5);
	    GdmaMem2Mem(Src_phy, Dst_phy[6], PATTERN_LEN, NULL);
	    Done(6);
	    GdmaMem2Mem(Src_phy, Dst_phy[7], PATTERN_LEN, NULL);
	    Done(7);
	}

	return 1;
}

int concurrent_mode(void *unused) 
{

	GdmaMem2Mem(Src_phy, Dst_phy[0], PATTERN_LEN, Done);
	GdmaMem2Mem(Src_phy, Dst_phy[1], PATTERN_LEN, Done);
	GdmaMem2Mem(Src_phy, Dst_phy[2], PATTERN_LEN, Done);
	GdmaMem2Mem(Src_phy, Dst_phy[3], PATTERN_LEN, Done);
	GdmaMem2Mem(Src_phy, Dst_phy[4], PATTERN_LEN, Done);
	GdmaMem2Mem(Src_phy, Dst_phy[5], PATTERN_LEN, Done);
	GdmaMem2Mem(Src_phy, Dst_phy[6], PATTERN_LEN, Done);
	GdmaMem2Mem(Src_phy, Dst_phy[7], PATTERN_LEN, Done);
	
	return 1;
}

int full_test_mode(void *unused) 
{
	int i,j;


	for(i=0;i<4;i++) {
	    for(j=0;j<4;j++) {

		printk("\n======src base=%d to dst base=%d=====\n",i,j);
		GdmaMem2Mem(Src_phy+i, Dst_phy[0]+j, PATTERN_LEN, NULL);
		check_result(Src+i, &Dst[0][j], PATTERN_LEN);

		GdmaMem2Mem(Src_phy+i, Dst_phy[0]+j, PATTERN_LEN+1, NULL);
		check_result(Src+i, &Dst[0][j], PATTERN_LEN+1);

		GdmaMem2Mem(Src_phy+i, Dst_phy[0]+j, PATTERN_LEN+2, NULL);
		check_result(Src+i, &Dst[0][j], PATTERN_LEN+2);

		GdmaMem2Mem(Src_phy+i, Dst_phy[0]+j, PATTERN_LEN+3, NULL);
		check_result(Src+i, &Dst[0][j], PATTERN_LEN+3);

	    }
	}
	
	return 1;
}

int chain_mode(void *unused) {

	GdmaMem2Mem(Src_phy, Dst_phy[0], PATTERN_LEN, Done);
	GdmaMem2Mem(Src_phy, Dst_phy[1], PATTERN_LEN, Done);
	GdmaMem2Mem(Src_phy, Dst_phy[2], PATTERN_LEN, Done);
	GdmaMem2Mem(Src_phy, Dst_phy[3], PATTERN_LEN, Done);
	GdmaMem2Mem(Src_phy, Dst_phy[4], PATTERN_LEN, Done);
	GdmaMem2Mem(Src_phy, Dst_phy[5], PATTERN_LEN, Done);
	GdmaMem2Mem(Src_phy, Dst_phy[6], PATTERN_LEN, Done);
	GdmaMem2Mem(Src_phy, Dst_phy[7], PATTERN_LEN, Done);
	GdmaUnMaskChannel(0); //kick of channel 0
	
	return 1;
}

static int RalinkGdmaTestInit(void)
{
	int i=0;

	printk("Enable Ralink GDMA Test Module\n");

	//alloc phy memory for src
	Src = pci_alloc_consistent(NULL, PATTERN_BOUNDARY , &Src_phy);

	if(Src==NULL) {
	    printk("pci_alloc_consistent fail: Src==NULL\n");
	    return 1;
	}

	//fill out content
	for(i=0;i< PATTERN_BOUNDARY;i++) {
	    Src[i]=i;
	}

	//alloc phy memory for dst
	for(i=0;i< MAX_GDMA_CHANNEL; i++) {
	    Dst[i] = pci_alloc_consistent(NULL, PATTERN_BOUNDARY, &Dst_phy[i]);

	    if(Dst[i]==NULL) {
		printk("pci_alloc_consistent fail: Dst[%d]==NULL\n", i);
		return 1;
	    }
	}


#if defined (CONCURRENT)
	printk("All channels run at the same time \n");
	kernel_thread(concurrent_mode, NULL, CLONE_VM);
#elif defined (GDMA_FULL_TEST)
	printk("Execute all test combination (48 pairs) \n");
	kernel_thread(full_test_mode, NULL, CLONE_VM);
#elif defined (POLL)
	printk("GDMA polling mode\n");
	kernel_thread(poll_mode, NULL, CLONE_VM);
#elif defined (CHAIN)
        printk("GDMA chain mode (Channel 0~7...->0~7 periodically\n");
	kernel_thread(chain_mode, NULL, CLONE_VM);
#endif
	
	return 0;
}

static void __exit RalinkGdmaTestExit(void)
{

	int i=0;

	printk("Disable Ralink GDMA Test Module\n");

	pci_free_consistent(NULL, PATTERN_LEN, Src, Src_phy);

	for(i=0;i<MAX_GDMA_CHANNEL;i++) {
		pci_free_consistent(NULL, PATTERN_LEN, Dst[i], Dst_phy[i]);
	}

}


module_init(RalinkGdmaTestInit);
module_exit(RalinkGdmaTestExit);

MODULE_DESCRIPTION("Ralink SoC DMA Test Module");
MODULE_AUTHOR("Steven Liu <steven_liu@ralinktech.com.tw>");
MODULE_LICENSE("GPL");
