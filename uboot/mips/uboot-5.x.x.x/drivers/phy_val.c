#include "phy_val.h"

#include <common.h>
#include <command.h>

int   mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data);
int   mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data);

typedef struct {
    unsigned char lch_sig_detect;
    unsigned char lch_rx_linkpulse;
    unsigned char lch_linkup_100;
    unsigned char lch_linkup_10;
    unsigned char lch_linkup_mdi;
    unsigned char lch_linkup_mdix;
    unsigned char lch_descr_lock;
    unsigned char mdix_status;
    unsigned char tx_amp_save;
    unsigned char final_duplex;
    unsigned char final_speed;
    unsigned char final_link;   
} tcphy_l0r28_reg_t;

typedef struct {
    unsigned char lp_eee_10g;
    unsigned char lp_eee_1000;
    unsigned char lp_eee_100;  
} tcphy_l3r18_reg_t;

tcphy_l0r28_reg_t mr28;
tcphy_l3r18_reg_t mrl3_18;


unsigned int 
tcPhyReadReg(unsigned char port_num, unsigned char reg_num)
{
    unsigned int val, val_r31;
    unsigned int phyAddr = port_num;

    if (reg_num<16 || reg_num==31){
        mii_mgr_read(phyAddr, reg_num, &val); 
    }
    else{
        mii_mgr_read(phyAddr, 31, &val_r31); // remember last page
        // set page to L0 if necessary
        if (val_r31 != 0x8000) {
            mii_mgr_write(phyAddr, 31, 0x8000);
        }
        // read reg
        mii_mgr_read(phyAddr, reg_num, &val); 
        // restore page if necessary
        if (val_r31 != 0x8000) {
            mii_mgr_write(phyAddr, 31, val_r31);
        }
    }
        
    return (val);
}

// read Local Reg
unsigned int
tcPhyReadLReg(unsigned char port_num,unsigned char page_num,unsigned char reg_num){
    unsigned int val, val_r31;
    unsigned int phyAddr = port_num;
    unsigned int pageAddr = (page_num<<12)+0x8000;

    mii_mgr_read(phyAddr, 31, &val_r31);  // remember last page
    // set page if necessary
    if (val_r31 != pageAddr) {
        mii_mgr_write(phyAddr, 31, pageAddr); // switch to page Lx
    }
    mii_mgr_read(phyAddr, reg_num, &val); 
    // restore page if necessary
    if (val_r31 != pageAddr) {
        mii_mgr_write(phyAddr, 31, val_r31);
    }
    if (page_num==3) {
		switch(reg_num){
			case 18:
               mrl3_18.lp_eee_10g = (val>>3)&0x0001;
               mrl3_18.lp_eee_1000 = (val>>2)&0x0001;
               mrl3_18.lp_eee_100 = (val>>1)&0x0001;
               break;
			default:
                 break; 
			}
    	}
    return val;
}

// write Local Reg
void
tcPhyWriteLReg(unsigned char port_num,unsigned char page_num,unsigned char reg_num,unsigned int reg_data){
    unsigned int val_r31;
    unsigned int phyAddr = port_num;
    unsigned int pageAddr = (page_num<<12)+0x8000;

    mii_mgr_read(phyAddr, 31, &val_r31);  // remember last page
    // set page if necessary
    if (val_r31 != pageAddr) {
        mii_mgr_write(phyAddr, 31, pageAddr); // switch to page Lx  
    }
    mii_mgr_write(phyAddr, reg_num, reg_data); 
    // restore page if necessary
    if (val_r31 != pageAddr) {
        mii_mgr_write(phyAddr, 31, val_r31);
    }
}

// read Global Reg
unsigned int
tcPhyReadGReg(unsigned char port_num,unsigned char page_num,unsigned char reg_num){
    unsigned int val, val_r31;
    unsigned int phyAddr = port_num;
    unsigned int pageAddr = (page_num<<12);

    mii_mgr_read(phyAddr, 31, &val_r31);  // remember last page
    // set page if necessary
    if (val_r31 != pageAddr) {
        mii_mgr_write(phyAddr, 31, pageAddr); // switch to page Gx  
    }
    mii_mgr_read(phyAddr, reg_num, &val); 
    // restore page if necessary
    if (val_r31 != pageAddr) {
        mii_mgr_write(phyAddr, 31, val_r31);
    }
    
    return val;
}

// write Global Reg
void
tcPhyWriteGReg(unsigned char port_num,unsigned char page_num,unsigned char reg_num,unsigned int reg_data){
    unsigned int val_r31;
    unsigned int phyAddr = port_num;
    unsigned int pageAddr = (page_num<<12);

    mii_mgr_read(phyAddr, 31, &val_r31);  // remember last page
    // set page if necessary
    if (val_r31 != pageAddr) {
        mii_mgr_write(phyAddr, 31, pageAddr); // switch to page Gx
    }
    mii_mgr_write(phyAddr, reg_num, reg_data); 
    // restore page if necessary
    if (val_r31 != pageAddr) {
        mii_mgr_write(phyAddr, 31, val_r31);
    }
}

int
tc2105mlReadErrOverSum(unsigned char port_num)
{
    int err_over_sum,err_over_cnt1,err_over_cnt2;
    
    err_over_cnt1 = (tcPhyReadReg(port_num,25) & 0x0000007ff);
    udelay(300000);
    err_over_cnt2 = (tcPhyReadReg(port_num,25) & 0x0000007ff);
    err_over_sum = err_over_cnt2 - err_over_cnt1;
    if( err_over_sum < 0 ){
        err_over_sum += 2048;
    }

    return err_over_sum;
}

unsigned short
tc2105mlReadSnrSum(unsigned char port_num, unsigned short cnt)
{
    unsigned short snr_sum = 0;
    unsigned short j;
    
    for(j=0;j<cnt;j++) {
      snr_sum += tc2105mlReadProbe(port_num,ProbeSnr);
    }
    return snr_sum;

}

int
tc2105mlReadAdcSum(unsigned char port_num)
{
    int cnt=1000;
    int AdcSign_sum = 0;
    int j;
	unsigned int val_g3r20, val_g3r20_newval, val_l0r30, val_l1r22;

	val_g3r20=tcPhyReadGReg(port_num,3,20);
	val_g3r20_newval = (val_g3r20) & 0x7fff;
	tcPhyWriteGReg(port_num,3,20,val_g3r20_newval);//switch to full AD

	val_l0r30=tcPhyReadLReg(port_num,0,30);
	tcPhyWriteLReg(port_num,0,30,0x1510);//power down buffer
	
	val_l1r22=tcPhyReadLReg(port_num,1,22);
	tcPhyWriteLReg(port_num,1,22,0x000c);//force HP
	
    for(j=0;j<cnt;j++){
        AdcSign_sum += tc2105mlReadProbe(port_num,ProbeAdcSign);
    }
    //shift right to show percent of the dc offset (unit:%)
    AdcSign_sum = (AdcSign_sum>>6);

	tcPhyWriteGReg(port_num,3,20,val_g3r20);
	tcPhyWriteLReg(port_num,0,30,val_l0r30);
	tcPhyWriteLReg(port_num,1,22,val_l1r22);
	
    return AdcSign_sum;
}

int
tc2105mlReadProbe(unsigned char port_num, unsigned char mode)
{
    unsigned int val, val_r31, val_g0r28;
    unsigned int rval, wval;
    unsigned int phyaddr = port_num;
    
    mii_mgr_read(phyaddr, 31, &val_r31); 
    mii_mgr_write( phyaddr, 31, 0x0000 );
    mii_mgr_read(phyaddr, 28, &val_g0r28);
	
    
    switch(mode){
        case ProbeZfgain:
            wval = 0x0b04 + port_num;           
            mii_mgr_write( phyaddr, 28, wval );
            mii_mgr_read(phyaddr, 27, &val);
            rval = (val)&0x3f ;
            break;
        case ProbeAgccode:
            wval = 0x2e04 + port_num;           
            mii_mgr_write( phyaddr, 28, wval );
            mii_mgr_read(phyaddr, 27, &val);
            rval = (val>>1)&0x1f ;
            break;
        case ProbeBoosten:
            wval = 0x2e04 + port_num;           
            mii_mgr_write( phyaddr, 28, wval );
            mii_mgr_read(phyaddr, 27, &val);
            rval = (val>>6)&0x01 ;
            break;
        break;
        case ProbeSnr:
            wval = 0x0904 + port_num;           
            mii_mgr_write( phyaddr, 28, wval );
            mii_mgr_read(phyaddr, 27, &val);
            rval = (val)&0xff ;
            break;  
        case ProbeAdcSign:
            wval = 0x4104 + port_num;           
            mii_mgr_write( phyaddr, 28, wval );
            mii_mgr_read(phyaddr, 27, &val);
            rval = (val)&0x7f ;
            if (rval>64){
                rval -= 128;
            }
            break;
        default:
            printf("\r\ntcphy error: ReadProbe %d.\r\n",mode);
            rval = 0;
            break;
    }

    mii_mgr_write( phyaddr, 31, 0x0000 );
    mii_mgr_write( phyaddr, 28, val_g0r28); // restore G0Reg28
    mii_mgr_write( phyaddr, 31, val_r31); // restore Reg31
    return rval;

}

void
tc2105mlDispProbe100(unsigned char port_num)
{

    const unsigned short tc2105mlReadSnrCnt = 1000;

    printf("tcphy[%d]:",port_num);
    printf(" boosten=%ld", tc2105mlReadProbe(port_num,ProbeBoosten));
    printf(" agccode=%ld", tc2105mlReadProbe(port_num,ProbeAgccode));
    printf(" zfgain=%ld", tc2105mlReadProbe(port_num,ProbeZfgain));
    printf(" ch_idx=%ld", (tcPhyReadReg(port_num,29)&0x003f));
    printf(" snr=%ld", tc2105mlReadProbe(port_num,ProbeSnr));
      
    printf(" err_over_sum=%ld", tc2105mlReadErrOverSum(port_num));

    printf(" snr_sum(x1000)=%d",tc2105mlReadSnrSum(port_num,tc2105mlReadSnrCnt)); // snr_sum   
    printf(" adc_avg=%ld/1000", tc2105mlReadAdcSum(port_num));  
    printf(" \r\n");

}

int phy_chk(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    unsigned int val;
    unsigned int mr02,mr03;
    unsigned char mr_anen, mr_dplx, mr_speed;
    unsigned char mr_an_capable;
    unsigned char mr_lp_an_capable;
    unsigned char mr_lp_anen;
    //unsigned char lr_force_mdix;
    unsigned char lr_linkup, lr_speed, lr_dplx, lr_mdix;
    unsigned int port_num; 

    if (argc < 2) {
	    printf ("Usage:\n%s\n", cmdtp->usage);
	    return 1;
    }
 
    port_num= simple_strtol(argv[1], NULL, 10);
    printf("port_num = %d\n", port_num);

    // show phy_id
    val = tcPhyReadReg(port_num, 2); 
    mr02 = val&(0xffff);
    val = tcPhyReadReg(port_num, 3); 
    mr03 = val&(0xffff);
    printf(" TcPhy ID: %lx %lx\r\n",mr02,mr03); 

    if (mr02 != 0x03a2)
        return 1;
 
    // reg0
    val = tcPhyReadReg(port_num, 0);
    mr_anen = (val>>12)&0x01;
    mr_dplx = (val>>8)&0x01;
    mr_speed = (val>>13)&0x01;
    // reg4
    val = tcPhyReadReg(port_num, 4);
    mr_an_capable = (val>>5)&0x0F; //100F,100H,10F,10H
    // reg5
    val = tcPhyReadReg(port_num, 5);
    mr_lp_an_capable = (val>>5)&0x0F; //100F,100H,10F,10H
    // reg6
    val = tcPhyReadReg(port_num, 6);
    mr_lp_anen = (val)&0x01;
    // l0reg28
    val = tcPhyReadReg(port_num, 28);
    lr_linkup = (val)&0x01;
    lr_speed = (val>>1)&0x01;
    lr_dplx = (val>>2)&0x01;
    lr_mdix = (val>>5)&0x01;
   
    mr28.lch_sig_detect  = (val>>15)&0x0001;
    mr28.lch_rx_linkpulse= (val>>14)&0x0001;
    mr28.lch_linkup_100  = (val>>13)&0x0001;
    mr28.lch_linkup_10   = (val>>12)&0x0001;
    mr28.lch_linkup_mdi  = (val>>11)&0x0001; // after LEM
    mr28.lch_linkup_mdix = (val>>10)&0x0001; // after LEM
    mr28.lch_descr_lock  = (val>>9)&0x0001; // after LEM
    mr28.mdix_status  = (val>>5)&0x0001; /* {0:mdi,1:mdix} */   
    mr28.tx_amp_save  = (val>>3)&0x0003; /* 0:100%, 1:90%, 2:80%, 3:70% */
    mr28.final_duplex = (val>>2)&0x0001; /* {0:half-duplex, 1:full-duplex} */
    mr28.final_speed  = (val>>1)&0x0001; /* {0:10, 1:100} */
    mr28.final_link   = (val)&0x0001; /* {0:linkdown, 1:linkup} */  
 
    printf(" TcPhy mode:");
    if(mr_anen) { // Auto-neg
        printf(" AN-(");
        printf(" %s,",((mr_an_capable>>3)&0x01)?"100F":"");
        printf(" %s,",((mr_an_capable>>2)&0x01)?"100H":"");     
        printf(" %s,",((mr_an_capable>>1)&0x01)?"10F":"");      
        printf(" %s)\r\n",((mr_an_capable>>0)&0x01)?"10H":"");  
    }
    else { // Force-speed
        printf(" Force-%d%s\r\n",
                (mr_speed?100:10),(mr_dplx?"F":"H"));
    }
 
     if(!lr_linkup) { // link-down
        printf(" *** Link is down!\r\n");
        printf(" tcphy[%ld]: adc_avg=%ld/1000\r\n", 
                   port_num, tc2105mlReadAdcSum(port_num));            


         // L0R28 message
        if (mr28.lch_sig_detect || mr28.lch_rx_linkpulse
            || mr28.lch_linkup_100 || mr28.lch_linkup_10)
        {
            printf(" tcphy[%ld]: ",port_num);
            if (mr28.lch_sig_detect) 
                printf("SigDet ");
            if (mr28.lch_rx_linkpulse)
                printf("RxLkp ");
            if (mr28.lch_linkup_100)
                printf("Up100 ");
            if (mr28.lch_linkup_10)
                printf("Up10 ");
            if (mr28.lch_linkup_mdi)
                printf("UpMdi ");
            if (mr28.lch_linkup_mdix)
                printf("UpMdix ");
            printf("\r\n");
        }
    }
    else { // link-up
        printf(" TCphy is link-up at %d%s.\r\n",
                (lr_speed?100:10),(lr_dplx?"F":"H"));
        
        if(mr_lp_anen) {
            printf(" Link-partner supports AN-(");
            printf(" %s,",((mr_lp_an_capable>>3)&0x01)?"100F":"");
            printf(" %s,",((mr_lp_an_capable>>2)&0x01)?"100H":"");  
            printf(" %s,",((mr_lp_an_capable>>1)&0x01)?"10F":"");       
            printf(" %s)\r\n",((mr_lp_an_capable>>0)&0x01)?"10H":"");   
        }
        else {
            printf(" Link-partner operates in Force mode.\r\n");
        }
 
        printf(" %s,",(lr_mdix?"mdix":"mdi"));
        printf(" tx_amp_save=%d\r\n",(mr28.tx_amp_save));
        printf("\r\n");
        
        if(lr_speed) { // 100Mbps
            tc2105mlDispProbe100(port_num);
        }
        printf("\r\n");
   
    }

    return 0;
}


U_BOOT_CMD(
	phyval, 2, 1, phy_chk,
	"phyval  - ethernet phy value\n",
	"phyval usage:\n"
	" phyval <port_no>"
);
