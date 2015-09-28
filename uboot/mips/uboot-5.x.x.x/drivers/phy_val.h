#ifndef USB_PHY_VAL_H
#define USB_PHY_VAL_H

struct _lp0r28
{
	unsigned short final_link:1;
	unsigned short final_speed:1;
	unsigned short final_duplex:1;
	unsigned short tx_amp_save:2;
	unsigned short mdix_status:1;
	unsigned short mr_autoneg_complete:1;
	unsigned short reserved:1;
	unsigned short polarity_neg_pcs:1;
	unsigned short lch_descr_lock:1;
	unsigned short lch_linkup_mdix:1;
	unsigned short lch_linkup_mdi:1;
	unsigned short lch_linkup_10:1;
	unsigned short lch_linkup_100:1;
	unsigned short lch_rx_linkpulse:1;
	unsigned short lch_sig_detect:1;
};

#define ProbeZfgain  0
#define ProbeAgccode 1
#define ProbeBoosten 2
#define ProbeSnr     3
#define ProbeDcoff   4
#define ProbeAdcoff  5
#define ProbeAdcSign 6

unsigned int tcPhyReadReg(unsigned char port_num,unsigned char reg_num);
unsigned int tcPhyReadLReg(unsigned char port_num,unsigned char page_num,unsigned char reg_num);
void tcPhyWriteLReg(unsigned char port_num,unsigned char page_num,unsigned char reg_num,unsigned int reg_data);
unsigned int tcPhyReadGReg(unsigned char port_num,unsigned char page_num,unsigned char reg_num);
void tcPhyWriteGReg(unsigned char port_num,unsigned char page_num,unsigned char reg_num,unsigned int reg_data);
int tc2105mlReadAdcSum(unsigned char port_num);
int tc2105mlReadProbe(unsigned char port_num, unsigned char mode);
void tc2105mlDispProbe100(unsigned char port_num);

#endif
