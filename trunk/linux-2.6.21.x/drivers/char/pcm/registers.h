#ifndef _REGISTERS_H_
#define _REGISTERS_H_

enum REGSPACE{
ID, //0x0x01          //         0    id             x0x01           part_num[2:0]           revision[3:0]
RESET, //x0           //         1    soft_reset     x0                                                      ch 1    ch 0
MSTREN,               //         2    irq_init_en    x0      pll_faulfs_faultpclk_fault_en
MSTRSTAT,             //         3    init_stat      x0      pll_faulfs_faultpclk_fausram_clkpll_lockfs_detecfs_validpclk_valid
RAMSTAT,              //         4    ind_stat       x0      ind_add[9:8]                                            ind_stat
RLYCON,               //         5    relay_cntl     xA3     rrd_invebatsel_pbatsel  ring_raird_oe   gpo/rrd trd2    trd1
LINEFEED,             //         6    linefeed       x0              linefeed_shadow[2:0]            linefeed[2:0]
POLREV,               //         7    pol_rev        x0                                      pol_rev voc_zeropol_rev_ramp_rate
SBIAS,                //         8    bjtbias        xE1     slic_stdsquelch cap_bypaen_bjtbibjtbias_oht[1:0]bjtbias_act[1:0]
LCRRTP,               //         9    lcr_rtp        x40     clamp_ofdac_highcm_high speedup voc_complong_hi rtp     lcr
ILIM, //10            //         10   ilim           x5                              ilim[4:0]
CALR1,                //         11   cal_r1         x3F     cal     cal_spducal_offRcal_offTcal_offRcal_offTcal_dingcal_cing
CALR2,                //         12   cal_r2         x3F                     cal_stbycal_stbycal_madccal_dac_cal_adc_cal_cm_bal
DIAG,                 //         13   diag           x0      iq2_highiq1_hightest_rintx_filt_slic_diaslic_diag_input[2:0]
IRQ0,                 //         14   irq_vec0       x0      spi_irq irq_vec3irq_vec2irq_vec1_1      irq_vec3irq_vec2irq_vec1_0
IRQ1,                 //         15   irq_vec1       x0      pulse_t2pulse_t1ring_t2 ring_t1 osc2_t2 osc2_t1 osc1_t2 osc1_t1
IRQ2,                 //         16   irq_vec2       x0      rxmdm   txmdm   indirectdtmf    voc_traclong_staloop_staring_trip
IRQ3,                 //         17   irq_vec3       x0      cm_bal  dsp     pq6     pq5     pq4     pq3     pq2     pq1
IRQEN1,               //         18   irq_en1        x0      pulse_t2pulse_t1ring_t2 ring_t1 osc2_t2 osc2_t1 osc1_t2 osc1_t1
IRQEN2,               //         19   irq_en2        x0      rxmdm   txmdm   indirectdtmf    voc_traclong_staloop_staring_trip
IRQEN3, //20          //         20   irq_en3        x0      cm_bal  dsp     pq6     pq5     pq4     pq3     pq2     pq1
AUDGAIN,              //         21   aud_gain       x0      cmtx_selatx_mute        atx             arx_mutearx
DIGCON,               //         22   dig_cntl       x0      codec_lppcm_lpbkhyb_lpbkhyb_dis tx_hpf_drx_hpf_ddtx_mutedrx_mute
RINGCON,              //         23   ring_cntl      x0      en_sync ring_dacring_unbt1_en   t2_en   ring_en unb_rev_trap
RINGTALO,             //         24   ring_t1_lo     x0      ring_t1[7:0]
RINGTAHI,             //         25   ring_t1_hi     x0      ring_t1[15:8]
RINGTILO,             //         26   ring_t2_lo     x0      ring_t2[7:0]
RINGTIHI,             //         27   ring_t2_hi     x0      ring_t2[15:8]
PMCON,                //         28   pulse_cntl     x0      en_sync                 t1_en   t2_en   pulse_en
PMTALO,               //         29   pulse_t1_lo    x0      pulse_t1[7:0]
PMTAHI, //30          //         30   pulse_t1_hi    x0      pulse_t1[15:8]
PMTILO,               //         31   pulse_t2_lo    x0      pulse_t2[7:0]
PMTIHI,               //         32   pulse_t2_hi    x0      pulse_t2[15:8]
ZRS,                  //         33   zsyn_rs        x0                                      rs[3:0]
ZZ,                   //         34   zsyn_z         x0      zsynth_dzsynth_ozp[1:0]                         zz[1:0]
ZB0LO,                //         35   zsyn_b0_lo     x0      coeff_b0[7:0]
ZB0MID,               //         36   zsyn_b0_mid    x0      coeff_b0[15:8]
ZB0HI,                //         37   zsyn_b0_hi     x0      coeff_b0[23:16]
ZB1LO,                //         38   zsyn_b1_lo     x0      coeff_b1[7:0]
ZB1MID,               //         39   zsyn_b1_mid    x0      coeff_b1[15:8]
ZB1HI, //40           //         40   zsyn_b1_hi     x0      coeff_b1[23:16]
ZB2LO,                //         41   zsyn_b2_lo     x0      coeff_b2[7:0]
ZB2MID,               //         42   zsyn_b2_mid    x0      coeff_b2[15:8]
ZB2HI,                //         43   zsyn_b2_hi     x0      coeff_b2[23:16]
ZB3LO,                //         44   zsyn_b3_lo     x0      coeff_b3[7:0]
ZB3MID,               //         45   zsyn_b3_mid    x0      coeff_b3[15:8]
ZB3HI,                //         46   zsyn_b3_hi     x0      coeff_b3[23:16]
ZA1LO,                //         47   zsyn_a1_lo     x0      coeff_a1[7:0]
ZA1MID,               //         48   zsyn_a1_mid    x0      coeff_a1[15:8]
ZA1HI,                //         49   zsyn_a1_hi     x0                              coeff_a1[20:16]
ZA2LO, //50           //         50   zsyn_a2_lo     x0      coeff_a2[7:0]
ZA2MID,               //         51   zsyn_a2_mid    x0      coeff_a2[15:8]
ZA2HI,                //         52   zsyn_a2_hi     x0                              coeff_a2[20:16]
PCMMODE,              //         53   pcm_mode       x5      gci_linepclk_2x pcm_tri pcm_en  alaw[1:0]       pcm_fmt[1:0]
TXSTLO,               //         54   tx_start_lo    x0      tx_start[7:0]
TXSTHI,               //         55   tx_start_hi    x0                                                      tx_start[9:8]
RXSTLO,               //         56   rx_start_lo    x0      rx_start[7:0]
RXSTHI,               //         57   rx_start_hi    x0                                                      rx_start[9:8]
OMODE,                //         58   osc_mode       x0      o1_fsk8 zero_en_routing_2[1:0]  osc1_fskzero_en_routing_1[1:0]
OCON,                 //         59   osc_cntl       x0      en_sync_osc2_t1_osc2_t2_osc2_en en_sync_osc1_t1_osc1_t2_en
O1TALO, //60          //         60   osc1_t1_lo     x0      osc1_t1[7:0]
O1TAHI,               //         61   osc1_t1_hi     x0      osc1_t1[15:8]
O1TILO,               //         62   osc1_t2_lo     x0      osc1_t2[7:0]
O1TIHI,               //         63   osc1_t2_hi     x0      osc1_t2[15:8]                                           osc1_en
O2TALO,               //         64   osc2_t1_lo     x0      osc2_t1[7:0]
O2TAHI,               //         65   osc2_t1_hi     x0      osc2_t1[15:8]
O2TILO,               //         66   osc2_t2_lo     x0      osc2_t2[7:0]
O2TIHI,               //         67   osc2_t2_hi     x0      osc2_t2[15:8]
FSKDAT,               //         68   fsk_data       x0      fsk_byte[7:0]
DTMF,                 //         69   dtmf           x0                      valid   valid_todtmf_digit[3:0]
TONEDET, //70         //         70   modem_cntl     x0      fail_cnt[3:0]                   pass_cnt[3:0]
TONDEN,                //         71   modem_dis      x0                                              dtmf_disrxmdm_dis
THERM, //72         //         72   i2_active      x45     stat    thrm_selthresh[1:0]     i2_active[3:0]
R73,//RESERVED        //
R74,                  //
R75,                  //
R76,                  //
R77,                  //
R78,                  //
R79,                  //
R80,                  //
R81,                  //
R82,                  //
R83,                  //
R84,                  //
R85,                  //
R86,                  //
TESTCON, //87         //         87   test_cntl      x0                                      test_scaunlock  pclk_fault
AUTOCON,                 //         88   auto           xFF     ilim_maxautostdbenspeed autotrk autobat autord  autold
LOOPBACK,             //         89   loopback       x0      hvic                                    ana_lpbkdig_lpbkbit0
PCLKMUL,              //         90   pclk_mult      xe                      fab_id[1:0]     pclk_mult[3:0]
VTIPREG,                 //         91   vtip           x0      vtip[7:0]                                               ana_lpbk
VRINGREG,                //         92   vring          x0      vring[7:0]
VBATREG,                 //         93   vbat           x0      vbat[7:0]
VRINGX,               //         94   vring_ext      x0      vring_ext[7:0]
IRINGX,               //         95   iring_ext      x0      iring_ext[7:0]
IQ1,                  //         96   iq1            x0      iq1[7:0]
IQ2,                  //         97   iq2            x0      iq2[7:0]
IQ3, //100            //         98   iq3            x0      iq3[7:0]
IQ4,                  //         99   iq4            x0      iq4[7:0]
IDDAT16,              //        100   ind_data_16    x0      ind_data_16[7:0]
RAMLO,              //        101   ind_data_lo    x0      ind_data_lo[7:0]
RAMHI,              //        102   ind_data_hi    x0      ind_data_hi[7:0]
RAMADDR,                 //        103   ind_addr       x0      ind_addr[7:0]
CALOFR,               //        104   cal_offsetR    x10                             cal_offsetR[4:0]
CALOFT,               //        105   cal_offsetT    x10                             cal_offsetT[4:0]
CALOFRN,              //        106   cal_offsetRN   x10                             cal_offsetRN[4:0]
CALOFTN,              //        107   cal_offsetTN   x10                             cal_offsetTN[4:0]
CALDGAIN, //110       //        108   cal_din_gain   x11                             cal_din_gain[4:0]
CALCGAIN,             //        109   cal_cin_gain   x11                             cal_cin_gain[4:0]
CALDACA,              //        110   cal_offset_stbyx1c                     cal_offset_stbyR[5:0]
CALDACD,              //        111   cal_offset_stbyx1c                     cal_offset_stbyT[5:0]
CALCMBAL,             //        112   cal_dac_o_a    x0                                      dac_os_pdac_os_nadc_os_p
TESTR1,               //        113   cal_dac_o_d    x0      dac_offset[7:0]
TESTR2,               //        114   cal_cm_bal     x20                     cm_bal[5:0]
BYPAADLO,             //        115   test_r1        x0      madc_cmp                cm_man  dac_1bitadc_1bitadc_man
BYPAADHI,             //        116   test_r2        x0                                      test_cntl[3:0]
PCLO,                 //        117   bypass_addr_lo x0      bypass_addr[7:0]
PCHI, //120           //        118   bypass_addr_hi x0                                                      bypass_addr[9:8]
PCSHADLO,             //        119   pc_lo          x0      pc[7:0]                                                 adc_deg
PCSHADHI,             //        120   pc_hi          x0                                      pc[11:8]
PDN1,                 //        121   pc_shad_lo     x0      pc_shad[7:0]
PDN2,                 //        122   pc_shad_hi     x0                                      pc_shad[11:8]
PDN3,                 //        123   pdn1           x0                                      ring_extpm_on   bias_off
PASLO,                //        124   pdn2           x0      hsp_man hsp_on_oadc_man a_on_offdac_man d_on_offgm_man
PASHI};               //        125   pdn3           x0                                              pll_off dsp_man

#endif /* _REGISTERS_H_ */ 