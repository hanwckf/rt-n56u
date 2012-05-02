/* Copyright 2009, Silicon Laboratories */
#ifndef SI321XREGS_H
#define SI321XREGS_H




#define IDA_LO  28
#define IDA_HI	29

#define IAA 30
#define CLOCK_MASK 0x04
#define ID_ACCES_STATUS 31
#define IAS_BIT 1

#define	I_STATUS	31

#define	SPI_MODE	0
#define	PCM_MODE	1
#define	PCM_XMIT_START_COUNT_LSB	2
#define	PCM_XMIT_START_COUNT_MSB	3
#define	PCM_RCV_START_COUNT_LSB	4
#define	PCM_RCV_START_COUNT_MSB	5
#define	DIO	6

#define	AUDIO_LOOPBACK	8
#define	AUDIO_GAIN	9
#define	LINE_IMPEDANCE	10
#define	HYBRID	11
#define	RESERVED12	12
#define	RESERVED13	13
#define	PWR_DOWN1	14
#define	PWR_DOWN2	15
#define	RESERVED16	16
#define	RESERVED17	17
#define	INTRPT_STATUS1	18
#define	INTRPT_STATUS2	19
#define	INTRPT_STATUS3	20
#define	INTRPT_MASK1	21
#define	INTRPT_MASK2	22
#define	INTRPT_MASK3	23
#define	DTMF_DIGIT	24
#define	RESERVED25	25
#define	RESERVED26	26
#define	RESERVED27	27
#define	I_DATA_LOW	28
#define	I_DATA_HIGH	29
#define	I_ADDRESS	30
#define	I_STATUS	31
#define	OSC1	32
#define	OSC2	33
#define	RING_OSC_CTL	34
#define	PULSE_OSC	35
#define	OSC1_ON__LO	36
#define	OSC1_ON_HI	37
#define	OSC1_OFF_LO	38
#define	OSC1_OFF_HI	39
#define	OSC2_ON__LO	40
#define	OSC2_ON_HI	41
#define	OSC2_OFF_LO	42
#define	OSC2_OFF_HI	43
#define	PULSE_ON__LO	44
#define	PULSE_ON_HI	45
#define	PULSE_OFF_LO	46
#define	PULSE_OFF_HI	47
#define	RING_ON__LO	48
#define	RING_ON_HI	49
#define	RING_OFF_LO	50
#define	RING_OFF_HI	51
#define	FSK_DATA	52	/*		0								fsk_data	*/
#define	RESERVED53	53
#define	RESERVED54	54
#define	RESERVED55	55
#define	RESERVED56	56
#define	RESERVED57	57
#define	RESERVED58	58
#define	RESERVED59	59
#define	RESERVED60	60
#define	RESERVED61	61
#define	RESERVED62	62
#define	RESERVED63	63
#define	LINE_STATE	64
#define			ACTIVATE_LINE 0x11
#define			RING_LINE     0x44
#define	BIAS_SQUELCH	65
#define	BAT_FEED	66
#define	AUTO_STATE	67
#define	LOOP_STAT	68
#define	LOOP_DEBOUCE	69
#define	RT_DEBOUCE	70
#define	LOOP_I_LIMIT	71
#define	ON_HOOK_V	72
#define	COMMON_V	73
#define	BAT_V_HI	74
#define	BAT_V_LO	75
#define	PWR_STAT_DEV	76
#define	PWR_STAT	77
#define	LOOP_V_SENSE	78
#define	LOOP_I_SENSE	79
#define	TIP_V_SENSE	80
#define	RING_V_SENSE	81
#define	BAT_V_HI_SENSE	82
#define	BAT_V_LO_SENSE	83
#define	IQ1	84
#define	IQ2	85
#define	IQ3	86
#define	IQ4	87
#define	IQ5	88
#define	IQ6	89
#define	RESERVED90	90
#define	RESERVED91	91
#define	DCDC_PWM_OFF	92
#define	DCDC	93
#define	DCDC_PW_OFF	94
#define	RESERVED95	95
#define	CALIBR1	96
#define CALIBRATE_LINE 0x78
#define NORMAL_CALIBRATION_COMPLETE 0x20
#define	CALIBR2	97
#define	RING_GAIN_CAL	98
#define	TIP_GAIN_CAL	99
#define	DIFF_I_CAL	100
#define	COMMON_I_CAL	101
#define	I_LIMIT_GAIN_CAL	102
#define	ADC_OFFSET_CAL	103
#define	DAC_ADC_OFFSET	104
#define	DAC_OFFSET_CAL	105
#define	COMMON_BAL_CAL	106
#define	DC_PEAK_CAL	107

/* Indirect Register (decimal) */
#define	DTMF_ROW_0_PEAK	0
#define	DTMF_ROW_1_PEAK	1
#define	DTMF_ROW2_PEAK	2
#define	DTMF_ROW3_PEAK	3
#define	DTMF_COL1_PEAK	4
#define	DTMF_FWD_TWIST	5
#define	DTMF_RVS_TWIST	6
#define	DTMF_ROW_RATIO_THRESH	7
#define	DTMF_COL_RATIO_THRESH	8
#define	DTMF_ROW_2ND_HARM	9
#define	DTMF_COL_2ND_HARM	10
#define	DTMF_PWR_MIN_THRESH	11
#define	DTMF_HOT_LIM_THRESH	12
#define	OSC1_COEF	13
#define	OSC1X	14
#define	OSC1Y	15
#define	OSC2_COEF	16
#define	OSC2X	17
#define	OSC2Y	18
#define	RING_V_OFF	19
#define	RING_OSC_COEF	20
#define	RING_X	21
#define	RING_Y	22
#define	PULSE_ENVEL	23
#define	PULSE_X	24
#define	PULSE_Y	25
#define	RECV_DIGITAL_GAIN	26
#define	XMIT_DIGITAL_GAIN	27
#define	LOOP_CLOSE_THRESH	28
#define	RING_TRIP_THRESH	29
#define	COMMON_MIN_THRESH	30
#define	COMMON_MAX_THRESH	31
#define	PWR_ALARM_Q1Q2	32
#define	PWR_ALARM_Q3Q4	33
#define	PWR_ALARM_Q5Q6	34
#define	LOOP_CLOSURE_FILTER	35
#define	RING_TRIP_FILTER	36
#define	THERM_LP_POLE_Q1Q2	37
#define	THERM_LP_POLE_Q3Q4	38
#define	THERM_LP_POLE_Q5Q6	39
#define	CM_BIAS_RINGING	40
#define	DCDC_MIN_V	41
#define	DCDC_XTRA	42
#define ALL_CHIPS 0x09
#define NO_CHIPS 0
#define	REVC	108	/*		0	ilim_max	fsk_revc	dc_err_en	zs_ext	batsel_pd	lcr_sense	en_subtr	hyst_en	*/
#define	FSK_X_0		99	/*	x	sign				fsk_x_0[15:0]												*/
#define	FSK_COEFF_0	100	/*	x	sign				fsk_coeff_0[15:0]												*/
#define	FSK_X_1		101	/*	x	sign				fsk_x_1[15:0]												*/
#define	FSK_COEFF_1	102	/*	x	sign				fsk_coeff_1[15:0]												*/
#define	FSK_X_01	103	/*	x	sign				fsk_x_01[15:0]												*/
#define	FSK_X_10	104	/*	x	sign				fsk_x_10[15:0]												*/

				
#define	BIT_CALM1_DR97	0x10	/*	CALM1 Monitor ADC Calibration 1. */
#define	BIT_CALM2_DR97	0x08	/*	CALM2 Monitor ADC Calibration 2. */
#define	BIT_CALDAC_DR97	0x04	/*	CALDAC DAC Calibration. */
#define	BIT_ADC_DR97	0x02	/*	CALADC ADC Calibration. */
#define	BIT_CALCM_DR97	0x01	/*	CALCM Common Mode Balance Calibration. */
#define	STANDARD_CAL_DR97	0x18	/*	Calibrations without the ADC and DAC offset and without common mode calibration. */
#define	MASK_CAL_DR96	0x40	/*	CAL Calibration Status Bit. */
#define	BIT_CAL_DR96	0x40	/*	CAL Calibration Control */
#define	BIT_CALR_DR96	0x10	/*	CALR RING Gain Mismatch Calibration. */
#define	BIT_CALT_DR96	0x08	/*	CALT TIP Gain Mismatch Calibration. */
#define	BIT_CALD_DR96	0x04	/*	CALD Differential DAC Gain Calibration. */
#define	BIT_CALC_DR96	0x02	/*	CALC Common Mode DAC Gain Calibration. */
#define	BIT_CALIL_DR96	0x01	/*	CALIL ILIM Calibration. */
#define	STANDARD_CAL_DR96	0x47	/*	Calibrate common mode and differential DAC mode DAC + ILIM */
#define	BIT_IRQE_CMCE	0x04	/*	Calibrate common mode and differential mode DAC + ILIM */
#define CAL_COMPLETE_DR96 0     /*  Value in dr96 after calibration is completed */
#define MAX_CAL_PERIOD 800		/* The longest period in ms. for a calibration to complete. */


#define DISABLE_ALL_DR21 0
#define DISABLE_ALL_DR22 0
#define DISABLE_ALL_DR23 0

#define ENB2_DR23  1<<2 /* enable interrupt for the balance Cal */

#define	OPEN_DR64	0		


#define SI321X_BROADCAST 0xff

#endif

