/*
Copyright (c) 2008 Silicon Laboratories, Inc.
2008-03-26 19:05:19 */
/*ProSLIC API Tool Rev0.31 Beta*/


#include "proslic.h"
#include "si324x.h"
Si324x_General_Cfg Si324x_General_Configuration  = {
0x8F5C24AL, /*vbatr_expect - CUSTOM = 140.00, nom = 95.000*/
0x7FD710CL, /*coeff_p_hvic = 1.000*/
0x2468ADL, /*p_th_hvic = 1000.000*/
0x353F7B5L, /*vbath_expect - CUSTOM = 52.00 , nom = 75.000*/
0x0, /*cm_clamp = 0.000*/
0x3F, /*auto = 0.000*/
0x640000L, /*cm_dbi = 100.000*/
0x3C0000L, /*bat_dbi = 60.000*/
0x20C49AL, /*bat_hyst = 2.000*/
0x10000L, /*bat_settle = 1.000*/
0x42, /*batselmap - CUSTOM - nom 0x46 */
0x7FD710CL, /*coef_p_offld = 1.000*/
#ifdef NO_VBATL
0x6, /*offld */
#else
0x7, /*offld*/
#endif
0x48D15BL, /*p_th_offld = 2000.000*/
0x199998DL, /*vbatl_expect = 25.000*/
0xff,     /* irqen1 */
0xff,	  /* irqen2 */
0xff,     /* irqen3 */
0x00      /* irqen4 */
};

Si324x_DTMFDec_Cfg Si324x_DTMFDec_Presets[] = {
	{0x2d40000L,0x1a660000L,0x2d40000L,0x6ba0000L,0x1dcc0000L,0x33f0000L,0xbd30000L,0x19d20000L,0x4150000L,0x188F0000L,0x4150000L,0xd970000L,0x18620000L,0xf1c0000L}
};
Si324x_GPIO_Cfg Si324x_GPIO_Configuration = {
	0x0, 0x0, 0x0,0x0,0x0,0x0
};
Si324x_CI_Cfg Si324x_CI_Presets [] = {
	{0}
};
Si324x_audioGain_Cfg Si324x_audioGain_Presets [] = {
	{0x1377080L,0},
	{0x80C3180L,0}
};

Si324x_Ring_Cfg Si324x_Ring_Presets[] ={


    /* inputs:  ringtrip ac thresh = 450.000, dc_thresh = 10.000, rt debounce = 0.075, vov_ring_gnd = 7.000, vov_ring_bat = 10.000*/
    { 0x7EFE000L, 0x1BA000L, 0x1EB3154L, 0x0L, 0x50000L, 0x80, 0x3E, 0x0, 0x7D, 0x58, 0x800000L, 0xC84B5BL, 0xF5E5200EL, 0x0L, 0x0L, 0x51EB8L, 0x6000L, 0x32EDF9EL, 0x6000L, 0x1C71ECL, 0xA3D70AL, 0x72B020L, 0x4CCCCCCL }
};

Si324x_DCfeed_Cfg Si324x_DCfeed_Presets[] = {

    /* inputs: v_vlim=48.000, i_vlim=0.000, v_rfeed=42.800, i_rfeed=10.000,v_ilim=33.200, i_ilim=20.000,
 vcm_oh=27.000, vov_gnd=4.000, vov_bat=4.000, r_ring=320.000
 lcronhk=10.000, lcroffhk=12.000, lcrdbi=5.000, longhith=8.000, longloth=7.000, longdbi=5.000
 lcrmask=80.000, lcrmask_polrev=80.000, lcrmask_state=200.000, lcrmask_linecap=200.000 */
    { 0x1DDF41C9L, 0x1EF68D5EL, 0x40A0E0L, 0x18AAD168L, 0x1CA39FFAL, 0x5A38633L, 0x5072476L, 0x3E67006L, 0xFDFAB5L, 0x5D0FA6L, 0x2D8D96L, 0x5B0AFBL, 0x6D4060L, 0x8000L, 0x48D595L, 0x3FBAE2L, 0x8000L, 0x80000L, 0x80000L, 0x140000L, 0x140000L, 0x1BA5E35L, 0x418937L, 0x418937L  }
};

Si324x_Impedance_Cfg Si324x_Impedance_Presets[] = {   /* Default - 600ohms, 30ohm RPROT */
{
  {0x08044B00L,0x1FF9F000L,0x00052600L,0x1FFE2280L, /* TXACEQ */
   0x07FECB00L,0x1FF9AC80L,0x00029900L,0x1FFB1100L}, /* RXACEQ */
  {0x00209A80L,0x1FB0B880L,0x02090000L,0x001CE380L,  /* ECFIR/ECIIR */
   0x0238AF80L,0x1F2E0E00L,0x005CC480L,0x1FED2680L,
   0x1FF6DB00L,0x00035580L,0x0256D200L,0x05866B80L},  
  {0x00207500L,0x1FD16580L,0x000E5C00L,0x0A138100L, /* ZSYNTH */
   0x1DEA2280L,0x6E},  
  {0x08769F00L,0x00}, /* TXACGAIN/MUTE */
  {0x01454B00L,0x00}, /* RXACGAIN/MUTE */ 
   0x07B3DA80L,0x184C2600L,0x0767B500L}  /* RXHPF */
};


Si324x_FSK_Cfg Si324x_FSK_Presets[] ={

    /* inputs: mark freq=1200.000, space freq2200.000, amp=0.220, baud=1200.000, startStopDis=0, interrupt depth = 0 */
    { 0x2232000L, 0x77C2000L, 0x3C0000L, 0x200000L, 0x6B60000L, 0x79C0000L,0, 0 }
};

Si324x_PulseMeter_Cfg Si324x_PulseMeter_Presets[] ={

    /* inputs:  freq = 12kHz, amp = 1.000Vrms, cal = First, ramp = 24kHz, power = Normal */
    { 0x7A2B6AL, 0x0, 0x0 }
};

Si324x_Tone_Cfg Si324x_Tone_Presets[] ={

    /* inputs:  freq1 = 350.000, amp1 = -18.000, freq2 = 440.000, amp2 = -18.000, ta1 = 0.000, ti1 = 0.000, ta2 = 0.000, ti2 = 0.000*/
    { {0x7B30000L, 0x3A000L, 0x0L, 0x0, 0x0, 0x0, 0x0}, {0x7870000L, 0x4A000L, 0x0L, 0x0, 0x0, 0x0, 0x0}, 0x66 },
    /* inputs:  freq1 = 480.000, amp1 = -18.000, freq2 = 620.000, amp2 = -18.000, ta1 = 0.500, ti1 = 0.500, ta2 = 0.500, ti2 = 0.500*/
    { {0x7700000L, 0x52000L, 0x0L, 0xA0, 0xF, 0xA0, 0xF}, {0x7120000L, 0x6A000L, 0x0L, 0xA0, 0xF, 0xA0, 0xF}, 0x66 },
    /* inputs:  freq1 = 480.000, amp1 = -18.000, freq2 = 440.000, amp2 = -18.000, ta1 = 2.000, ti1 = 4.000, ta2 = 2.000, ti2 = 4.000*/
    { {0x7700000L, 0x52000L, 0x0L, 0x80, 0x3E, 0x0, 0x7D}, {0x7870000L, 0x4A000L, 0x0L, 0x80, 0x3E, 0x0, 0x7D}, 0x66 },
    /* inputs:  freq1 = 480.000, amp1 = -18.000, freq2 = 620.000, amp2 = -18.000, ta1 = 0.300, ti1 = 0.200, ta2 = 0.300, ti2 = 0.200*/
    { {0x7700000L, 0x52000L, 0x0L, 0x60, 0x9, 0x40, 0x6}, {0x7120000L, 0x6A000L, 0x0L, 0x60, 0x9, 0x40, 0x6}, 0x66 },
    /* inputs:  freq1 = 480.000, amp1 = -18.000, freq2 = 620.000, amp2 = -18.000, ta1 = 0.200, ti1 = 0.200, ta2 = 0.200, ti2 = 0.200*/
    { {0x7700000L, 0x52000L, 0x0L, 0x40, 0x6, 0x40, 0x6}, {0x7120000L, 0x6A000L, 0x0L, 0x40, 0x6, 0x40, 0x6}, 0x66 }
};

Si324x_PCM_Cfg Si324x_PCM_Presets[] ={

    /* inputs:  u-law narrowband positive edge, dtx positive edge, both disabled, tx hwy = A, rx hwy = A */
    { 0x1, 0x0, 0x0, 0x0 }
};

