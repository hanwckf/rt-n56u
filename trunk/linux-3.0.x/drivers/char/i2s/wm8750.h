#ifndef _WM875x_H
#define _WM875x_H

#define bool	unsigned char
#define false 0
#define true 1

/* volume/balance/treble/bass interdependency */
#define VOLUME_MIN -730
#define VOLUME_MAX  60


/* Register addresses and bits */
#define OUTPUT_MUTED                0x2f
#define OUTPUT_0DB                  0x79

#define LOUT1                       0x02
#define LOUT1_LOUT1VOL_MASK         (0x07f << 0)
#define LOUT1_LOUT1VOL(x)           ((x) & 0x7f)
#define LOUT1_LO1ZC                 (1 << 7)
#define LOUT1_LO1VU                 (1 << 8)

#define ROUT1                       0x03
#define ROUT1_ROUT1VOL(x)           ((x) & 0x7f)
#define ROUT1_RO1ZC                 (1 << 7)
#define ROUT1_RO1VU                 (1 << 8)

#define DACCTRL                     0x05
#define DACCTRL_DEEMPH_NONE         (0 << 1)
#define DACCTRL_DEEMPH_32           (1 << 1)
#define DACCTRL_DEEMPH_44           (2 << 1)
#define DACCTRL_DEEMPH_48           (3 << 1)
#define DACCTRL_DEEMPH(x)           ((x) & (0x3 << 1))
#define DACCTRL_DACMU               (1 << 3)
#define DACCTRL_DAT                 (1 << 7)

#define AINTFCE                     0x07
#define AINTFCE_FORMAT_RJUST        (0 << 0)
#define AINTFCE_FORMAT_LJUST        (1 << 0)
#define AINTFCE_FORMAT_I2S          (2 << 0)
#define AINTFCE_FORMAT_DSP          (3 << 0)
#define AINTFCE_FORMAT(x)           ((x) & 0x3)
#define AINTFCE_WL_16               (0 << 2)
#define AINTFCE_WL_20               (1 << 2)
#define AINTFCE_WL_24               (2 << 2)
#define AINTFCE_WL_32               (3 << 2)
#define AINTFCE_WL(x)               (((x) & 0x3) << 2)
#define AINTFCE_LRP                 (1 << 4)
#define AINTFCE_LRSWAP              (1 << 5)
#define AINTFCE_MS                  (1 << 6)
#define AINTFCE_BCLKINV             (1 << 7)

#define CLOCKING                    0x08
#define CLOCKING_SR_USB             (1 << 0)
#define CLOCKING_SR(x)              (((x) & 0x1f) << 1)
#define CLOCKING_MCLK_DIV2          (1 << 6)
#define CLOCKING_BCLK_DIV2          (1 << 7)
#define CLOCKING_BCM(x)				(((x) & 0x3) << 7)

#define LEFTGAIN                    0x0a
#define LEFTGAIN_LDACVOL(x)         ((x) & 0xff)
#define LEFTGAIN_LDVU               (1 << 8)

#define RIGHTGAIN                   0x0b
#define RIGHTGAIN_LDACVOL(x)        ((x) & 0xff)
#define RIGHTGAIN_LDVU              (1 << 8)

#define BASSCTRL                    0x0c
#define BASSCTRL_BASS(x)            ((x) & 0xf)
#define BASSCTRL_BC                 (1 << 6)
#define BASSCTRL_BB                 (1 << 7)

#define TREBCTRL                    0x0d
#define TREBCTRL_TREB(x)            ((x) & 0xf)
#define TREBCTRL_TC                 (1 << 6)

#define RESET                       0x0f
#define RESET_RESET                 0x000

#define ADDITIONAL1                 0x17
#define ADDITIONAL1_TOEN            (1 << 0)
#define ADDITIONAL1_DACINV          (1 << 1)
#define ADDITIONAL1_DMONOMIX_LLRR   (0 << 4)
#define ADDITIONAL1_DMONOMIX_ML0R   (1 << 4)
#define ADDITIONAL1_DMONOMIX_0LMR   (2 << 4)
#define ADDITIONAL1_DMONOMIX_MLMR   (3 << 4)
#define ADDITIONAL1_DMONOMIX(x)     (((x) & 0x03) << 4)
#define ADDITIONAL1_VSEL_LOWEST     (0 << 6)
#define ADDITIONAL1_VSEL_LOW        (1 << 6)
#define ADDITIONAL1_VSEL_DEFAULT2   (2 << 6)
#define ADDITIONAL1_VSEL_DEFAULT    (3 << 6)
#define ADDITIONAL1_VSEL(x)         (((x) & 0x3) << 6)
#define ADDITIONAL1_TSDEN           (1 << 8)

#define ADDITIONAL2                 0x18
#define ADDITIONAL2_DACOSR          (1 << 0)
#define ADDITIONAL2_LRCM_ON         (1 << 2)
#define ADDITIONAL2_HPSWZC          (1 << 3)
#define ADDITIONAL2_ROUT2INV        (1 << 4)
#define ADDITIONAL2_HPSWPOL         (1 << 5)
#define ADDITIONAL2_HPSWEN          (1 << 6)
#define ADDITIONAL2_OUT3SW_VREF     (0 << 7)
#define ADDITIONAL2_OUT3SW_ROUT1    (1 << 7)
#define ADDITIONAL2_OUT3SW_MONOOUT  (2 << 7)
#define ADDITIONAL2_OUT3SW_R_MIX_OUT (3 << 7)
#define ADDITIONAL2_OUT3SW(x)       (((x) & 0x3) << 7)

#define PWRMGMT1                    0x19
#define PWRMGMT1_DIGENB             (1 << 0)
#define PWRMGMT1_VREF               (1 << 6)
#define PWRMGMT1_VMIDSEL_DISABLED   (0 << 7)
#define PWRMGMT1_VMIDSEL_50K        (1 << 7)
#define PWRMGMT1_VMIDSEL_500K       (2 << 7)
#define PWRMGMT1_VMIDSEL_5K         (3 << 7)
#define PWRMGMT1_VMIDSEL(x)         (((x) & 0x3) << 7)

#define PWRMGMT2                    0x1a
#define PWRMGMT2_OUT3               (1 << 1)
#define PWRMGMT2_MOUT               (1 << 2)
#define PWRMGMT2_ROUT2              (1 << 3)
#define PWRMGMT2_LOUT2              (1 << 4)
#define PWRMGMT2_ROUT1              (1 << 5)
#define PWRMGMT2_LOUT1              (1 << 6)
#define PWRMGMT2_DACR               (1 << 7)
#define PWRMGMT2_DACL               (1 << 8)

#define ADDITIONAL3                 0x1b
#define ADDITIONAL3_ADCLRM(x)       (((x) & 0x3) << 7)
#define ADDITIONAL3_HPFLREN         (1 << 5)
#define ADDITIONAL3_VROI            (1 << 6)

#define LEFTMIX1                    0x22
#define LEFTMIX1_LI2LO_DEFAULT      (5 << 4)
#define LEFTMIX1_LI2LOVOL(x)        (((x) & 0x7) << 4)
#define LEFTMIX1_LI2LO              (1 << 7)
#define LEFTMIX1_LD2LO              (1 << 8)

#define LEFTMIX2                    0x23
#define LEFTMIX2_MI2LO_DEFAULT      (5 << 4)
#define LEFTMIX2_MI2LOVOL(x)        (((x) & 0x7) << 4)
#define LEFTMIX2_MI2LO              (1 << 7)
#define LEFTMIX2_RD2LO              (1 << 8)

#define RIGHTMIX1                   0x24
#define RIGHTMIX1_MI2RO_DEFAULT     (5 << 4)
#define RIGHTMIX1_MI2ROVOL(x)       (((x) & 0x7) << 4)
#define RIGHTMIX1_MI2RO             (1 << 7)
#define RIGHTMIX1_LD2RO             (1 << 8)

#define RIGHTMIX2                   0x25
#define RIGHTMIX2_RI2RO_DEFAULT     (5 << 4)
#define RIGHTMIX2_RI2ROVOL(x)       (((x) & 0x7) << 4)
#define RIGHTMIX2_RI2RO             (1 << 7)
#define RIGHTMIX2_RD2RO             (1 << 8)

#define MONOMIX1                    0x26
#define MONOMIX1_DMEN               (1 << 0)
#define MONOMIX1_LI2MOVOL(x)        (((x) & 0x7) << 4)
#define MONOMIX1_LI2MO              (1 << 7)
#define MONOMIX1_LD2MO              (1 << 8)

#define MONOMIX2                    0x27
#define MONOMIX2_RI2MOVOL(x)        (((x) & 0x7) << 4)
#define MONOMIX2_RI2MO              (1 << 7)
#define MONOMIX2_RD2MO              (1 << 8)

#define LOUT2                       0x28
#define LOUT2_LOUT2VOL(x)           ((x) & 0x7f)
#define LOUT2_LO2ZC                 (1 << 7)
#define LOUT2_LO2VU                 (1 << 8)

#define ROUT2                       0x29
#define ROUT2_ROUT2VOL(x)           ((x) & 0x7f)
#define ROUT2_RO2ZC                 (1 << 7)
#define ROUT2_RO2VU                 (1 << 8)

#define MONOOUT                     0x2a
#define MONOOUT_MONOOUTVOL(x)       ((x) & 0x7f)
#define MONOOUT_MOZC                (1 << 7)

/* WM8750 only register */
#define LINV						0x00
#define LINV_LIVU					(1 << 8)
#define LINV_LINMUTE					(1 << 7)
#define LINV_LINVOL(x)            	((x) & 0x3f)
#define LINV_LIZC                 	(1 << 6)

#define RINV						0x01
#define RINV_RIVU					(1 << 8)
#define RINV_RINMUTE					(1 << 7)
#define RINV_RINVOL(x)            	((x) & 0x3f)
#define RINV_RIZC                   (1 << 6)

#define ALC1							0x11
#define ALC1_ALCOFF						(0x0 << 7)
#define	ALC1_ALCRONLY					(0x1 << 7)
#define ALC1_ALCLONLY					(0x2 << 7)
#define ALC1_ALCSTEREO					(0x3 << 7)
#define ALC1_ALCSEL(x)					(((x) & 0x3) << 7)
#define ALC1_SET_MAXGAIN(x)				((x & 0x7) << 4)
#define ALC1_GET_MAXGAIN(x)				((x) & (0x7 << 4))
#define ALC1_ALCL(x)					((x) & 0x0f)	

#define ALC2							0x12
#define ALC2_ALCZC_ON                 	(1 << 7)
#define ALC2_HLD(x)						((x) & 0x0f)

#define ALC3							0x13
#define ALC3_SET_DCY(x)					((x & 0x0f) << 4)
#define ALC3_GET_DCY(x)					((x) & (0x0f << 4))
#define ALC3_ATK(x)						((x) & 0x0f)

#define NOISEGATE						0x14
#define NOISEGATE_SET_NGTH(x)			((x & 0x1f) << 3)
#define NOISEGATE_GET_NGTH(x)			((x) & (0x1f << 3))
#define NOISEGATE_MUTEADCOUTPUT			(0x2 << 1)
#define NOISEGATE_SET_NGG(x)			((x & 0x03) << 1)
#define NOISEGATE_GET_NGG(x)			((x) & (0x03 << 1))
#define NOISEGATE_NGAT_ENABLE	        1

#define LADCVOL							0x15
#define LADCVOL_LAVU_EN					(1 << 7)
#define LADCVOL_LADCVOL(x)				((x) & 0x0ff)

#define RADCVOL							0x16
#define RADCVOL_RAVU_EN					(1 << 7)
#define RADCVOL_RADCVOL(x)				((x) & 0x0ff)

#define ADCINMODE						0x1F
#define ADCINMODE_DS_IN2				(1 << 7)
#define ADCINMODE_MONOMIX_LADC			(0x1 << 6)
#define ADCINMODE_MONOMIX_RADC			(0x2 << 6)
#define ADCINMODE_MONOMIX_MONO			(0x3 << 6)
#define ADCINMODE_SET_MONOMIX(x)		((x & 0x3) << 6)
#define ADCINMODE_GET_MONOMIX(x)		((x) & (0x3 << 6))
#define ADCINMODE_RDCM_EN				(1 << 5)
#define ADCINMODE_LDCM_EN				(1 << 4)

#define ADCLPATH						0x20
#define ADCLPATH_LINSEL_IN1				(0x0 << 6)
#define ADCLPATH_LINSEL_IN2				(0x1 << 6)
#define ADCLPATH_LINSEL_IN3				(0x2 << 6)
#define ADCLPATH_LINSEL_DS				(0x3 << 6)
#define ADCLPATH_SET_LINSEL(x)			((x & 0x3) << 6)
#define ADCLPATH_LMICBOOST_13DB			(0x1 << 4)
#define ADCLPATH_LMICBOOST_20DB			(0x2 << 4)
#define ADCLPATH_LMICBOOST_29DB			(0x3 << 4)
#define ADCLPATH_SET_LMICBOOST(x)		((x & 0x3) << 4)

#define ADCRPATH						0x21
#define ADCRPATH_RINSEL_IN1				(0x0 << 6)
#define ADCRPATH_RINSEL_IN2				(0x1 << 6)
#define ADCRPATH_RINSEL_IN3				(0x2 << 6)
#define ADCRPATH_RINSEL_DS				(0x3 << 6)
#define ADCRPATH_SET_RINSEL(x)			((x & 0x3) << 6)
#define ADCRPATH_RMICBOOST_13DB			(0x1 << 4)
#define ADCRPATH_RMICBOOST_20DB			(0x2 << 4)
#define ADCRPATH_RMICBOOST_29DB			(0x3 << 4)
#define ADCRPATH_SET_RMICBOOST(x)		((x & 0x3) << 4)


/* PWRMGMT1 */
#define PWRMGMT1_AINL				(1 << 5)
#define PWRMGMT1_AINR				(1 << 4)
#define PWRMGMT1_ADCL             	(1 << 3)
#define PWRMGMT1_ADCR             	(1 << 2)
#define PWRMGMT1_MICB             	(1 << 1)

/* ADC/DAC control */
#define ADCCTRL                     0x05
#define ADCCTRL_ADCDIV2	         	(1 << 8)
#define ADCCTRL_DACDIV2	         	(1 << 7)
#define ADCCTRL_ADCPOL(x)           (((x) & 0x3) << 5)
#define ADCCTRL_DACMU               (1 << 3)
#define ADCCTRL_DEEMPH_44           (2 << 1)
#define ADCCTRL_DEEMPH_48           (3 << 1)
#define ADCCTRL_DEEMPH(x)           (((x) & 0x3) << 1)
#define ADCCTRL_ADCHPD              (1 << 0)

//#define ADDITIONAL3                 0x1B
#define ADDITIONAL3_ADCLRM(x)		(((x) & 0x3) << 7)
//#define ADDITIONAL3_VROI			(1 << 6)
//#define ADDITIONAL3_HPFLREN			(1 << 5)

#define LEFTMIX1_LMIXSEL_LIN1		0
#define LEFTMIX1_LMIXSEL_LIN2		1
#define LEFTMIX1_LMIXSEL_LIN3		2
#define LEFTMIX1_LMIXSEL_LADC		3
#define LEFTMIX1_LMIXSEL_DIN		4


#define RIGHTMIX1_RMIXSEL_RIN1		0
#define RIGHTMIX1_RMIXSEL_RIN2		1
#define RIGHTMIX1_RMIXSEL_RIN3		2
#define RIGHTMIX1_RMIXSEL_RADC		3
#define RIGHTMIX1_RMIXSEL_DIN		4	

/* codec API */
void audiohw_preinit(void);
void audiohw_postinit(int bSlave, int Ain, int Aout);
void audiohw_close(void);
void audiohw_set_frequency(int fsel);
void audiohw_set_MCLK(unsigned int bUsb);
void audiohw_mute(bool mute);
				
#endif /* _WM875x_H */
