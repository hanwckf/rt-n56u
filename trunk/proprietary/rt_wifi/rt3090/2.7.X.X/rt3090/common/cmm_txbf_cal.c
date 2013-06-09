/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2009, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_txbf_cal.c

	Abstract:
	Tx Beamforming calibration and profile related functions

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
	Shiang     2010/07/12
*/

#include "rt_config.h"


/* 
	iAtan2 - fixed point atan2. Returns +/-pi. Scaled so pi=0x1000
		Code was taken from MyCordic:
			int MyCordic(int y, int x, int shift, int iter, int *alpha)
			Parameters are hard coded so it's equivalent to MyCordic(y, x, 0, 11, alpha10);
*/
static int iAtan2(int y, int x)
{
	int z = 0, xtemp, i;
	int onepi, halfpi;
	int shift=0;
	int iter=11;
	static int alpha[11] = {0x400, 0x25c, 0x13f, 0x0a2,
				0x051, 0x028, 0x014, 0x00a,
				0x005, 0x002, 0x001};
	
	onepi = (alpha[0]<<2), 
	halfpi = (alpha[0]<<1);
 
    if (x == 0) {
		if (y == 0)
              z = 0;
        else if (y > 0)
                z = halfpi;
        else
            z = -halfpi;
		}
    else if ((x < 0) && (y == 0)){
            z = -onepi;
		}
    else if ((x > 0) && (y == 0)){
            z = 0;
		}
	else{
		x <<= shift;
		y <<= shift;
		xtemp = x; 
		if ((x < 0) && (y > 0)){
            x = y;
            y = -xtemp;
            z = halfpi;
			}
		else if ((x < 0) && (y < 0)){
            x = -y;
            y = xtemp;
            z = -halfpi;
			}      
		for (i = 0; i < iter; i++){
			/* printf("%d %d %x\n", x, y, z); */
			if (y == 0)
				break;
			if (y < 0) {
				z -= alpha[i];
				xtemp = x - (y>>i);
				y = y + (x>>i);
				x = xtemp;
			}
			else {
				z += alpha[i];
				xtemp = x + (y>>i);
				y = y - (x>>i);
				x = xtemp;
				}
		}
	}

	if (z == alpha[0]*4)
		z = -(alpha[0]*4);
 
	return z;
}


/*
	isqrt - fixed point sqrt
		x - unsigned value
*/
static UINT32 isqrt (UINT32 x) 
{ 
	UINT32 base, y;

	if (x &      0xF0000000)
		base = 1<<15;
	else if (x & 0x0F000000)
		base = 1<<13;
	else if (x & 0x00F00000)
		base = 1<<11;
	else if (x & 0x000F0000)
		base = 1<<9;
	else
		base = 1<<7;

    y = 0; 
    while (base) { 
		y += base; 
		if  ((y * y) > x)
			y -= base;
		base >>= 1;
    }
    return y; 
} 


/*
	icexp - fixed point complex exponential
		phase - 0 to 255 representing 0 to 2pi
		return cos and sin in 1p10 format
*/
static void icexp(short c[2], int phase)
{
	/* cosine table generated with Matlab: round(1024*cos(2*pi*[0:255]/256) */
	static short cosTable[256] = {
		1024, 1024, 1023, 1021, 1019, 1016, 1013, 1009,
		1004, 999, 993, 987, 980, 972, 964, 955,
		946, 936, 926, 915, 903, 891, 878, 865,
		851, 837, 822, 807, 792, 775, 759, 742,
		724, 706, 688, 669, 650, 630, 610, 590,
		569, 548, 526, 505, 483, 460, 438, 415,
		392, 369, 345, 321, 297, 273, 249, 224,
		200, 175, 150, 125, 100, 75, 50, 25,
		0, -25, -50, -75, -100, -125, -150, -175,
		-200, -224, -249, -273, -297, -321, -345, -369,
		-392, -415, -438, -460, -483, -505, -526, -548,
		-569, -590, -610, -630, -650, -669, -688, -706,
		-724, -742, -759, -775, -792, -807, -822, -837,
		-851, -865, -878, -891, -903, -915, -926, -936,
		-946, -955, -964, -972, -980, -987, -993, -999,
		-1004, -1009, -1013, -1016, -1019, -1021, -1023, -1024,
		-1024, -1024, -1023, -1021, -1019, -1016, -1013, -1009,
		-1004, -999, -993, -987, -980, -972, -964, -955,
		-946, -936, -926, -915, -903, -891, -878, -865,
		-851, -837, -822, -807, -792, -775, -759, -742,
		-724, -706, -688, -669, -650, -630, -610, -590,
		-569, -548, -526, -505, -483, -460, -438, -415,
		-392, -369, -345, -321, -297, -273, -249, -224,
		-200, -175, -150, -125, -100, -75, -50, -25,
		0, 25, 50, 75, 100, 125, 150, 175,
		200, 224, 249, 273, 297, 321, 345, 369,
		392, 415, 438, 460, 483, 505, 526, 548,
		569, 590, 610, 630, 650, 669, 688, 706,
		724, 742, 759, 775, 792, 807, 822, 837,
		851, 865, 878, 891, 903, 915, 926, 936,
		946, 955, 964, 972, 980, 987, 993, 999,
		1004, 1009, 1013, 1016, 1019, 1021, 1023, 1024};
	c[0] = cosTable[phase & 0xFF];
	c[1] = cosTable[(phase-64) & 0xFF];
}


/*
	icMult - fixed point complex multiply
		r = a*b
*/
static void icMult(INT32 r[2], INT32 a[2], INT32 b0, INT32 b1)
{
	INT32 t;
	t = a[0]*b0 - a[1]*b1;
	r[1] = a[0]*b1 + a[1]*b0;
	r[0] = t;
}


/*
	------------ DIVIDER AND LNA CALIBRATION --------
*/
typedef	struct {
	LONG	i:8;
	LONG	q:8;
}	COMPLEX_VALUE;		/* Signed 8-bit complex values */

#define CALC_LENGTH		1024				/* Number of samples used to perform phase calculation for LNA or Divider Calibration */
#define DIVCAL_SKIP_LENGTH	512				/* Number of samples to skip at the beginning */
#define CALC_LENGTH_DC	(CALC_LENGTH+512)	/* Number of samples used for DC removal */
#define MAX_CAPTURE_LENGTH		4096			/* Maximum number of samples to capture */
#define DIVCAL_CAPTURE_LENGTH	(CALC_LENGTH_DC+DIVCAL_SKIP_LENGTH+256)	/* Length of capture for Divider or LNA Calibration */

#define FIXED_M_PI		0x1000						/* Scaling for fixed point PI */
#define DEG(rad)		(radToDeg180(rad-FIXED_M_PI)+180)	/* Convert fixed radians (0x1000=pi) to degrees range [0 360) */
#define DEG180(rad)		radToDeg180(rad)		/* Convert fixed radians (0x1000=pi) to degrees range [-180 180) */

#define BYTE_PHASE_SHIFT		5					/* Shift to convert from byte phase (0x80=pi) to normal phase (0x1000=pi) */
#define CONVERT_TO_BYTE_PHASE(p)	(int)(((p)+(1<<(BYTE_PHASE_SHIFT-1)))>>BYTE_PHASE_SHIFT)	/* Convert from normal phase to byte phase */

#define R65_LNA_LOW		0x4
#define R65_LNA_MID		0x8
#define R65_LNA_HIGH	0xC

#define MAX_RF_TX_ALC	0x7F

/*
	radMod2pi - converts angle in radians to the range [-pi pi)
*/
static LONG radMod2pi(LONG a)
{
	while (a < -FIXED_M_PI)
		a += 2*FIXED_M_PI;
	while (a >= FIXED_M_PI)
		a -= 2*FIXED_M_PI;

	return a;
}


/*
	radToDeg180 - converts angle in radians to the deg range [-180 180)
*/
static int radToDeg180(LONG rad)
{
	return (int)(radMod2pi(rad)*180/FIXED_M_PI);
}


/*
	avgPhase - computes the average phase.
		Phase is adjusted so all values are within the range mPhase[0] +/-pi
			mPhase - values to average (radians)
			pLength - number of values to average
		return average
*/
static LONG avgPhase(LONG mPhase[], int pLength)
{
	int i;
	short cval[2];
	LONG sumCos = 0, sumSin=0;
	for (i=0; i<pLength; i++) {
		icexp(cval, CONVERT_TO_BYTE_PHASE(mPhase[i]));
		sumCos += cval[0];
		sumSin += cval[1];
	}

	return iAtan2(sumSin, sumCos);
}


typedef
	COMPLEX_VALUE (*PCAP_IQ_DATA)[3];	/* CAP_IQ_DATA - Buffer to hold I/Q data for three RX chains */


/*
	RemoveDC - calculate mean and subtract. Return peak values
		peak - used to return the peak value of the three RX chains
		iqData - pointer to array of I/Q data for the three RX chains. DC is removed from the samples
		dataLength - number of samples in iqData
*/
static void RemoveDC(
	IN int peak[3],
	IN COMPLEX_VALUE (*iqData)[3],
	IN int dataLength)
{
	int i, j;
	int dcI[3] = {0, 0, 0};
	int dcQ[3] = {0, 0, 0};

	/* Calculate DC offset for each RX chain */
	for (i=0; i<dataLength; i++) {
		for (j=0; j<3; j++) {
			dcI[j] += iqData[i][j].i;
			dcQ[j] += iqData[i][j].q;
		}
	}

	for (j=0; j<3; j++) {
		dcI[j] /= dataLength;
		dcQ[j] /= dataLength;
	}

	/* Subtract DC and find peak */
	peak[0] = peak[1] = peak[2] = 0;

	for (i=0; i<dataLength; i++) {
		for (j=0; j<3; j++) {
			int sati = iqData[i][j].i - dcI[j];
			int satq = iqData[i][j].q - dcQ[j];

			/* Saturate */
			if (sati > 127)
				sati = 127;
			else if (sati < -128)
				sati = -128;
			iqData[i][j].i = sati;

			if (satq > 127)
				satq = 127;
			else if (satq < -128)
				satq = -128;
			iqData[i][j].q = satq;

			/* Record peak */
			if (peak[j] < iqData[i][j].i)
				peak[j] = iqData[i][j].i;
			if (peak[j] < iqData[i][j].q)
				peak[j] = iqData[i][j].q;
		}
	}
}


/*
	CalcRFCalPhase - process RF calibration to calculate phase of the three channels
		Parameters:
			phase - returns the phase of each channel. Fixed point value scaled so 0x1000 = PI
			avgI, avgQ - returns the avg I/Q of each channel. Implied scale factor of 256
			peak - returns the peak value of each channel after DC removal
			iqData - the input I/Q data for three channels. DC is removed.
			relPhase - If true it returns phase relative to Ant1. Otherwise it returns the 
						phase relative to the reference signal.
			actTx - index of an active TX chain, used to detect start of signal
*/
static void CalcRFCalPhase(
	OUT LONG phase[3],
	OUT int avgI[3],
	OUT int avgQ[3],
	OUT int peak[3],
	IN COMPLEX_VALUE (*iqData)[3],
	IN BOOLEAN relPhase,
	IN int actTx)
{
	int i, j;
	LONG sumI[3], sumQ[3];
	static CHAR refSignal[64] = {	/* round(sin(-[0:63]*6*pi/64)*127) - three cycles per 64 samples */
		0, -37, -71, -98, -117, -126, -125, -112,
		-90, -60, -25, 12, 49, 81, 106, 122,
		127, 122, 106, 81, 49, 12, -25, -60,
		-90, -112, -125, -126, -117, -98, -71, -37,
		0, 37, 71, 98, 117, 126, 125, 112,
		90, 60, 25, -12, -49, -81, -106, -122,
		-127, -122, -106, -81, -49, -12, 25, 60,
		90, 112, 125, 126, 117, 98, 71, 37};


	/* Skip the first DIVCAL_SKIP_LENGTH samples to avoid the transient at the beginning */
	iqData += DIVCAL_SKIP_LENGTH;

	/* Remove DC offset to help with low signal levels */
	RemoveDC(peak, iqData, CALC_LENGTH_DC);

	/* Search active channel to find sample with abs>12 */
	for (i=0; i<(CALC_LENGTH_DC-CALC_LENGTH-16); i++, iqData++) {
		if ((iqData[0][actTx].i*iqData[0][actTx].i + iqData[0][actTx].q*iqData[0][actTx].q) >= 144)
			break;
	}

	/* Move in 16 samples */
	iqData += 16;

	/* Sum the I and Q then calculate the angle of the sum */
	sumI[0] = sumI[1] = sumI[2] = 0;
	sumQ[0] = sumQ[1] = sumQ[2] = 0;

	for (i=0; i<CALC_LENGTH; i++) {
		/* Either calculate the phase relative to Ant1 or phase relative to reference */
		if (relPhase) {
			sumQ[0] += -iqData[i][0].i*iqData[i][1].q + iqData[i][0].q*iqData[i][1].i;
			sumI[0] +=  iqData[i][0].i*iqData[i][1].i + iqData[i][0].q*iqData[i][1].q;
			sumQ[2] += -iqData[i][2].i*iqData[i][1].q + iqData[i][2].q*iqData[i][1].i;
			sumI[2] +=  iqData[i][2].i*iqData[i][1].i + iqData[i][2].q*iqData[i][1].q;
		}
		else {
			int cval= refSignal[(i+16) % 64];
			int sval= refSignal[i % 64];
			for (j=0; j<3; j++) {
				sumQ[j] += -iqData[i][j].i*sval + iqData[i][j].q*cval;
				sumI[j] +=  iqData[i][j].i*cval + iqData[i][j].q*sval;
			}
		}
	}


	for (i=0; i<3; i++) {
		if (relPhase && i==1) {
			phase[i] = 0;
			avgI[i] = avgQ[i] = 0;
		}
		else {
		phase[i] = iAtan2(sumQ[i]>>6, sumI[i]>>6);
			/* Multiplication by refSignal added a scale factor of 128. Shift left by 1 for 256 scale factor */
			avgI[i] = (sumI[i]<<1)/CALC_LENGTH;
			avgQ[i] = (sumQ[i]<<1)/CALC_LENGTH;
		}
	}
}


#ifdef DBG
#ifdef LINUX
/* #define TIMESTAMP_CAL_CAPTURE0 */
/* #define TIMESTAMP_CAL_CAPTURE1 */
#endif /* LINUX */
#endif /* DBG */

/*
	DoCalibrationCapture - perform capture with specified BBP and RF register settings
		txAnt - antenna to enable (0, 1 or 2). -1 enables all antennas
		papdIQParam - table of PAPD IQ values. Optional, if NULL then PAPD is not used
		lnaSettings - LNA settings for the 3 chains. LNA is encoded in b3:b2, same as R65
		r66Setting, r36r37Setting, r25Setting - Other BBP/RF reg settings
*/
static void DoCalibrationCapture(
	PRTMP_ADAPTER	pAd,
	int txAnt,
	USHORT papdIQParam[3],
	UCHAR lnaSettings[3],
	UCHAR r66Setting,
	USHORT r36r37Setting,
	UCHAR r25Setting)
{
	UINT32 capCtrl;
	UINT8 r25Value, r27Value, r65Value, r66Value[3], r186Value, r250Value, tmpValue;
	UINT8 rf36Value, rf37Value;
	UINT8 rf53Value, rf54Value, rf55Value;

	UINT16 papdIQ[3] = {0,0,0};
	BOOLEAN usePapd = FALSE;

	int i, j, papdEntry, rxVGAAdjust, txALC;

#ifdef TIMESTAMP_CAL_CAPTURE0
	struct timeval tval0, tval1, tval2;
#endif

	r25Value = r186Value = r65Value = r250Value = r27Value = tmpValue = 0;

#ifdef RT3593
#ifdef HIGH_POWER_SUPPORT
	if (IS_RT3593(pAd))
	{
		papdEntry = 0x7f7f;
		rxVGAAdjust = 8;
		txALC = MAX_RF_TX_ALC;
	}
	else
#endif /* HIGH_POWER_SUPPORT */
#endif /* RT3593 */
	{
		papdEntry = 0x7f00;
		rxVGAAdjust = 0;
		txALC = 0;
	}

	/* 
		For backwards compatibility txAnt:
			0, 1, 2 = only enable Ant0/1/2
			-1 = if no papd parameters then don't use papd. Otherwise read papd parameters
	*/
	if (txAnt==0 || txAnt==1 || txAnt==2)
	{
		papdIQ[txAnt] = papdEntry;
		usePapd = TRUE;
	}
	else if (papdIQParam != NULL)
	{
		papdIQ[0] = papdIQParam[0];
		papdIQ[1] = papdIQParam[1];
		papdIQ[2] = papdIQParam[2];
		usePapd = TRUE;
	}
	/* Read RF R25 */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R25, &r25Value);

	/* Disable MAC Tx/Rx */
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x00);

	/* Wait up to 50ms for MAC to empty queues */
	for (i=0; i<2500; i++)
	{
		UINT32 MacCsr12;
		RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &MacCsr12);
		if ((MacCsr12 & 0x3)==0)
			break;
		RTMPusecDelay(20);
	}
	/*DBGPRINT(RT_DEBUG_ERROR, ("==>CalCap: i=%dus\n", i*20)); */

	/* Disable Tx/Rx Queue */
	RTMP_IO_WRITE32(pAd, PBF_CFG, 0x00000000);

	/* Overwrite PAPD table and enable PAPD */
	if (usePapd) {
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R186, &r186Value);
		for (i=0; i<3; i++) {
			for (j=0; j<32; j++) {
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, (i<<6) | (j<<1));
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, (papdIQ[i]>>8) & 0xFF);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, (i<<6) | (j<<1) | 1);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, papdIQ[i] & 0xFF);
			}
		}
		/* printk("PAPD = [%4x %4x %4x]\n", papdIQ[0], papdIQ[1], papdIQ[2]); */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x80);
	}

	/* Enable Capture mode */
	RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, 0x00004E80);

#ifdef TIMESTAMP_CAL_CAPTURE0
	do_gettimeofday(&tval0);
#endif

	/* Save and set LNA - BBP R65 and R250 */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R65, &r65Value);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R250, &r250Value);
	if (lnaSettings != NULL) {
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, lnaSettings[0]);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R250, (r250Value & ~0x3c) | 0x40 | lnaSettings[1] | (lnaSettings[2]<<2));
	}

	/* Save and set R66 (VGA) */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R27, &r27Value);
	for (i=0; i<3; i++) {
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, (r27Value & ~0x60) | (i<<5));
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R66, &r66Value[i]);
		if (r66Setting != 0)
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, r66Setting+rxVGAAdjust);
	}

	/* Enable RF Loopback */
	RT30xxReadRFRegister(pAd, RF_R36, &rf36Value);
	RT30xxWriteRFRegister(pAd, RF_R36, rf36Value | (r36r37Setting & 0xFF));
	RT30xxReadRFRegister(pAd, RF_R37, &rf37Value);
	RT30xxWriteRFRegister(pAd, RF_R37, rf37Value | (r36r37Setting >> 8));

	/* Set Maximum RF gain */
	if (txALC != 0) {
		RT30xxReadRFRegister(pAd, RF_R53, &rf53Value);
		RT30xxReadRFRegister(pAd, RF_R54, &rf54Value);
		RT30xxReadRFRegister(pAd, RF_R55, &rf55Value);
		RT30xxWriteRFRegister(pAd, RF_R53, txALC);
		RT30xxWriteRFRegister(pAd, RF_R54, txALC);
		RT30xxWriteRFRegister(pAd, RF_R55, txALC);
	}

	/* Initialize Capture. */
	capCtrl = 0;		/* ADC capture. Offset=0 */
	RTMP_IO_WRITE32(pAd, PBF_CAP_CTRL, capCtrl | 0x40000000);

	/* Enable MAC RX, trigger capture and start RF loopback */
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x08);
	RTMP_IO_WRITE32(pAd, PBF_CAP_CTRL, capCtrl | 0x20000000);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R25, r25Value | r25Setting);

#ifdef TIMESTAMP_CAL_CAPTURE0
	do_gettimeofday(&tval1);
#endif

	/* Wait up to 1ms for capture buffer to fill */
	for (i=0; i<20; i++)
	{
		RTMP_IO_READ32(pAd, PBF_CAP_CTRL, &capCtrl);
		if ((capCtrl & 0x40000000)==0)
			break;
		RTMPusecDelay(50);
	}

	/* Stop RX */
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x00);
	
	/* Wait up to 5ms for RC Cal to finish */
	for (i=0; i<100; i++)
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R25, &tmpValue);
		if ((tmpValue & 0x10)==0)
			break;
		RTMPusecDelay(50);
	}

	/* Restore BBP R65, R250, R66, R27, R186 and RF R36, R53-54, */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, r65Value);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R250, r250Value);

	for (i=0; i<3; i++) {
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, (r27Value & ~0x60) | (i<<5));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, r66Value[i]);
	}
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, r27Value);

	if (usePapd)
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, r186Value);

	RT30xxWriteRFRegister(pAd, RF_R36, rf36Value);
	RT30xxWriteRFRegister(pAd, RF_R37, rf37Value);

	if (txALC != 0) {
		RT30xxWriteRFRegister(pAd, RF_R53, rf53Value);
		RT30xxWriteRFRegister(pAd, RF_R54, rf54Value);
		RT30xxWriteRFRegister(pAd, RF_R55, rf55Value);
	}

#ifdef TIMESTAMP_CAL_CAPTURE0
	do_gettimeofday(&tval2);

	DBGPRINT(RT_DEBUG_WARN, ("DoCalibrationCapture c=%x, t1=%ld t2=%ld\n", capCtrl,
				tval1.tv_usec - tval0.tv_usec, tval2.tv_usec - tval0.tv_usec));
#endif

}


/*
	ReadCaptureData - Read capture data from MAC memory
		iqData - used to return the data read. Array of samples for three RF chains
		numSamples - the number of samples to read
*/
static void ReadCaptureData(PRTMP_ADAPTER pAd, COMPLEX_VALUE iqData[][3], int numSamples)
{
	UINT32 CaptureStartAddr;
	UINT32 PKT_Addr;
	UINT32 SMM_Addr;
	int i;

	/*********************************************************/
	/* Read [0x440] bit[12:0] */
	RTMP_IO_READ32(pAd, PBF_CAP_CTRL, &CaptureStartAddr);
	CaptureStartAddr = CaptureStartAddr & 0x00001FFF;

	PKT_Addr = 0x8000+(CaptureStartAddr*4);
	SMM_Addr = 0x4000+(CaptureStartAddr*2);

	for (i=0; i<numSamples; i++) {
		CAPTURE_MODE_PACKET_BUFFER SMM, PKT1, PKT2;

		RTMP_IO_READ32(pAd, SMM_Addr, &SMM.Value);
		SMM_Addr += 4;
		if (SMM_Addr >= 0x8000)  SMM_Addr = SMM_Addr - 0x4000;		

		RTMP_IO_READ32(pAd, PKT_Addr, &PKT1.Value);
		PKT_Addr += 4;
		if (PKT_Addr >= 0x10000) PKT_Addr = PKT_Addr - 0x8000;

		RTMP_IO_READ32(pAd, PKT_Addr, &PKT2.Value);
		PKT_Addr += 4;
		if (PKT_Addr >= 0x10000) PKT_Addr = PKT_Addr - 0x8000;

		/* Reorder samples so iqData[i][0] is Ant0, iqData[i][1] is Ant1, iqData[i][2] is Ant2 */
		iqData[i][2].i = SMM.field.BYTE0;
		iqData[i][2].q = SMM.field.BYTE1;
		iqData[i][1].i = PKT1.field.BYTE2;
		iqData[i][1].q = PKT1.field.BYTE3;
		iqData[i][0].i = PKT1.field.BYTE0;
		iqData[i][0].q = PKT1.field.BYTE1;
		if (++i >= numSamples)
			break;

		iqData[i][2].i = SMM.field.BYTE2;
		iqData[i][2].q = SMM.field.BYTE3;
		iqData[i][1].i = PKT2.field.BYTE2;
		iqData[i][1].q = PKT2.field.BYTE3;
		iqData[i][0].i = PKT2.field.BYTE0;
		iqData[i][0].q = PKT2.field.BYTE1;
	}
}


/*
	CaptureRFCal - Capture a single RF calibration loopback
		iqData - returns 2048 samples of the 3 channels of IQ data
		txAnt - selects active antenna (0, 1 or 2). -1 selects all
*/
static void CaptureRFCal(PRTMP_ADAPTER pAd, COMPLEX_VALUE iqData[][3], int txAnt)
{
	UCHAR lnaValues[3] = {R65_LNA_HIGH, R65_LNA_HIGH, R65_LNA_HIGH};
	UCHAR channel = pAd->CommonCfg.Channel;

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		channel = pAd->ate.Channel;
#endif /* RALINK_ATE */

	DoCalibrationCapture(pAd, txAnt, NULL, lnaValues, channel<=14? 0x0e: 0x12, 0x70, 0x50);

	ReadCaptureData(pAd, iqData, DIVCAL_CAPTURE_LENGTH);
}


/*
	DisplayCaptureData - Display capture data
		iqData - 3 channels of IQ data
		numSamples - number of samples to display
*/
static void DisplayCaptureData(COMPLEX_VALUE iqData[][3], int numSamples)
{
	int i;
	for (i=0; i<numSamples; i++) {
		printk("%d %d\t%d %d\t%d %d\n",
				iqData[i][0].i, iqData[i][0].q,
				iqData[i][1].i, iqData[i][1].q,
				iqData[i][2].i, iqData[i][2].q);
	}
}


/*
	ITxBFSaveData - save MAC data
		Returns pointer to allocated buffer containing saved data
*/
static UINT32 *ITxBFSaveData(PRTMP_ADAPTER pAd)
{
	UINT32 *saveData, *sdPtr, macAddr, maxAddr;

	/* Save 48KB MAC data. */
	if (os_alloc_mem(pAd, (UCHAR **)&saveData, 0xC000)!= NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s():Alloc memory failed\n", __FUNCTION__));
		return NULL;
	}

	maxAddr = 0x10000;

	for (sdPtr=saveData, macAddr=0x4000; macAddr<maxAddr; macAddr += 4, sdPtr++) {
		RTMP_IO_READ32(pAd, macAddr, sdPtr);
	}
	return saveData;
}


/* 
	ITxBFSaveData - restore MAC data
		saveData - buffer containing data to restore
*/
static void ITxBFRestoreData(PRTMP_ADAPTER pAd, UINT32 *saveData)
{
	UINT32 *sdPtr, macAddr, maxAddr;

	maxAddr = 0x10000;

	for (sdPtr=saveData, macAddr=0x4000; macAddr<maxAddr; macAddr += 4, sdPtr++)
	{
		RTMP_IO_WRITE32(pAd, macAddr, *sdPtr);
	}
}


/*
	mapChannelKHz - map channel number to KHz
*/
static LONG mapChannelKHz(int ch)
{
	long x;
	MAP_CHANNEL_ID_TO_KHZ(ch, x);
	return x;
}


/*
	InterpParam - Interpolate calibration parameters
		ch - channel to interpolate for
		chBeg, chEnd - begining and ending channel
		yBeg, yEnd - the hex phase values corresponding to chBeg and chEnd
*/
static UCHAR InterpParam(int ch, int chBeg, int chEnd, UCHAR yBeg, UCHAR yEnd)
{
	long x, xBeg, xEnd, yDelta;

	x = mapChannelKHz(ch);
	xBeg = mapChannelKHz(chBeg);
	xEnd = mapChannelKHz(chEnd);
	yDelta = yEnd - yBeg;

	/*
		Handle the phase wraparound. We always assume the delta phase is in
		the range [-180, 180] degrees = [0x80, 0x7f] in hex
	*/
	if (yDelta >= 128)
		yDelta -= 256;
	else if (yDelta <= -128)
		yDelta += 256;

	return yBeg + yDelta*(x-xBeg)/(xEnd-xBeg);
}


/*
	ITxBFDivParams - interpolate Divider calibration parameter based on channel and EEPROM
		divValues - returns the Divider Calibration values for this channel
		channel - the channel to interpolate for
		divParams - the Divider Calibration parameters from EEPROM
*/
static void ITxBFDivParams(UCHAR divValues[2], int channel, ITXBF_DIV_PARAMS *divParams)
{
	if (channel <= 14) {
		divValues[0] = InterpParam(channel, 1, 14, divParams->gBeg[0], divParams->gEnd[0]);
		divValues[1] = InterpParam(channel, 1, 14, divParams->gBeg[1], divParams->gEnd[1]);
	}
	else if (channel <= 64) {
		divValues[0] = divParams->aLow[0];
		divValues[1] = divParams->aLow[1];
	}
	else if (channel <= 128) {
		divValues[0] = divParams->aMid[0];
		divValues[1] = divParams->aMid[1];
	}
	else {
		divValues[0] = divParams->aHigh[0];
		divValues[1] = divParams->aHigh[1];
	}
}


/*
	ITxBFLnaParams - interpolate LNA compensation parameter based on channel and EEPROM.
		lnaValues - returns the quantized LNA compensation values for M-L, H-L and H-M
		channel - the channel to interpolate for
		lnaParams - the LNA Calibration parameters from EEPROM
*/
static void ITxBFLnaParams(UCHAR lnaValues[3], int channel, ITXBF_LNA_PARAMS *lnaParams)
{
	int i;

	if (channel <= 14) {
		lnaValues[0] = InterpParam(channel, 1, 14, lnaParams->gBeg[0], lnaParams->gEnd[0]);
		lnaValues[2] = InterpParam(channel, 1, 14, lnaParams->gBeg[1], lnaParams->gEnd[1]);
	}
	else if (channel <= 64) {
		lnaValues[0] = InterpParam(channel, 36, 64, lnaParams->aLowBeg[0], lnaParams->aLowEnd[0]);
		lnaValues[2] = InterpParam(channel, 36, 64, lnaParams->aLowBeg[1], lnaParams->aLowEnd[1]);
	}
	else if (channel <= 128) {
		lnaValues[0] = InterpParam(channel, 100, 128, lnaParams->aMidBeg[0], lnaParams->aMidEnd[0]);
		lnaValues[2] = InterpParam(channel, 100, 128, lnaParams->aMidBeg[1], lnaParams->aMidEnd[1]);
	}
	else {
		lnaValues[0] = InterpParam(channel, 132, 165, lnaParams->aHighBeg[0], lnaParams->aHighEnd[0]);
		lnaValues[2] = InterpParam(channel, 132, 165, lnaParams->aHighBeg[1], lnaParams->aHighEnd[1]);
	}

	/* Compute L-H from M-H and M-L and quantize */
	lnaValues[1] = lnaValues[2]-lnaValues[0];
	for (i=0; i<3; i++)
		lnaValues[i] = (lnaValues[i] + 0x8) & 0xF0;
}


/*
	ITxBFPhaseParams - interpolate Phase compensation parameters based on channel and EEPROM
		phaseValues - returns the Phase compensation values for this channel
		channel - the channel to interpolate for
		phaseParams - the Phase Calibration parameters from EEPROM
*/
static void ITxBFPhaseParams(UCHAR phaseValues[2], int channel, ITXBF_PHASE_PARAMS *phaseParams)
{

	if (channel <= 14) {
		phaseValues[0] = InterpParam(channel, 1, 14, phaseParams->gBeg[0], phaseParams->gEnd[0]);
		phaseValues[1] = InterpParam(channel, 1, 14, phaseParams->gBeg[1], phaseParams->gEnd[1]);
	}
	else if (channel <= 64) {
		phaseValues[0] = InterpParam(channel, 36, 64, phaseParams->aLowBeg[0], phaseParams->aLowEnd[0]);
		phaseValues[1] = InterpParam(channel, 36, 64, phaseParams->aLowBeg[1], phaseParams->aLowEnd[1]);
	}
	else if (channel <= 128) {
		phaseValues[0] = InterpParam(channel, 100, 128, phaseParams->aMidBeg[0], phaseParams->aMidEnd[0]);
		phaseValues[1] = InterpParam(channel, 100, 128, phaseParams->aMidBeg[1], phaseParams->aMidEnd[1]);
	}
	else {
		phaseValues[0] = InterpParam(channel, 132, 165, phaseParams->aHighBeg[0], phaseParams->aHighEnd[0]);
		phaseValues[1] = InterpParam(channel, 132, 165, phaseParams->aHighBeg[1], phaseParams->aHighEnd[1]);
	}
}


#define ITXBF_EEPROM_WORDS		19	/* 38 bytes of ITxBF parameters */



/*
	ITxBFGetEEPROM - Read ITxBF calibration parameters from EEPROM
		phaseParams - pointer to BBP Phase calibration parameters. If NULL then parameters are not returned
		lnaParams - pointer to BBP LNA calibration parameters. If NULL then parameters are not returned
		divParams - divider calibration parameters. If NULL then parameters are not returned
*/
void ITxBFGetEEPROM(
	IN RTMP_ADAPTER *pAd,
	IN ITXBF_PHASE_PARAMS *phaseParams,
	IN ITXBF_LNA_PARAMS *lnaParams,
	IN ITXBF_DIV_PARAMS *divParams)
{
	USHORT	EE_Value[8], andValue;
	int		i;

	/* Get Phase parameters */
	if (phaseParams != NULL) {
		/* Read and check for initialized values */
		andValue = 0xFFFF;
		for (i=0; i<8; i++) {
			RT28xx_EEPROM_READ16(pAd, EEPROM_ITXBF_CAL + 2*i, EE_Value[i]);
			andValue &= EE_Value[i];
		}

		if (andValue == 0xFFFF) {
			memset(phaseParams, 0, sizeof(*phaseParams));
		} else {
			phaseParams->gBeg[0] = (EE_Value[0] & 0x00FF);
			phaseParams->gBeg[1] = (EE_Value[0] & 0xFF00)>>8;
			phaseParams->gEnd[0] = (EE_Value[1] & 0x00FF);
			phaseParams->gEnd[1] = (EE_Value[1] & 0xFF00)>>8;

			phaseParams->aLowBeg[0] = (EE_Value[2] & 0x00FF);
			phaseParams->aLowBeg[1] = (EE_Value[2] & 0xFF00)>>8;
			phaseParams->aLowEnd[0] = (EE_Value[3] & 0x00FF);
			phaseParams->aLowEnd[1] = (EE_Value[3] & 0xFF00)>>8;
			phaseParams->aMidBeg[0] = (EE_Value[4] & 0x00FF);
			phaseParams->aMidBeg[1] = (EE_Value[4] & 0xFF00)>>8;
			phaseParams->aMidEnd[0] = (EE_Value[5] & 0x00FF);
			phaseParams->aMidEnd[1] = (EE_Value[5] & 0xFF00)>>8;
			phaseParams->aHighBeg[0] = (EE_Value[6] & 0x00FF);
			phaseParams->aHighBeg[1] = (EE_Value[6] & 0xFF00)>>8;
			phaseParams->aHighEnd[0] = (EE_Value[7] & 0x00FF);
			phaseParams->aHighEnd[1] = (EE_Value[7] & 0xFF00)>>8;
		}
	}

	/* Get Divider Phase parameters */
	if (divParams != NULL) {
		/* Read and check for initialized values */
		andValue = 0xFFFF;
		for (i=0; i<5; i++) {
			int eeAddr = i<3? EEPROM_ITXBF_CAL+16+2*i: EEPROM_ITXBF_CAL+38+(i-3)*2;
			RT28xx_EEPROM_READ16(pAd, eeAddr, EE_Value[i]);
			andValue &= EE_Value[i];
		}

		if (andValue == 0xFFFF) {
			memset(divParams, 0, sizeof(*divParams));
	}
		else {
			divParams->gBeg[0] = (EE_Value[0] & 0x00FF);
			divParams->gBeg[1] = (EE_Value[0] & 0xFF00)>>8;
			divParams->gEnd[0] = (EE_Value[1] & 0x00FF);
			divParams->gEnd[1] = (EE_Value[1] & 0xFF00)>>8;

			divParams->aLow[0] = (EE_Value[2] & 0x00FF);
			divParams->aLow[1] = (EE_Value[2] & 0xFF00)>>8;
			divParams->aMid[0] = (EE_Value[3] & 0x00FF);
			divParams->aMid[1] = (EE_Value[3] & 0xFF00)>>8;
			divParams->aHigh[0] = (EE_Value[4] & 0x00FF);
			divParams->aHigh[1] = (EE_Value[4] & 0xFF00)>>8;
	}
	}

	/* Get LNA Parameters */
	if (lnaParams != NULL) {
		/* Read and check for initialized values */
		andValue = 0xFFFF;
		for (i=0; i<8; i++) {
			RT28xx_EEPROM_READ16(pAd, EEPROM_ITXBF_CAL + 22 + 2*i, EE_Value[i]);
			andValue &= EE_Value[i];
		}

		if (andValue == 0xFFFF) {
			memset(lnaParams, 0, sizeof(*lnaParams));
	}
		else {
			lnaParams->gBeg[0] = (EE_Value[0] & 0x00FF);
			lnaParams->gBeg[1] = (EE_Value[0] & 0xFF00)>>8;
			lnaParams->gEnd[0] = (EE_Value[1] & 0x00FF);
			lnaParams->gEnd[1] = (EE_Value[1] & 0xFF00)>>8;

			lnaParams->aLowBeg[0] = (EE_Value[2] & 0x00FF);
			lnaParams->aLowBeg[1] = (EE_Value[2] & 0xFF00)>>8;
			lnaParams->aLowEnd[0] = (EE_Value[3] & 0x00FF);
			lnaParams->aLowEnd[1] = (EE_Value[3] & 0xFF00)>>8;
			lnaParams->aMidBeg[0] = (EE_Value[4] & 0x00FF);
			lnaParams->aMidBeg[1] = (EE_Value[4] & 0xFF00)>>8;
			lnaParams->aMidEnd[0] = (EE_Value[5] & 0x00FF);
			lnaParams->aMidEnd[1] = (EE_Value[5] & 0xFF00)>>8;
			lnaParams->aHighBeg[0] = (EE_Value[6] & 0x00FF);
			lnaParams->aHighBeg[1] = (EE_Value[6] & 0xFF00)>>8;
			lnaParams->aHighEnd[0] = (EE_Value[7] & 0x00FF);
			lnaParams->aHighEnd[1] = (EE_Value[7] & 0xFF00)>>8;
		}
	}
}


/*
	ITxBFSetEEPROM - Save ITxBF calibration parameters in EEPROM
		phaseParams - pointer to BBP calibration parameters. If NULL then parameters are not written
		lnaParams - pointer to BBP LNA calibration parameters. If NULL then parameters are not written
		divParams - divider calibration parameters. If NULL then parameters are not written
*/
void ITxBFSetEEPROM(
	IN PRTMP_ADAPTER pAd,
	IN ITXBF_PHASE_PARAMS *phaseParams,
	IN ITXBF_LNA_PARAMS *lnaParams,
	IN ITXBF_DIV_PARAMS *divParams)
{
	USHORT	EE_Value[8], eeTmp;
	int		i, eeAddr;

	/* Set EEPROM parameters */

	/* Phase parameters */
	if (phaseParams != NULL) {
		EE_Value[0] = phaseParams->gBeg[0] | (phaseParams->gBeg[1]<<8);
		EE_Value[1] = phaseParams->gEnd[0] | (phaseParams->gEnd[1]<<8);

		EE_Value[2] = phaseParams->aLowBeg[0] | (phaseParams->aLowBeg[1]<<8);
		EE_Value[3] = phaseParams->aLowEnd[0] | (phaseParams->aLowEnd[1]<<8);
		EE_Value[4] = phaseParams->aMidBeg[0] | (phaseParams->aMidBeg[1]<<8);
		EE_Value[5] = phaseParams->aMidEnd[0] | (phaseParams->aMidEnd[1]<<8);
		EE_Value[6] = phaseParams->aHighBeg[0] | (phaseParams->aHighBeg[1]<<8);
		EE_Value[7] = phaseParams->aHighEnd[0] | (phaseParams->aHighEnd[1]<<8);

		for (i=0; i<8; i++)
		{
			eeAddr = EEPROM_ITXBF_CAL+2*i;
			RT28xx_EEPROM_READ16(pAd, eeAddr, eeTmp);
			if (eeTmp != EE_Value[i])
				RT28xx_EEPROM_WRITE16(pAd, eeAddr, EE_Value[i]);
		}
	}

	/* Divider Phase parameters */
	if (divParams != NULL) {
		EE_Value[0] = divParams->gBeg[0] | (divParams->gBeg[1]<<8);
		EE_Value[1] = divParams->gEnd[0] | (divParams->gEnd[1]<<8);
		EE_Value[2] = divParams->aLow[0] | (divParams->aLow[1]<<8);
		EE_Value[3] = divParams->aMid[0] | (divParams->aMid[1]<<8);
		EE_Value[4] = divParams->aHigh[0] | (divParams->aHigh[1]<<8);

		for (i=0; i<5; i++)
		{
			eeAddr = i<3? EEPROM_ITXBF_CAL+16+2*i: EEPROM_ITXBF_CAL+38+(i-3)*2;
			RT28xx_EEPROM_READ16(pAd, eeAddr, eeTmp);
			if (eeTmp != EE_Value[i])
				RT28xx_EEPROM_WRITE16(pAd, eeAddr, EE_Value[i]);
		}
	}

	/* LNA Phase parameters */
	if (lnaParams != NULL) {
		EE_Value[0] = lnaParams->gBeg[0] | (lnaParams->gBeg[1]<<8);
		EE_Value[1] = lnaParams->gEnd[0] | (lnaParams->gEnd[1]<<8);

		EE_Value[2] = lnaParams->aLowBeg[0] | (lnaParams->aLowBeg[1]<<8);
		EE_Value[3] = lnaParams->aLowEnd[0] | (lnaParams->aLowEnd[1]<<8);
		EE_Value[4] = lnaParams->aMidBeg[0] | (lnaParams->aMidBeg[1]<<8);
		EE_Value[5] = lnaParams->aMidEnd[0] | (lnaParams->aMidEnd[1]<<8);
		EE_Value[6] = lnaParams->aHighBeg[0] | (lnaParams->aHighBeg[1]<<8);
		EE_Value[7] = lnaParams->aHighEnd[0] | (lnaParams->aHighEnd[1]<<8);

		for (i=0; i<8; i++)
		{
			eeAddr = EEPROM_ITXBF_CAL+22+2*i;
			RT28xx_EEPROM_READ16(pAd, eeAddr, eeTmp);
			if (eeTmp != EE_Value[i])
				RT28xx_EEPROM_WRITE16(pAd, eeAddr, EE_Value[i]);
		}
	}
}


/*
	ITxBFLoadLNAComp - load the LNA compensation registers
*/
VOID ITxBFLoadLNAComp(
	IN RTMP_ADAPTER *pAd)
{
	ITXBF_LNA_PARAMS lnaParams;
	UCHAR lnaValues[3];
	UCHAR bbpValue = 0;
	int i;
	UCHAR channel = pAd->CommonCfg.Channel;

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		channel = pAd->ate.Channel;
#endif /* RALINK_ATE */

	/* Get values */
	ITxBFGetEEPROM(pAd, 0, &lnaParams, 0);
	ITxBFLnaParams(lnaValues, channel, &lnaParams);

	/* Update R174 */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R27, &bbpValue);
	bbpValue &= ~0x60;

	for (i=0; i<3; i++) {
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, bbpValue | (i<<5));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R174, lnaValues[i]);
	}

	/* Enable RX Phase Compensation */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R173, &bbpValue);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R173, bbpValue | 0x20);
}


/*
	ITxBFDividerCalibration - perform divider calibration
		calFunction - the function to perform
			0=>Display cal param,
			1=>Update EEPROM
			2=>Update BBP
			3=>Just return the quantized divider phase in divPhase
			10=> Display params and dump capture data
		calMethod - the calibration method to use. 0=>use default method for the band
		divPhase - if not NULL, returns the quantized divider phase (0, +/-90, 180 for 2.4G, 0,180 for 5G)
	returns TRUE if no errors
*/
#define ITXBF_MAX_WAIT_CYCLE	10000
INT ITxBFDividerCalibration(
	IN RTMP_ADAPTER *pAd,
	IN int calFunction,
	IN int calMethod,
	OUT UCHAR *divPhase)
{
	int i;
	ITXBF_DIV_PARAMS divParams;
	UCHAR	calRefValue[2];
	UCHAR channel = pAd->CommonCfg.Channel;
	UCHAR newRefValue[2];
	LONG refValue[2];
	LONG phase[3][3];
	int avgI[3], avgQ[3];
	int peak[3][3];
	LONG d01, d21;
	int result = TRUE;

	PCAP_IQ_DATA capIqData = NULL;
	UINT32 *saveData = NULL;
	UINT32 saveSysCtrl, savePbfCfg, saveMacSysCtrl, saveDmaCtrl;

	int allocSize = (calFunction==11? MAX_CAPTURE_LENGTH: DIVCAL_CAPTURE_LENGTH)*sizeof(COMPLEX_VALUE)*3;

	UCHAR r27Value, bbpValue;
	UCHAR divPhaseValue[2];
	ITXBF_PHASE_PARAMS phaseParams;
	UCHAR phaseValues[2];

	BOOLEAN displayParams = (calFunction==0 || calFunction==10);
	BOOLEAN gBand;
#ifdef TIMESTAMP_CAL_CAPTURE1
	struct timeval tval0, tval1, tval2, tval3, tval4, tval5;
#endif
#ifdef TIMESTAMP_CAL_CAPTURE1
	do_gettimeofday(&tval0);
#endif

	r27Value = bbpValue = 0;
#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		channel = pAd->ate.Channel;
#endif /* RALINK_ATE */

	gBand = channel<=14;

	//DBGPRINT(RT_DEBUG_ERROR, ("==> ITxBFDividerCalibration cf=%d cm=%d\n", calFunction, calMethod));

	/* Handle optional divPhase parameter */
	if (divPhase == NULL)
		divPhase = divPhaseValue;

	/* If calMethod is 0 then choose default method for the band */
	if (calMethod==0)
		calMethod = channel<=14? 1: 2;

	/* Allocate buffer for capture data */
	capIqData = (PCAP_IQ_DATA) kmalloc(allocSize, MEM_ALLOC_FLAG);
	if (capIqData == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s():Alloc memory failed\n", __FUNCTION__));
		return FALSE;
	}

	/* Save MAC registers */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &saveMacSysCtrl);
	RTMP_IO_READ32(pAd, PBF_SYS_CTRL, &saveSysCtrl);
	RTMP_IO_READ32(pAd, PBF_CFG, &savePbfCfg);
	RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &saveDmaCtrl);
	
	{
		UINT32 dmaCfg, macCfg, macStatus, txrxPgcnt;
		UINT32 DTxCycle, DRxCycle, MTxCycle, MRxCycle;
		ULONG stTime, dt_time, dr_time, mt_time, mr_time;

		DTxCycle = DRxCycle = MTxCycle = MRxCycle = 0;
		RTMP_IO_READ32(pAd, 0x438, &txrxPgcnt);

		/* Disable DMA Tx and wait DMA Tx status in idle state */
		NdisGetSystemUpTime(&stTime);
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &dmaCfg);
		dmaCfg &= (~0x1);
		RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, dmaCfg);
		for (DTxCycle = 0; DTxCycle < ITXBF_MAX_WAIT_CYCLE; DTxCycle++)
		{
            RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &dmaCfg);
			if (dmaCfg & 0x2)
                RTMPusecDelay(50);
			else
				break;
		}
		NdisGetSystemUpTime(&dt_time);
		dt_time -= stTime;	
		if (DTxCycle == ITXBF_MAX_WAIT_CYCLE)
		{
			DBGPRINT(RT_DEBUG_WARN, ("%s(cnt=%d,time=0x%lx):stop DTx,dmaCfg=%d!\n",
				__FUNCTION__, DTxCycle, dt_time, dmaCfg));
		}

		/* stop PBF txQ */
		RTMP_IO_WRITE32(pAd, PBF_CFG, (savePbfCfg & (~0x14)));

		/* Disable MAC Tx and MAC Rx and wait MAC Tx/Rx status in idle state */
		/* MAC Tx */
		NdisGetSystemUpTime(&stTime);
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &macCfg);
		macCfg &= (~0x04);
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, macCfg);
		for (MTxCycle = 0; MTxCycle < ITXBF_MAX_WAIT_CYCLE; MTxCycle++)
		{
			RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &macStatus);
			if (macStatus & 0x1)
                RTMPusecDelay(50);
			else
				break;
		}
		NdisGetSystemUpTime(&mt_time);
		mt_time -= stTime;
		if (MTxCycle == ITXBF_MAX_WAIT_CYCLE)
		{
			DBGPRINT(RT_DEBUG_WARN, ("%s(cnt=%d,time=0x%lx):stop MTx,macStatus=0x%x!\n", 
				__FUNCTION__, MTxCycle, mt_time, macStatus));
		}
		
		/* MAC Rx */
		NdisGetSystemUpTime(&stTime);
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &macCfg);
		macCfg &= (~0x08);
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, macCfg);
		for (MRxCycle = 0; MRxCycle < ITXBF_MAX_WAIT_CYCLE; MRxCycle++)
		{
			RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &macStatus);
			if (macStatus & 0x2)
				RTMPusecDelay(50);
			else
				break;
		}
		NdisGetSystemUpTime(&mr_time);
		mr_time -= stTime;
		if (MRxCycle == ITXBF_MAX_WAIT_CYCLE)
		{
			DBGPRINT(RT_DEBUG_WARN, ("%s(cnt=%d,time=0x%lx):stop MRx, macStatus=%d!\n",
				__FUNCTION__, MRxCycle, mr_time, macStatus));
		}

		/* stop PBF rxQ */
		RTMP_IO_WRITE32(pAd, PBF_CFG, (savePbfCfg & (~0x1e)));


		/* Disable DMA Rx */
		NdisGetSystemUpTime(&stTime);
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &dmaCfg);
		dmaCfg &= (~0x4);
		RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, dmaCfg);
		for (DRxCycle = 0; DRxCycle < ITXBF_MAX_WAIT_CYCLE; DRxCycle++)
		{
			RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &dmaCfg);
			if (dmaCfg & 0x8)
				RTMPusecDelay(50);
			else
				break;
		}
		NdisGetSystemUpTime(&dr_time);
		dr_time -= stTime;
		if (DRxCycle == ITXBF_MAX_WAIT_CYCLE)
		{
			DBGPRINT(RT_DEBUG_WARN, ("%s(cnt=%d,time=0x%lx):stop DRx, dmaCfg=%d!\n",
					__FUNCTION__, DRxCycle, dr_time, dmaCfg));
		}

        /* Check status */
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &dmaCfg);
		RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &macStatus);
		if ((dmaCfg & 0xa) || (macStatus & 0x3) || 
			(dt_time > 50) || (dr_time > 50) || (mr_time> 50) || (mt_time > 50))
		{
			UINT32 txrxPgcnt2;

			RTMP_IO_READ32(pAd, 0x438, &txrxPgcnt2);
			
			DBGPRINT(RT_DEBUG_WARN, ("%s():After Disable DMA/MAC Tx/Rx, dmaBusy=%d,macBusy=%d!\n",
                                __FUNCTION__, (dmaCfg & 0xa), (macStatus & 0x3)));
			DBGPRINT(RT_DEBUG_WARN, ("%s():DMA=>Tx(time:cycle)=(0x%lx:%d), Rx(time:cycle)=(0x%lx:%d)!\n",
					__FUNCTION__, dt_time, DTxCycle, dr_time, DRxCycle));
			DBGPRINT(RT_DEBUG_WARN, ("%s():MAC=>Tx(time:cycle)=(0x%lx:%d), Rx(time:cycle)=(0x%lx:%d)!\n", 
					__FUNCTION__, mt_time, MTxCycle, mr_time, MRxCycle));
			DBGPRINT(RT_DEBUG_WARN, ("%s():PBF=>Original:0x%x, now:0x%x!\n", 
					__FUNCTION__, txrxPgcnt, txrxPgcnt2));
		}
	}	

	/* Save MAC data */
	saveData = ITxBFSaveData(pAd);
	if (saveData == NULL) {
		os_free_mem(pAd, capIqData);

		/* Restore MAC/DMA cfg register to HW */
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, saveMacSysCtrl);
		RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, saveDmaCtrl);
		return FALSE;
	}

	/* Handle special mode */
	if (calFunction == 11)
	{
		/* Special mode - Capture all antennas without doing RF Calibration loopback. Leave LNA/VGA unchanged */
		DoCalibrationCapture(pAd, -1, NULL, NULL, 0, 0, 0);
		ReadCaptureData(pAd, capIqData, MAX_CAPTURE_LENGTH);
		DisplayCaptureData(capIqData, MAX_CAPTURE_LENGTH);
		goto exitDivCal;
	}

	/* Normal Divider Calibration mode */
	
	/* Read Divider calibration values from EEPROM */
	ITxBFGetEEPROM(pAd, 0, 0, &divParams);
	ITxBFDivParams(calRefValue, channel, &divParams);
	refValue[0] = calRefValue[0]<<BYTE_PHASE_SHIFT;
	refValue[1] = calRefValue[1]<<BYTE_PHASE_SHIFT;

#ifdef TIMESTAMP_CAL_CAPTURE1
	do_gettimeofday(&tval1);
#endif

	/* Do Calibration */
	switch (calMethod) {
	case 1:
	case 4:
		/* Method 1 or 4, G-Band */
		/* RF calibration with all TX chains active, followed by RF cal with Tx0 */
		CaptureRFCal(pAd, capIqData, -1);
#ifdef TIMESTAMP_CAL_CAPTURE1
		do_gettimeofday(&tval2);
#endif
		CalcRFCalPhase(phase[0], avgI, avgQ, peak[0], capIqData, TRUE, 0);

		CaptureRFCal(pAd, capIqData, 0);
		CalcRFCalPhase(phase[1], avgI, avgQ, peak[1], capIqData, FALSE, 0);

		/* Method 4 => do a second Tx0 capture and average it */
		if (calMethod==4) {
			LONG phase1[3];
			int avgI1[3], avgQ1[3];
			int peak1[3];

			CaptureRFCal(pAd, capIqData, 0);
			CalcRFCalPhase(phase1, avgI1, avgQ1, peak1, capIqData, FALSE, 0);

			for (i=0; i<3; i++)
				phase[1][i] = iAtan2(avgQ1[i]+avgQ[i], avgI1[i]+avgI[i]);

			/* Display parameters */
			if (displayParams) {
				DBGPRINT(RT_DEBUG_OFF, ("Method #4 Second cap\n"
							 "Phase = [%d %d %d] deg\n"
							 "Peak  = [%d, %d, %d]\n",
							DEG(phase1[0]), DEG(phase1[1]), DEG(phase1[2]),
							peak1[0], peak1[1], peak1[2]));
			}
		}
#ifdef TIMESTAMP_CAL_CAPTURE1
		do_gettimeofday(&tval3);
#endif

		/* Calculate difference */
		d01 = phase[0][0] - 2*(phase[1][0]-phase[1][1]) - refValue[0];
		d21 = phase[0][2] - 2*(phase[1][2]-phase[1][1]) - refValue[1];

		/* Display parameters */
		if (displayParams) {
			DBGPRINT(RT_DEBUG_OFF, ("Method #%d\nPhase0 = [%d %d] deg\n"
						 "Phase1 = [%d %d %d] deg\n"
						 "Peak  = [%d, %d, %d] [%d, %d, %d]\n"
						 "Delta = [%d, %d] deg\n",
						calMethod, DEG(phase[0][0]), DEG(phase[0][2]),
						DEG(phase[1][0]), DEG(phase[1][1]), DEG(phase[1][2]),
						peak[0][0], peak[0][1], peak[0][2], peak[1][0], peak[1][1], peak[1][2],
						DEG(d01), DEG(d21)) );
		}
		break;

	case 2:
		/* Method 2, A-Band */
		/* A single RF calibration with all TX chains active */
		CaptureRFCal(pAd, capIqData, -1);
#ifdef TIMESTAMP_CAL_CAPTURE1
		do_gettimeofday(&tval2);
#endif
		CalcRFCalPhase(phase[0], avgI, avgQ, peak[0], capIqData, TRUE, 0);
#ifdef TIMESTAMP_CAL_CAPTURE1
		do_gettimeofday(&tval3);
#endif

		/* Calculate difference */
		d01 = phase[0][0] - refValue[0];
		d21 = phase[0][2] - refValue[1];

		/* Display parameters */
		if (displayParams) {
			DBGPRINT(RT_DEBUG_OFF, ("Method #2\nPhase = [%d, %d] deg\n"
						 "Peak  = [%d, %d, %d]\n"
						 "Delta = [%d, %d] deg\n",
						DEG(phase[0][0]), DEG(phase[0][2]),
						peak[0][0], peak[0][1], peak[0][2],
						DEG(d01), DEG(d21)) );
		}
		break;

	default:
		/* Method 3. Optional A-Band or G-Band. RF Cal with each TX active */
		for (i=0; i<3; i++) {
			CaptureRFCal(pAd, capIqData, i);
			CalcRFCalPhase(phase[i], avgI, avgQ, peak[i], capIqData, FALSE, i);
		}

		/* Calculate difference */
		d01 = (phase[0][1]-phase[1][0]) - refValue[0];
		d21 = (phase[2][1]-phase[1][2]) - refValue[1];

		/* Display parameters */
		if (displayParams) {
			DBGPRINT(RT_DEBUG_OFF, ("Method #3\n"
					 "Peak = [%d %d %d]  [%d %d %d]  [%d %d %d]\n"
					 "Delta = [%d %d]\n",
					peak[0][0], peak[0][1], peak[0][2],
					peak[1][0], peak[1][1], peak[1][2],
						peak[2][0], peak[2][1], peak[2][2],
					DEG(d01), DEG(d21)) );
		}
		break;
	}

	/* Compute the quantized delta phase */
	if (calFunction==1)
	{
		/* Phase relative to EEPROM is 0 */
		divPhase[0] = divPhase[1] = 0;
	}
	else {
		divPhase[0] = CONVERT_TO_BYTE_PHASE(d01);
		divPhase[1] = CONVERT_TO_BYTE_PHASE(d21);

		/* Quantize to 90 deg (0x40) or 180 deg (0x80) with rounding */
		if (gBand)
			divPhase[0] = (divPhase[0] + 0x20) & 0xC0;
		else
			divPhase[0] = (divPhase[0] + 0x40) & 0x80;

		if (gBand)
			divPhase[1] = (divPhase[1] + 0x20) & 0xC0;
		else
			divPhase[1] = (divPhase[1] + 0x40) & 0x80;
	}

	/* Either display parameters, update EEPROM, update BBP registers or dump capture data */
	switch (calFunction) {
	case 0:
		break;

	case 1:
		/*
			Save new reference values in EEPROM. The new reference is just the delta phase
			values with the old ref value added back in
		*/
		newRefValue[0] = CONVERT_TO_BYTE_PHASE(refValue[0] + d01);
		newRefValue[1] = CONVERT_TO_BYTE_PHASE(refValue[1] + d21);

		/* Only allow calibration on specific channels */
		if (channel == 1) {
			divParams.gBeg[0] = newRefValue[0];
			divParams.gBeg[1] = newRefValue[1];
		}
		else if (channel == 14) {
			divParams.gEnd[0] = newRefValue[0];
			divParams.gEnd[1] = newRefValue[1];
		}
		else if (channel == 36) {
			divParams.aLow[0] = newRefValue[0];
			divParams.aLow[1] = newRefValue[1];
		}
		else if (channel == 120) {
			/*
				Remove any 180 phase shift relative to Ch36. That will allow older FW to 
				use DivCal parameters calibrated using newer FW. If DivCal reference
				is shifted relative to Ch36 then Phase Calibration parameters will
				have a 180 phse shift. Older FW will use only Ch36 DivCal parameter.
			*/
			UCHAR deltaPhase;

			deltaPhase = newRefValue[0] - divParams.aLow[0];
			if (deltaPhase>0x40 && deltaPhase<0xC0)
				newRefValue[0] -= 0x80;
			deltaPhase = newRefValue[1] - divParams.aLow[1];
			if (deltaPhase>0x40 && deltaPhase<0xC0)
				newRefValue[1] -= 0x80;

			divParams.aMid[0] = newRefValue[0];
			divParams.aMid[1] = newRefValue[1];
		}
		else if (channel == 165) { 
			/*
				Remove any 180 phase shift relative to Ch116. Needed if we
				interpolate between Ch120 and Ch165.
			*/
			UCHAR deltaPhase;

			deltaPhase = newRefValue[0] - divParams.aMid[0];
			if (deltaPhase>0x40 && deltaPhase<0xC0)
				newRefValue[0] -= 0x80;
			deltaPhase = newRefValue[1] - divParams.aMid[1];
			if (deltaPhase>0x40 && deltaPhase<0xC0)
				newRefValue[1] -= 0x80;

			divParams.aHigh[0] = newRefValue[0];
			divParams.aHigh[1] = newRefValue[1];
		}
		else {
			DBGPRINT(RT_DEBUG_ERROR, ("Invalid channel: %d\nMust calibrate channel 1, 14, 36, 120 or 165", channel) );
			result = FALSE;
			goto exitDivCal;
		}

		ITxBFSetEEPROM(pAd, 0, 0, &divParams);
		break;

	case 2:
		/*
			Update BBP Registers. Quantize DeltaPhase to 90 or 180 depending on band. Then
			update original phase calibration values from EEPROM and set R176 for Ant 0 and Ant2
		*/
		ITxBFGetEEPROM(pAd, &phaseParams, 0, 0);
		ITxBFPhaseParams(phaseValues, channel, &phaseParams);

		/* Ant0 */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R27, &r27Value);
		r27Value &= ~0x60;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, r27Value);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R176, phaseValues[0]+divPhase[0]);

		/* Ant2 */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, r27Value | 0x40);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R176, phaseValues[1]+divPhase[1]);

		/* Enable TX Phase Compensation */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R173, &bbpValue);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R173, bbpValue | 0x08);
		break;

	case 3:
		/* Just return the Divider Phase */
		break;

	case 10:
		/* Dump capture data */
		DisplayCaptureData(capIqData, DIVCAL_CAPTURE_LENGTH);
		break;

	default:
		result = FALSE;
		break;
	}

exitDivCal:
	/*	Return to normal mode */
	RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, saveSysCtrl);

	/* Restore MAC data */
	ITxBFRestoreData(pAd, saveData);

#ifdef TIMESTAMP_CAL_CAPTURE1
	do_gettimeofday(&tval4);
#endif

	/* Restore registers */
	/*	reset packet buffer */
	/* RTMP_IO_WRITE32(pAd, PBF_CTRL, 0x00000020); */
	
	/*	enable Tx/Rx Queue */
	RTMP_IO_WRITE32(pAd, PBF_CFG, savePbfCfg);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, saveMacSysCtrl);
	RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, saveDmaCtrl);

	/* Free data */
	if (saveData != NULL)
		os_free_mem(pAd, saveData);
	if (capIqData != NULL)
		os_free_mem(pAd, capIqData);

#ifdef TIMESTAMP_CAL_CAPTURE1
	do_gettimeofday(&tval5);
	
	DBGPRINT(RT_DEBUG_ERROR, ("%s t1=%ld t2=%ld t3=%ld t4=%ld t5=%ld\n", __FUNCTION__,
			tval1.tv_usec - tval0.tv_usec, tval2.tv_usec - tval0.tv_usec,
			tval3.tv_usec - tval0.tv_usec, tval4.tv_usec - tval0.tv_usec,
			tval5.tv_usec - tval0.tv_usec));
#endif

	return result;
}

#define MAX_LNA_CAPS	10		/* Maximum number of LNA captures per calibration */


/*
	ITxBFOldLNACalCapLoop - do the old LNA capture loop (method 2). All Tx are active.
		phase - returns capture phase results for three chains. One row for each capture.
		peak - returns peak value results for three chains. One row for each capture.
		capIqData - buffer to hold one capture
		lnaValues - table of lnaValues for each chain
		capCount - number of captures to do
		displayParams - flag to enable display of intermediate results
		calMethod - calibration method (used for debug display)
*/
static void ITxBFOldLNACalCapLoop(
	IN PRTMP_ADAPTER pAd,
	IN LONG phase[][3],
	IN int peak[][3],
	IN PCAP_IQ_DATA capIqData,
	IN UCHAR lnaValues[][3],
	IN int capCount,
	IN BOOLEAN displayParams,
	IN int calMethod)
{
	int i;
	int avgI[MAX_LNA_CAPS][3], avgQ[MAX_LNA_CAPS][3];
	UCHAR channel = pAd->CommonCfg.Channel;

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		channel = pAd->ate.Channel;
#endif /* RALINK_ATE */

	/* Do the LNA capture loop */
	for (i=0; i<capCount; i++) {
		DoCalibrationCapture(pAd, -1, NULL, lnaValues[i], channel<=14? 0x0e: 0x12, 0xC001, 0x50);
		ReadCaptureData(pAd, capIqData, DIVCAL_CAPTURE_LENGTH);
		CalcRFCalPhase(phase[i], avgI[i], avgQ[i], peak[i], capIqData, FALSE, 0);

		/* Make phase relative to Ant1 */
		phase[i][0] -= phase[i][1];
		phase[i][2] -= phase[i][1];
		phase[i][1] = 0;
	}
}


/*
	ITxBFDiffLNACalCapLoop - do the Differential LNA capture loop.
	For each LNA configuration it does 4x captures with inverted phase and combines results
		phase - returns phase results for three chains. One row for each capture.
		peak - returns peak value results for three chains. One row for each capture.
		capIqData - buffer to hold one capture
		lnaValues - table of lnaValues for each chain
		capCount - number of capture results to return
		displayParams - flag to enable display of intermediate results
		calMethod - calibration method (used for debug display)
*/
static void ITxBFDiffLNACalCapLoop(
	IN PRTMP_ADAPTER pAd,
	IN LONG phase[][3],
	IN int peak[][3],
	IN PCAP_IQ_DATA capIqData,
	IN UCHAR lnaValues[][3],
	IN int capCount,
	IN BOOLEAN displayParams,
	IN int calMethod)
{
	int i, j;
	USHORT papdIQ[4][3] = {{0x7F00,0x7F00,0}, {0x8000,0x7F00,0}, {0,0x7F00,0x7F00}, {0,0x7F00,0x8000}};
	/* USHORT papdIQ[4][3] = {{0x3F00,0x3F00,0}, {0xC000,0x3F00,0}, {0,0x3F00,0x3F00}, {0,0x3F00,0xC000}}; */
	LONG tempPhase[3];
	int avgI[4][3], avgQ[4][3];
	int tempPeak[4][3];
	UCHAR channel = pAd->CommonCfg.Channel;

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		channel = pAd->ate.Channel;
#endif /* RALINK_ATE */

	for (i=0; i<capCount; i++) {
		/* Do four actual captures */
		for (j=0; j<4; j++) {
			DoCalibrationCapture(pAd, -1, papdIQ[i], lnaValues[i], channel<=14? 0x0e: 0x12, 0xC001, 0x50);
			ReadCaptureData(pAd, capIqData, DIVCAL_CAPTURE_LENGTH);
			CalcRFCalPhase(tempPhase, avgI[j], avgQ[j], tempPeak[j], capIqData, FALSE, 0);
		}

		/* Combine results from four captures */
		avgI[0][0] -= avgI[1][0];
		avgQ[0][0] -= avgQ[1][0];
		avgI[0][1] += avgI[1][1];
		avgQ[0][1] += avgQ[1][1];

		avgI[2][1] += avgI[3][1];
		avgQ[2][1] += avgQ[3][1];
		avgI[2][2] -= avgI[3][2];
		avgQ[2][2] -= avgQ[3][2];

		/* Compute relative phase and peak */
		phase[i][0] = iAtan2(avgQ[0][0], avgI[0][0]) - iAtan2(avgQ[0][1], avgI[0][1]);
		phase[i][1] = 0;
		phase[i][2] = iAtan2(avgQ[2][2], avgI[2][2]) - iAtan2(avgQ[2][1], avgI[2][1]);

		peak[i][0] = tempPeak[0][0]>tempPeak[1][0]? tempPeak[0][0]: tempPeak[1][0];
		peak[i][1] = tempPeak[0][1]>tempPeak[1][1]? tempPeak[0][1]: tempPeak[1][1];
		peak[i][2] = tempPeak[2][2]>tempPeak[3][2]? tempPeak[2][2]: tempPeak[3][2];
	}
	if (displayParams) {
		/* static char *lnaString[5] = {"M/M/M", "L /M/L ", "H/M/H", "L /L /L ", "H/H/H"}; */

		DBGPRINT(RT_DEBUG_WARN, ("Diff LNA Method #%d\n", calMethod));

		for (i=0; i<capCount; i++) {
			DBGPRINT(RT_DEBUG_WARN,
					("%c-Phase = [%d __ %d] deg,"
					" Peak = [%d %d %d]\n",
					i+'A', DEG(phase[i][0]), DEG(phase[i][2]),
					peak[i][0], peak[i][1], peak[i][2]) );
		}
	}
}


/*
	ITxBFLNACalCapLoop - do the LNA capture loop. TX are selectively enabled
		phase - returns capture phase results for three chains. One row for each capture.
		peak - returns peak value results for three chains. One row for each capture.
		capIqData - buffer to hold one capture
		papdIQ - PAPD IQ values for each chain. One row per capture
		lnaValues - table of lnaValues for each chain. One row per capture
		capCount - number of captures to do
		displayParams - flag to enable display of intermediate results
		calMethod - calibration method (used for debug display)
*/
void ITxBFLNACalCapLoop(
	IN PRTMP_ADAPTER pAd,
	IN LONG phase[][3],
	IN int peak[][3],
	IN PCAP_IQ_DATA capIqData,
	IN USHORT papdIQ[][3],
	IN UCHAR lnaValues[][3],
	IN int capCount,
	IN BOOLEAN displayParams,
	IN int calMethod)
{
	int i;
	int avgI[MAX_LNA_CAPS][3], avgQ[MAX_LNA_CAPS][3];
	UCHAR channel = pAd->CommonCfg.Channel;

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		channel = pAd->ate.Channel;
#endif /* RALINK_ATE */

	for (i=0; i<capCount; i++) {
		DoCalibrationCapture(pAd, -1, papdIQ[i], lnaValues[i], channel<=14? 0x0e: 0x12, 0xC001, 0x50);
		ReadCaptureData(pAd, capIqData, DIVCAL_CAPTURE_LENGTH);
		CalcRFCalPhase(phase[i], avgI[i], avgQ[i], peak[i], capIqData, FALSE, 0);

		/* Make phase relative to Ant1 */
		phase[i][0] -= phase[i][1];
		phase[i][2] -= phase[i][1];
		phase[i][1] = 0;
	}
	/* Display intermediate results */
	if (displayParams) {
		static char *lnaString[9] = {"M/m/m", "L/m/m", "H/m/m",
							  		"m/M/m", "m/L/m", "m/H/m",
							  		"m/m/M", "m/m/L", "m/m/H"};

		DBGPRINT(RT_DEBUG_WARN, ("LNA Method #%d\n", calMethod));

		for (i=0; i<capCount; i++) {
			DBGPRINT(RT_DEBUG_WARN,
					("%c-Phase (%s)=[%d __ %d] deg,"
					" E=[(%d,%d) (%d,%d) (%d,%d)],"
					" Peak=[%d %d %d]\n",
					i+'A', lnaString[i], DEG(phase[i][0]), DEG(phase[i][2]),
					avgI[i][0]/256, avgQ[i][0]/256, avgI[i][1]/256,
					avgQ[i][1]/256, avgI[i][2]/256, avgQ[i][2]/256,
					peak[i][0], peak[i][1], peak[i][2]) );
		}
	}
}


/*
	ITxBFLNACalibration - perform divider calibration
		calFunction - the function to perform
						0=>Display cal param,
						1=>Update EEPROM and BBP,
						2=>Update BBP only,
						10=>Display and dump data
		calMethod - the calibration method to use (0=default, 1=simple, 2=old, 3=diff, 4=oneTx)
		gBand - specifies G band or A band
	returns TRUE if no errors
*/
int ITxBFLNACalibration(
	IN RTMP_ADAPTER *pAd,
	IN int calFunction,
	IN int calMethod,
	IN BOOLEAN gBand)
{
	PCAP_IQ_DATA capIqData;
	UINT32 *saveData, saveSysCtrl, savePbfCfg, saveMacSysCtrl;

	LONG phase[2];
	LONG mPhase[MAX_LNA_CAPS];
	LONG cap[MAX_LNA_CAPS][3];

	static UCHAR lnaValuesOneTx[9][3] = {
		{R65_LNA_MID,  R65_LNA_MID,  R65_LNA_MID},
		{R65_LNA_LOW,  R65_LNA_MID,  R65_LNA_MID},
		{R65_LNA_HIGH, R65_LNA_MID,  R65_LNA_MID},
		{R65_LNA_MID,  R65_LNA_MID,  R65_LNA_MID},
		{R65_LNA_MID,  R65_LNA_LOW,  R65_LNA_MID},
		{R65_LNA_MID,  R65_LNA_HIGH, R65_LNA_MID},
		{R65_LNA_MID,  R65_LNA_MID,  R65_LNA_MID},
		{R65_LNA_MID,  R65_LNA_MID,  R65_LNA_LOW},
		{R65_LNA_MID,  R65_LNA_MID,  R65_LNA_HIGH}};
	static USHORT papdIQOneTx[9][3] = {
		{0x7f00, 0x0000, 0x0000},
		{0x7f00, 0x0000, 0x0000},
		{0x7f00, 0x0000, 0x0000},
		{0x0000, 0x7f00, 0x0000},
		{0x0000, 0x7f00, 0x0000},
		{0x0000, 0x7f00, 0x0000},
		{0x0000, 0x0000, 0x7f00},
		{0x0000, 0x0000, 0x7f00},
		{0x0000, 0x0000, 0x7f00}};

	static UCHAR lnaValues5[5][3] = {
		{R65_LNA_MID,  R65_LNA_MID,  R65_LNA_MID},
		{R65_LNA_LOW,  R65_LNA_MID,  R65_LNA_LOW},
		{R65_LNA_HIGH, R65_LNA_MID,  R65_LNA_HIGH},
		{R65_LNA_LOW,  R65_LNA_LOW,  R65_LNA_LOW},
		{R65_LNA_HIGH, R65_LNA_HIGH, R65_LNA_HIGH}};
	static UCHAR lnaValues2[2][3] = {
		{R65_LNA_MID,  R65_LNA_MID,  R65_LNA_MID},
		{R65_LNA_LOW,  R65_LNA_MID,  R65_LNA_HIGH}};

	int peak[MAX_LNA_CAPS][3];
	int txAntennas = pAd->Antenna.field.TxPath;		/* # of Tx Antennas */
	UCHAR channel = pAd->CommonCfg.Channel;

	ITXBF_LNA_PARAMS lnaParams;
	UCHAR quantPhase[3], hexPhaseValues[2];
	UCHAR bbpValue = 0;
	BOOLEAN displayParams = (calFunction==0 || calFunction==10);
	int result = TRUE;
	int i;
#ifdef TIMESTAMP_CAL_CAPTURE1
	struct timeval tval0, tval1, tval2, tval3, tval4;
#endif

#ifdef TIMESTAMP_CAL_CAPTURE1
	do_gettimeofday(&tval0);
#endif

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		channel = pAd->ate.Channel;
#endif /* RALINK_ATE */

	/* Default is Method 4 */
	if (calMethod <= 0)
		calMethod = 4;

	/* Allocate buffer for capture data */
	capIqData = (PCAP_IQ_DATA) kmalloc(DIVCAL_CAPTURE_LENGTH*sizeof(COMPLEX_VALUE)*3, MEM_ALLOC_FLAG);
	if (capIqData == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s():Alloc memory failed\n", __FUNCTION__));
		return FALSE;
	}

	/* Save MAC data */
	saveData = ITxBFSaveData(pAd);
	if (saveData == NULL) {
		os_free_mem(pAd, capIqData);
		return FALSE;
	}

	/* Save MAC registers */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &saveMacSysCtrl);
	RTMP_IO_READ32(pAd, PBF_SYS_CTRL, &saveSysCtrl);
	RTMP_IO_READ32(pAd, PBF_CFG, &savePbfCfg);

	/* Do LNA Calibration. */
	switch (calMethod) {
	case 1:
		/* Method 1. Two Measurements. */
		ITxBFOldLNACalCapLoop(pAd, cap, peak, capIqData, lnaValues2, 2, displayParams, calMethod);

		/* Compute LNA Compensation parameters */
		phase[0] = cap[0][0]-cap[1][0];						/* A0-B0 = M-L */
		phase[1] = cap[0][2]-cap[1][2];						/* A2-B2 = M-H */
		break;

	case 2:
	case 3:
		/* Method 2. Five Measurements. */
		if (calMethod == 3)
			ITxBFDiffLNACalCapLoop(pAd, cap, peak, capIqData, lnaValues5, 5, displayParams, calMethod);
		else
			ITxBFOldLNACalCapLoop(pAd, cap, peak, capIqData, lnaValues5, 5, displayParams, calMethod);

		if (txAntennas == 2) {
			for (i=0; i<5; i++)
				cap[i][2] = 0;
		}

		/* Derive LNA Compensation values by averaging M-L and M-H for each chain. */
		mPhase[0] = radMod2pi(-cap[1][0]+cap[0][0]);	 /* Mid-low */
		mPhase[1] = radMod2pi(cap[3][0]-cap[1][0]);
		mPhase[2] = radMod2pi(-cap[1][2]+cap[0][2]);
		mPhase[3] = radMod2pi(-cap[2][0]+cap[0][0]);	 /* Mid-High */
		mPhase[4] = radMod2pi(cap[4][0]-cap[2][0]);
		mPhase[5] = radMod2pi(-cap[2][2]+cap[0][2]);

		phase[0] = avgPhase(mPhase, txAntennas);
		phase[1] = avgPhase(mPhase+3, txAntennas);
		/* Display table of LNA phase vs Ant */
		if (displayParams) {
			DBGPRINT(RT_DEBUG_WARN, (
				"Phase vs Ant (deg) Ch%02d\n"
				"  L  [%1d  %1d/%1d  %1d] = %1d\n"
				"  H [%1d  %1d/%1d  %1d] = %1d\n",
				channel,
				DEG180(mPhase[0]), DEG180(mPhase[1]), DEG180(cap[3][2]-cap[1][2]),
				DEG180(mPhase[2]), DEG180(phase[0]),
				DEG180(mPhase[3]), DEG180(mPhase[4]), DEG180(cap[4][2]-cap[2][2]),
				DEG180(mPhase[5]), DEG180(phase[1]) ) );
		}
		break;

	default:
		/* Method 4. Nine captures with only one Tx enabled for each capture */
		ITxBFLNACalCapLoop(pAd, cap, peak, capIqData, papdIQOneTx, lnaValuesOneTx, txAntennas==2? 6: 9, displayParams, calMethod);

		if (txAntennas == 2) {
			for (i=0; i<9; i++) {
				if (i >= 6)
					cap[i][0] = cap[i][1] = 0;
				cap[i][2] = 0;
			}
		}

		/* Calculate M-L and M-H for each chain */
		/* Mid-Low. */
		mPhase[0] = radMod2pi(cap[0][0]-cap[1][0]);
		mPhase[1] = radMod2pi(cap[4][0]-cap[3][0]);
		if (txAntennas == 3) {
			mPhase[2] = radMod2pi(cap[4][2]-cap[3][2]);
			mPhase[1] = avgPhase(mPhase+1, 2);
			}
		mPhase[2] = radMod2pi(cap[6][2]-cap[7][2]);

		/* High-Mid */
		mPhase[3] = radMod2pi(cap[0][0]-cap[2][0]);
		mPhase[4] = radMod2pi(cap[5][0]-cap[3][0]);
		if (txAntennas == 3) {
			mPhase[5] = radMod2pi(cap[5][2]-cap[3][2]);
			mPhase[4] = avgPhase(mPhase+4, 2);
		}
		mPhase[5] = radMod2pi(cap[6][2]-cap[8][2]);

		/* Derive LNA Compensation values by averaging M-L and M-H for each chain. */
		phase[0] = avgPhase(mPhase, txAntennas);
		phase[1] = avgPhase(mPhase+3, txAntennas);

		if (displayParams) {
			DBGPRINT(RT_DEBUG_WARN, (
					"Phase vs Ant (deg) Ch%02d\n"
					"  L : (a-b/e-d/g-h) [%1d, %1d/%1d, %1d] = %1d\n"
					"  H: (a-c/f-d/g-i) [%1d, %1d/%1d, %1d] = %1d\n\n",
					channel, DEG180(mPhase[0]),
					DEG180(cap[4][0]-cap[3][0]), DEG180(cap[4][2]-cap[3][2]),
					DEG180(mPhase[2]), DEG180(phase[0]),
					DEG180(mPhase[3]),
					DEG180(cap[5][0]-cap[3][0]), DEG180(cap[5][2]-cap[3][2]),
					DEG180(mPhase[5]), DEG180(phase[1]) ) );
		}
		break;
	}

	/* Calculate hex phase correction */
	hexPhaseValues[0] = CONVERT_TO_BYTE_PHASE(phase[0]);	/* M-L */
	hexPhaseValues[1] = -CONVERT_TO_BYTE_PHASE(phase[1]);	/* H-M */

	/* Calulate LNA compensation and quantize to 4 bits */
	quantPhase[0] = hexPhaseValues[0];						/* M-L */
	quantPhase[1] = hexPhaseValues[1]-hexPhaseValues[0];	/* H-L */
	quantPhase[2] = hexPhaseValues[1];						/* H-M */

	for (i=0; i<3; i++)
		quantPhase[i] = (quantPhase[i] + 0x8) & 0xF0;

	/* Either display parameters, update EEPROM and BBP registers or dump capture data */
	switch (calFunction) {
	case 0:
		DBGPRINT(RT_DEBUG_WARN, ("M-L/H-L/H-M:  [%d %d %d] deg = [0x%02X  0x%02X]\n"
					"R174:  [%d %d %d] deg = [%02x  %02x  %02x]\n",
					DEG(phase[0]), DEG(phase[0]+phase[1]), DEG(phase[1]),
					hexPhaseValues[0], hexPhaseValues[1],
					quantPhase[0]*360/256, quantPhase[1]*360/256, quantPhase[2]*360/256,
					quantPhase[0], quantPhase[1], quantPhase[2]) );
		break;

	case 1:
		/* Save new reference values in EEPROM and BBP */
		ITxBFGetEEPROM(pAd, 0, &lnaParams, 0);

		/* Only allow calibration on specific channels */
		if (channel == 1) {
			lnaParams.gBeg[0] = hexPhaseValues[0];
			lnaParams.gBeg[1] = hexPhaseValues[1];
		}
		else if (channel == 14) {
			lnaParams.gEnd[0] = hexPhaseValues[0];
			lnaParams.gEnd[1] = hexPhaseValues[1];
		}
		else if (channel == 36) {
			lnaParams.aLowBeg[0] = hexPhaseValues[0];
			lnaParams.aLowBeg[1] = hexPhaseValues[1];
		}
		else if (channel == 64) {
			lnaParams.aLowEnd[0] = hexPhaseValues[0];
			lnaParams.aLowEnd[1] = hexPhaseValues[1];
		}
		else if (channel == 100) {
			lnaParams.aMidBeg[0] = hexPhaseValues[0];
			lnaParams.aMidBeg[1] = hexPhaseValues[1];
		}
		else if (channel == 128) {
			lnaParams.aMidEnd[0] = hexPhaseValues[0];
			lnaParams.aMidEnd[1] = hexPhaseValues[1];
		}
		else if (channel == 132) {
			lnaParams.aHighBeg[0] = hexPhaseValues[0];
			lnaParams.aHighBeg[1] = hexPhaseValues[1];
		}
		else if (channel == 165) {
			lnaParams.aHighEnd[0] = hexPhaseValues[0];
	 		lnaParams.aHighEnd[1] = hexPhaseValues[1];
		}
		else {
			DBGPRINT(RT_DEBUG_OFF,
					("Invalid channel: %d\nMust calibrate channel 1, 14, 36, 64, 100, 128, 132 or 165", channel) );
			result = FALSE;
			goto exitLnaCal;
		}

		ITxBFSetEEPROM(pAd, 0, &lnaParams, 0);
		/* FALL THROUGH to update BBP */
	case 2:
		/* Update BBP */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R27, &bbpValue);
		bbpValue &= ~0x60;

		/* Update R174 registers */
		for (i=0; i<3; i++) {
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, bbpValue | (i<<5));
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R174, quantPhase[i]);
		}
		break;

	case 10:
		/* Dump capture data */
		DisplayCaptureData(capIqData, DIVCAL_CAPTURE_LENGTH);
		break;

	default:
		result = FALSE;
		break;
	}

exitLnaCal:
	/*	Return to normal mode */
	RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, saveSysCtrl);

	/* Restore MAC data */
	ITxBFRestoreData(pAd, saveData);
#ifdef TIMESTAMP_CAL_CAPTURE1
	do_gettimeofday(&tval3);
#endif
	/* Restore registers */
	/*	reset packet buffer */
	RTMP_IO_WRITE32(pAd, PBF_CTRL, 0x00000020);

	/*	enable Tx/Rx Queue */
	RTMP_IO_WRITE32(pAd, PBF_CFG, savePbfCfg);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, saveMacSysCtrl);

#ifdef TIMESTAMP_CAL_CAPTURE1
	do_gettimeofday(&tval4);

	printk("%s t1=%ld t2=%ld t3=%ld t4=%ld\n", __FUNCTION__,
			tval1.tv_usec - tval0.tv_usec, tval2.tv_usec - tval0.tv_usec,
			tval3.tv_usec - tval0.tv_usec, tval4.tv_usec - tval0.tv_usec);
#endif

	/* Free data */
	os_free_mem(pAd, saveData);
	os_free_mem(pAd, capIqData);

	return result;
}


/* ------------ BEAMFORMING PROFILE HANDLING ------------ */
static SC_TABLE_ENTRY impSubCarrierTable[2] = { {36, 63, 1, 28}, {70, 126, 2, 58} };
static SC_TABLE_ENTRY expSubCarrierTable[2] = { {100, 127, 1, 28}, {70, 126, 2, 58} };

PROFILE_DATA profData;

/* Read_TagField - read a profile tagfield */
void Read_TagField(
	IN	PRTMP_ADAPTER	pAd, 
	IN  UCHAR	*row,
	IN  int		profileNum)
{
	int byteIndex;

	/* Assume R179 has already been set to select Explicit or Implicit profiles */
	
	/* Read a tagfield */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R181, 0x80);
	for (byteIndex=0; byteIndex<EXP_MAX_BYTES; byteIndex++)
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R180, (profileNum<<5) | byteIndex);
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R182, &row[byteIndex]);
	}
}


/*
	Write_TagField - write a profile tagfield
*/
void Write_TagField(
	IN	PRTMP_ADAPTER	pAd, 
	IN  UCHAR	*row,
	IN  int		profileNum)
{
	int byteIndex;

	/* Assume R179 has already been set to select Explicit or Implicit profiles */
	
	/* Write a tagfield */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R181, 0x80);
	for (byteIndex=0; byteIndex<EXP_MAX_BYTES; byteIndex++ ) {
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R180, (profileNum<<5) | byteIndex);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R182, row[byteIndex]);
	}
}


/*
	displayTagfield - display one tagfield
*/
void displayTagfield(
	IN	PRTMP_ADAPTER	pAd, 
	IN	int		profileNum,
	IN	BOOLEAN implicitProfile)
{
	int byteIndex;
	UCHAR row[EXP_MAX_BYTES];

	/* Print a row of Tagfield data */
	DBGPRINT(RT_DEBUG_OFF, ("%d: ", profileNum));

	Read_TagField(pAd, row, profileNum);
	for (byteIndex=EXP_MAX_BYTES; --byteIndex >= 0; )
		DBGPRINT(RT_DEBUG_OFF, ("%02X ", row[byteIndex]));

	/* Decode the tag */
	DBGPRINT(RT_DEBUG_OFF, (" - "));
	if (implicitProfile) {
		static char *modeTable[8] = {"INV", "1 Leg", "1 HT", "2 HT", "INV", "INV", "INV", "INV"};
		static char *bwTable[8] = {"20M", "10M", "40Mh", "INV", "INV", "INV", "40Mf", "INV"};
		int mode = (row[7]>>4) & 0x7;

		switch (mode) {
		case 1:
		case 2:
		case 3:
			DBGPRINT(RT_DEBUG_OFF, ("%s %dx%s %c", bwTable[row[7] & 0x7],
				(row[0] & 0x3)+1, modeTable[mode], (row[7] & 0x80)? 'S': 'L') );
			break;
		default:
			DBGPRINT(RT_DEBUG_OFF, ("Invalid"));
		}
	}
	else {
		static char *tagTable[16] = {
			"INV", "INV", "INV", "INV", "2x1", "2x2", "INV", "INV",
			"3x1", "3x2", "3x3", "INV", "INV", "INV", "INV", "INV"};
		switch (row[7] & 0xF) {
		case 4:
		case 5:
		case 8:
		case 9:
		case 10:
			DBGPRINT(RT_DEBUG_OFF, ("%2dM %s", (row[7] & 0x10)? 40: 20, tagTable[row[7] & 0xF]) );
			break;
		default:
			DBGPRINT(RT_DEBUG_OFF, ("Invalid"));
		}
	}
	DBGPRINT(RT_DEBUG_OFF, ("\n"));
}


/*
	Unpack an ITxBF matrix element from a row of bytes
*/
int Unpack_IBFValue(
	IN UCHAR *row,
	IN int elemNum)
{
	int bitNum, byteOffset, bitOffset;
	int val;

	bitNum = elemNum*IMP_COEFF_SIZE;
	byteOffset = bitNum/8;
	bitOffset = bitNum - byteOffset*8;

	val = row[byteOffset] | (row[byteOffset+1]<<8);
	val = (val>>bitOffset) & IMP_COEFF_MASK;
	if (val >= 1<<(IMP_COEFF_SIZE-1) )
		val -= 1<<IMP_COEFF_SIZE;

	return val;
}


/*
	Pack an ITxBF matrix element into a row of bytes
*/
void Pack_IBFValue(
	IN	UCHAR	*row,
	IN	int		elemNum,
	IN	int		value)
{
	int bitNum, byteOffset, bitOffset;
	int rowBytes;

	bitNum = elemNum*IMP_COEFF_SIZE;
	byteOffset = bitNum/8;
	bitOffset = bitNum - byteOffset*8;

	rowBytes = row[byteOffset] | (row[byteOffset+1]<<8);

	rowBytes &= ~(IMP_COEFF_MASK<<bitOffset);
	rowBytes |= (value & IMP_COEFF_MASK)<<bitOffset;

	row[byteOffset] = rowBytes & 0xFF;
	row[byteOffset+1] = (rowBytes >> 8) & 0xFF;
}


/*
	Read_BFRow - read a row from a BF profile
*/
void Read_BFRow(
	IN	PRTMP_ADAPTER	pAd, 
	IN	UCHAR	*row,
	IN	int		profileNum,
	IN	int		rowIndex,
	IN	int		bytesPerRow)
{
	int byteIndex;

	/* Assume R179 has already been set to select Explicit or Implicit profiles */
	
	/* Read a row of data */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R181, rowIndex);
			
	for (byteIndex=0; byteIndex <bytesPerRow; byteIndex++) {
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R180, (profileNum<<5) | byteIndex);
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R182, &row[byteIndex]);
	}

}


/*
	Write_BFRow - write a row for a BF profile
*/
void Write_BFRow(
	IN	PRTMP_ADAPTER	pAd,
	IN	int		profileNum,
	IN	int		rowIndex, 
	IN	PROFILE_DATA	*pExp,
	IN	int		carrierIndex)
{
	int byteIndex, bytesPerRow;
	UCHAR *row = pExp->data[carrierIndex];

	/* Optimize the number of bytes written */
	if (pExp->impProfile)
		bytesPerRow = pExp->columns==1? IMP_MAX_BYTES_ONE_COL: IMP_MAX_BYTES;
	else
		bytesPerRow = EXP_MAX_BYTES;

	/* Assume R179 has already been set to select Explicit or Implicit profiles */
	
	/* Write a row of data */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R181, rowIndex);
			
	for (byteIndex=0; byteIndex <bytesPerRow; byteIndex++) {
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R180, (profileNum<<5) | byteIndex);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R182, row[byteIndex]);
	}
}


#ifdef DBG
#ifdef LINUX
/* Calculate time for BF profile access */
/* #define TIMESTAMP_BF_PROFILE		*/
#endif /* LINUX */
#endif /* DBG */

void Read_TxBfProfile(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PROFILE_DATA	*prof,
	IN	int				profileNum,
	IN	BOOLEAN			implicitProfile)
{
	int carrierIndex, scIndex;
	int maxBytes;
	SC_TABLE_ENTRY *pTab;
	int j, c;
	UCHAR r163Value = 0;

#ifdef TIMESTAMP_BF_PROFILE
	struct timeval tval1, tval2;
	do_gettimeofday(&tval1);
#endif

	/* Disable Profile Updates during access */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R163, &r163Value);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R163, r163Value & ~0x88);

	/* Select Implicit/Explicit profile */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R179, implicitProfile? 0: 0x04);
	
	/* Read tag and set up profile data */
	Read_TagField(pAd, prof->tag, profileNum);

	if (implicitProfile) {
		prof->impProfile = TRUE;
		prof->fortyMHz = (prof->tag[7] & 0x7)==6;

		switch (prof->tag[0] & 0x3)
		{
			case 0:
				prof->rows =  1;
				break;
			case 1:
				prof->rows =  2;
				break;
			default:
				prof->rows =  3;
				break;
		}

		prof->columns = (prof->tag[7] & 0x70)==0x30? 2: 1;
		prof->grouping = 1;

		/* Read subcarrier data */
		pTab = &impSubCarrierTable[prof->fortyMHz];
		maxBytes = prof->columns==1? IMP_MAX_BYTES_ONE_COL: IMP_MAX_BYTES;

		/* Negative subcarriers */
		carrierIndex = 0;
		for (scIndex=pTab->lwb1; scIndex <= pTab->upb1; scIndex++)
			Read_BFRow(pAd, prof->data[carrierIndex++], profileNum, scIndex, maxBytes);

		/* Positive subcarriers */
		for (scIndex=pTab->lwb2; scIndex <= pTab->upb2; scIndex++)
			Read_BFRow(pAd, prof->data[carrierIndex++], profileNum, scIndex, maxBytes);
	}
	else {
		prof->impProfile = FALSE;
		prof->fortyMHz = (prof->tag[7] & 0x10)!=0;
		prof->rows = 1 + (prof->tag[7]>>2 & 0x3);
		prof->columns = 1 + (prof->tag[7] & 0x03);

		switch (prof->tag[7] & 0x60) {
		case 0x20:
			prof->grouping = 2;
			break;
		case 0x40:
			prof->grouping = 4;
			break;
		default:	/* 1 or invalid values */
			prof->grouping = 1;
			break;
		}

		/* Read subcarrier data */
		pTab = &expSubCarrierTable[prof->fortyMHz];
		carrierIndex = 0;

		/* Negative subcarriers */
		for (scIndex=pTab->lwb1; scIndex < pTab->upb1; scIndex += prof->grouping) {
			c = carrierIndex;
			Read_BFRow(pAd, prof->data[carrierIndex++], profileNum, scIndex, EXP_MAX_BYTES);

			/*
				Replicate data if subcarriers are grouped. For 20Mhz the last carrier requires 
				special handling to make sure it isn't overwritten when replicating the data
			*/
			for (j=1; j<prof->grouping; j++) {
				if (!prof->fortyMHz && carrierIndex==(PROFILE_MAX_CARRIERS_20/2 - 1))
					break;
				memcpy(prof->data[carrierIndex++], prof->data[c], sizeof(prof->data[c]));
			}
		}
		Read_BFRow(pAd, prof->data[carrierIndex++], profileNum, pTab->upb1, EXP_MAX_BYTES);

		/* Positive subcarriers */
		for (scIndex=pTab->lwb2; scIndex < pTab->upb2; scIndex += prof->grouping) {
			c = carrierIndex;
			Read_BFRow(pAd, prof->data[carrierIndex++], profileNum, scIndex, EXP_MAX_BYTES);

			/* 
				Replicate data if subcarriers are grouped. For 20Mhz the last carrier requires 
				special handling to make sure it isn't overwritten when replicating the data
			*/
			for (j=1; j<prof->grouping; j++) {
				if (!prof->fortyMHz && carrierIndex==(PROFILE_MAX_CARRIERS_20-1))
					break;
				memcpy(prof->data[carrierIndex++], prof->data[c], sizeof(prof->data[c]));
			}
		}
		Read_BFRow(pAd, prof->data[carrierIndex++], profileNum, pTab->upb2, EXP_MAX_BYTES);
	}

	/* Restore Profile Updates */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R163, r163Value);

#ifdef TIMESTAMP_BF_PROFILE
	do_gettimeofday(&tval2);
	DBGPRINT(RT_DEBUG_WARN, ("BF Read elasped = %ld usec\n", tval2.tv_usec - tval1.tv_usec));
#endif
}


void Write_TxBfProfile(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PROFILE_DATA	*prof,
	IN	int				profileNum)
{
	int carrierIndex, scIndex;
	SC_TABLE_ENTRY *pTab;
	int maxBytes;
	UCHAR r163Value = 0;
#ifdef TIMESTAMP_BF_PROFILE
	struct timeval tval1, tval2;
	do_gettimeofday(&tval1);
#endif

	/* Disable Profile Updates during access */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R163, &r163Value);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R163, r163Value & ~0x88);

	/* Select Implicit/Explicit profile */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R179, prof->impProfile? 0: 0x04);
	
	/* Write Tagfield format byte so it matches the profile */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R181, 0x80);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R180, (profileNum<<5) | 0x7);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R182, profData.tag[7]);
	
	/* Write Implicit or Explicit profile */
	if (prof->impProfile) {
		/* Write subcarrier data */
		pTab = &impSubCarrierTable[profData.fortyMHz];
		maxBytes = profData.columns==1? IMP_MAX_BYTES_ONE_COL: IMP_MAX_BYTES;

		carrierIndex = 0;
		for (scIndex=pTab->lwb1; scIndex <= pTab->upb1; scIndex++)
			Write_BFRow(pAd, profileNum, scIndex, &profData, carrierIndex++);

		for (scIndex=pTab->lwb2; scIndex <= pTab->upb2; scIndex++)
			Write_BFRow(pAd, profileNum, scIndex, &profData, carrierIndex++);
	}
	else {
		/* Write subcarrier data. If data is grouped then just write every n-th subcarrier */
		pTab = &expSubCarrierTable[profData.fortyMHz];
		carrierIndex = 0;

		/* Negative subcarriers */
		for (scIndex=pTab->lwb1; scIndex<pTab->upb1; scIndex += profData.grouping) {
			Write_BFRow(pAd, profileNum, scIndex, &profData, carrierIndex);
			carrierIndex += profData.grouping;
		}
		/*  In 20MHz mode the last carrier in the group is a special case */
		if (!profData.fortyMHz)
			carrierIndex--;
		Write_BFRow(pAd, profileNum, pTab->upb1, &profData, carrierIndex++);

		/* Positive subcarriers */
		for (scIndex=pTab->lwb2; scIndex<pTab->upb2; scIndex += profData.grouping) {
			Write_BFRow(pAd, profileNum, scIndex, &profData, carrierIndex);
			carrierIndex += profData.grouping;
		}
		if (!profData.fortyMHz)
			carrierIndex--;
		Write_BFRow(pAd, profileNum, pTab->upb2, &profData, carrierIndex);
	}

	/* Restore Profile Updates */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R163, r163Value);

#ifdef TIMESTAMP_BF_PROFILE
	do_gettimeofday(&tval2);
	DBGPRINT(RT_DEBUG_WARN, ("BF Write elasped = %ld usec\n", tval2.tv_usec - tval1.tv_usec));
#endif
}


#ifdef DBG
#ifdef LINUX
/* #define TIMESTAMP_CALC_CALIBRATION */
#endif /* LINUX */
#endif /* DBG */

#define P_RESOLUTION	256		/* Resolution of phase calculation: 2pi/256 */

INT32 ei0[PROFILE_MAX_CARRIERS_40][2];
INT32 ei1[PROFILE_MAX_CARRIERS_40][2];
INT32 ei2[PROFILE_MAX_CARRIERS_40][2];

/*
	iCalcCalibration - calculate calibration parameters
		Returns 0 if successful, -1 if profiles are invalid
*/
int iCalcCalibration(PRTMP_ADAPTER pAd, int calParams[2], int profileNum)
{
	int		pi, maxCarriers, ii;

	short rot[2], rot1[2];
	INT32 c0[2], c1[2];
	INT32 minSum=0;
	int	  di1=0, di2=0;

	PROFILE_DATA *pExpData, *pImpData;
	int result = 0;
#ifdef TIMESTAMP_CALC_CALIBRATION
	struct timeval tval1, tval2;
#endif

	if (os_alloc_mem(pAd, (UCHAR **)&pExpData, sizeof(PROFILE_DATA))!= NDIS_STATUS_SUCCESS)
	{
		return -3;
	}
	
	if (os_alloc_mem(pAd, (UCHAR **)&pImpData, sizeof(PROFILE_DATA)) != NDIS_STATUS_SUCCESS)
	{
		os_free_mem(pAd, pExpData);
		return -3;
	}
	/* Read Implicit and Explicit data */
	Read_TxBfProfile(pAd, pImpData, profileNum, TRUE);
	Read_TxBfProfile(pAd, pExpData, profileNum, FALSE);

	hex_dump("pImpData", &pImpData->tag[1], 6);
	hex_dump("pExpData", &pExpData->tag[1], 6);
	
	/* Quit if MAC addresses don't match */
	for (ii=1; ii<7; ii++) {
		if (pImpData->tag[ii]!=pExpData->tag[ii]) {
			result = -2;
			goto exitCalcCal;
		}
	}

	/* Quit if profiles cannot be used */
	if (pImpData->fortyMHz!=pExpData->fortyMHz || pImpData->rows<2 || pExpData->rows<2) {
		result = -1;
		goto exitCalcCal;
	}

	/* 
		If Implicit profile is legacy then zero out the unused carriers so they don't
		affect the calculation
	*/
	if ((pImpData->tag[7] & 0x70)==0x10) {
	   memset(pImpData->data[0], 0x00, sizeof(pImpData->data[0]));
	   memset(pImpData->data[1], 0x00, sizeof(pImpData->data[0]));
	   memset(pImpData->data[PROFILE_MAX_CARRIERS_20-2], 0x00, sizeof(pImpData->data[0]));
	   memset(pImpData->data[PROFILE_MAX_CARRIERS_20-1], 0x00, sizeof(pImpData->data[0]));
	}

#ifdef TIMESTAMP_CALC_CALIBRATION
	do_gettimeofday(&tval1);
#endif

	maxCarriers = pImpData->fortyMHz? PROFILE_MAX_CARRIERS_40: PROFILE_MAX_CARRIERS_20;

	/* Compute Exp .* conj(Imp). ei0 is 2p25, ei1 and ei2 are are 2p15 */
	for (pi=0; pi<maxCarriers; pi++) {
		INT32 ed[2];
		ed[0] = ((CHAR)pExpData->data[pi][0])<<10;
		ed[1] = ((CHAR)pExpData->data[pi][1])<<10;
		icMult(ei0[pi], ed, Unpack_IBFValue(pImpData->data[pi], 1), -Unpack_IBFValue(pImpData->data[pi], 0));
		ed[0] = (CHAR)pExpData->data[pi][6];
		ed[1] = (CHAR)pExpData->data[pi][7];
		icMult(ei1[pi], ed, Unpack_IBFValue(pImpData->data[pi], 3), -Unpack_IBFValue(pImpData->data[pi], 2));
		ed[0] = (CHAR)pExpData->data[pi][12];
		ed[1] = (CHAR)pExpData->data[pi][13];
		icMult(ei2[pi], ed, Unpack_IBFValue(pImpData->data[pi], 5), -Unpack_IBFValue(pImpData->data[pi], 4));
	}

	/* Search for best Phase 1 */
	for (ii=0; ii<P_RESOLUTION; ii++) {
		INT32 sum = 0;

		icexp(rot, ii*256/P_RESOLUTION);

		for (pi=0; pi<maxCarriers; pi++) {
			icMult(c1, ei1[pi], rot[0], rot[1]);
			/* Sum as 1p25 and scale 1p25 => 1p13 */
			c0[0] = (ei0[pi][0] + c1[0])>>12;
			c0[1] = (ei0[pi][1] + c1[1])>>12;
			sum -= isqrt(c0[0]*c0[0] + c0[1]*c0[1]);
		}
		/* ATEDBGPRINT(RT_DEBUG_OFF, ("%d s=%d %d %d\n", ii, sum, c0[0], c1[0])); */

		/* Record minimum */
		if (ii==0 || minSum>sum) {
			di1 = ii;
			minSum = sum;
		}
	}

	/* Search for best Phase 2 */
	if (pImpData->rows==2 || pExpData->rows==2) {
		di2 = 0;
	}
	else {
		icexp(rot1, di1);
		/* ei0 = ei0 + rot1*ei1 */
		for (pi=0; pi<maxCarriers; pi++) {
			icMult(c1, ei1[pi], rot1[0], rot1[1]);
			ei0[pi][0] += c1[0];
			ei0[pi][1] += c1[1];
		}

		for (ii=0; ii<P_RESOLUTION; ii++) {
			INT32 sum = 0;

			icexp(rot, ii*256/P_RESOLUTION);

			for (pi=0; pi<maxCarriers; pi++) {
				/* Compute ei0 + ei2*rot. Scale 1p25 => 1p13 */
				icMult(c1, ei2[pi], rot[0], rot[1]);
				c0[0] = (ei0[pi][0] + c1[0]) >> 12;
				c0[1] = (ei0[pi][1] + c1[1]) >> 12;
				sum -= isqrt(c0[0]*c0[0] + c0[1]*c0[1]);
			}

			/* Record minimum */
			if (ii==0 || minSum>sum) {
				di2 = ii;
				minSum = sum;
			}
		}
	}

	/* Convert to calibration parameters */
	calParams[0] = -di1 & 0xFF;
	calParams[1] = -(di1-di2) & 0xFF;

#ifdef TIMESTAMP_CALC_CALIBRATION
	do_gettimeofday(&tval2);
    ATEDBGPRINT(RT_DEBUG_WARN, ("iCalcCal = %ld usec\n", tval2.tv_usec - tval1.tv_usec));
#endif

exitCalcCal:
	os_free_mem(pAd, pExpData);
	os_free_mem(pAd, pImpData);

	return result;
}

