#ifndef __SLIC_H_
#define __SLIC_H_
//initilization function
unsigned char getChipID(void);
int dualProSLIC_channelInit(void);

void initilaize_ringing(void);
void enable_PulseMetering(void);


//switch states
void activateRinging(void);
void goActive(void);
void goOpen(void);

//Using the ProSLIC
void genTone(tone_struct *tone);
void cadenceRingPhone(ringStruct ringRegs);
unsigned char getLoopStatus(void);
unsigned char getDTMFdigit(void);
void disableOscillators(void);

//PCM connections
void breakConnection( unsigned char c1, unsigned char c2);
void makeConnection( unsigned char c1, unsigned char c2);


//Gr-909 and Line testing

unsigned char checkForGroundShorts (void);

// slic data
extern unsigned short initSramValues[];
extern unsigned short initextSRAMValues[][2];
extern unsigned char direct_init_table[][2];
#endif /* __SLIC_H_ */

