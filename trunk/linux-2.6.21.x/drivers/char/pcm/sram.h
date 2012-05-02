#ifndef __SRAM_H_
#define __SRAM_H_

enum    xRAMSPACE
        {VOC,                       //      sramadd        0                VOC
        VOCDELTA,                   //      sramadd        1                VOC_DELTA
        VOCLTH,                     //      sramadd        2                VOCL_THR
        VOCHTH,                     //      sramadd        3                VOCH_THR
        VCM,                        //      sramadd        4                VCM
        VOV,                        //      sramadd        5                VOV
        VOVRING,                    //      sramadd        6                VOV_RING
        VLOOP,                        //      sramadd        7                VLOOP
        ILOOP,                        //      sramadd        8                ILOOP
        ILONG,                       //      sramadd        9                ILONG
        VOCTRACK,                   //      sramadd       10                VOC_TRACK
        VTIP,                   //      sramadd       11                VTIP_SCALED
        VRING,                  //      sramadd       12                VRING_SCALED
        VBAT,                   //      sramadd       13                VBAT_SCALED
        ITIPP,                    //      sramadd       14                IQ1_SCALED
        IRINGP,                    //      sramadd       15                IQ2_SCALED
        IRINGN,                    //      sramadd       16                IQ3_SCALED
        ITIPN,                    //      sramadd       17                IQ4_SCALED
        IRING,                    //      sramadd       18                IQ5_SCALED
        ITIP,                    //      sramadd       19                IQ6_SCALED
        VRNGNG,                  //      sramadd       20                VRING_EXT_SCALED
        IRNGNG,                  //      sramadd       21                RING_I
        LCROFFHK,                    //      sramadd       22                OFFHK_THR                   loop closure detection
        LCRONHK,                     //      sramadd       23                ONHK_THR                    loop closure detection
        LCRDBI,                     //      sramadd       24                DBI_LCR                     loop closure detection
        LCRLPF,                     //      sramadd       25                NLCR                        loop closure detection
        LCRMASK,                    //      sramadd       26                LCRMASK                     loop closure detection
        LONGHITH,                   //      sramadd       27                LONGHI_THR                  longitudinal detection
        LONGLOTH,                   //      sramadd       28                LONGLO_THR                  longitudinal detection
        LONGDBI,                    //      sramadd       29                DBI_LONG                    longitudinal detection
        LONGLPF,                    //      sramadd       30                NLONG                       longitudinal detection
        BATHTH,                     //      sramadd       31                BATH_THR                    Batttery Selection
        BATLTH,                     //      sramadd       32                BATL_THR                    Batttery Selection
        BSWLPF,                     //      sramadd       33                NBAT                        Batttery Selection
        BATLPF,                     //      sramadd       34                NVBAT                       VOC Tracking
        CMLOTH,                     //      sramadd       35                CM_LOW_THR                  Speed up thresholds
        CMHITH,                     //      sramadd       36                CM_HIGH_THR                 Speed up thresholds
        PTH12,                     //      sramadd       37                PPTHR12                     Power Calculations
        PTH34,                     //      sramadd       38                PPTHR34                     Power Calculations
        PTH56,                     //      sramadd       39                NPTHR56                     Power Calculations
        PLPFQ12,                       //      sramadd       40                PLPFQ12                        while       all     coefficiare     13bits, most    have    a       range   of
        PLPFQ34,                       //      sramadd       41                PLPFQ34                        0-2.0       this    gives   an      lsb     size    of      1/2^11
        PLPFQ56,                       //      sramadd       42                PLPFQ56                        the         NQ      coefficihave    a       range   of      0-1.0   which   gives   them
        RB56,                       //      sramadd       43                RB56                        and         lsb     size    of      1/2^12
        PQ1DH,                      //      sramadd       44                PQ1_DH
        PQ2DH,                      //      sramadd       45                PQ2_DH
        PQ3DH,                      //      sramadd       46                PQ3_DH
        PQ4DH,                      //      sramadd       47                PQ4_DH
        PQ5DH,                      //      sramadd       48                PQ5_DH
        PQ6DH,                      //      sramadd       49                PQ6_DH
        PSUM,                      //      sramadd       50                PQ1_DL
        DIAGDC,                   //      sramadd       51                SLIC_DIAG_DC_DH             Diagnostic Filters
        DIAGDCCO,                   //      sramadd       52                SLIC_DC_COEFF               Diagnostic Filters
        DIAGAC,                   //      sramadd       53                SLIC_DIAG_AC_DH             Diagnostic Filters
        DIAGACCO,                   //      sramadd       54                SLIC_AC_COEFF               Diagnostic Filters
        DIAGPK,                     //      sramadd       55                SLIC_DIAG_PEAK              Diagnostic Filters
        RINGOF,                     //      sramadd       56                RING_OFFSET                 Ringing
        RINGFRHI,                   //      sramadd       57                RING_COEFF_REG              Ringing
        RINGFRLO,                   //      sramadd       58                RING_COEFF_REG_LO           Ringing
        RINGAMP,                    //      sramadd       59                RING_X_REG                  Ringing
        RINGPHAS,                   //      sramadd       60                RING_Y_REG                  Ringing
        RTCOUNT,                    //      sramadd       61                WTCHDOG_TIMEOUT             Ringing
        RTDCTH,                     //      sramadd       62                RINGTRIP_DC_THRESH          Ring Trip
        RTPER,                      //      sramadd       63                RINGTRIP_FULLPERIOD         Ring Trip
        RTACTH,                     //      sramadd       64                RINGTRIP_AC_THRESH          Ring Trip
        RTDCDB,                     //      sramadd       65                RINGTRIP_DC_DEB             Ring Trip
        RTACDB,                     //      sramadd       66                RINGTRIP_AC_DEB             Ring Trip
        PMFREQ,                     //      sramadd       67                PULSE_COEFF_REG             Pulse Metering
        PMAMPL,                     //      sramadd       68                PULSE_X_REG                 Pulse Metering
        PMRAMP,                     //      sramadd       69                PULSE_DELTA_REG             Pulse Metering
        PMAMPTH,                    //      sramadd       70                PM_AMP_THRESH               Pulse Metering
        RXGAIN,                    //      sramadd       71                DAC_GAIN                    Audio Gains
        TXGAIN,                    //      sramadd       72                ADC_GAIN                    Audio Gains
        TXEQC03,                   //      sramadd       73                ADCEQ_COEFF3                Equilizer
        TXEQC02,                   //      sramadd       74                ADCEQ_COEFF2                Equilizer
        TXEQCO1,                   //      sramadd       75                ADCEQ_COEFF1                Equilizer
        TXEQCO0,                   //      sramadd       76                ADCEQ_COEFF0                Equilizer
        RXEQCO3,                   //      sramadd       77                DACEQ_COEFF3                Equilizer
        RXEQCO2,                   //      sramadd       78                DACEQ_COEFF2                Equilizer
        RXEQCO1,                   //      sramadd       79                DACEQ_COEFF1                Equilizer
        RXEQCO0,                   //      sramadd       80                DACEQ_COEFF0                Equilizer
        RXIIRPOL,                   //      sramadd       81                DACIIREQ_POLE               Equilizer
        ECCO1,                      //      sramadd       82                EC_COEFF1                   Hybrid Echo Cancelor
        ECCO2,                      //      sramadd       83                EC_COEFF2                   Hybrid Echo Cancelor
        ECCO3,                      //      sramadd       84                EC_COEFF3                   Hybrid Echo Cancelor
        ECCO4,                      //      sramadd       85                EC_COEFF4                   Hybrid Echo Cancelor
        ECCO5,                      //      sramadd       86                EC_COEFF5                   Hybrid Echo Cancelor
        ECCO6,                      //      sramadd       87                EC_COEFF6                   Hybrid Echo Cancelor
        ECCO7,                      //      sramadd       88                EC_COEFF7                   Hybrid Echo Cancelor
        ECCO0,                      //      sramadd       89                EC_COEFF0                   Hybrid Echo Cancelor
        ECIIRB0,                    //      sramadd       90                ECIIR_B0                    Hybrid Echo Cancelor
        ECIIRB1,                    //      sramadd       91                ECIIR_B1                    Hybrid Echo Cancelor
        ECIIRA1,                    //      sramadd       92                ECIIR_A1                    Hybrid Echo Cancelor
        ECIIRA2,                    //      sramadd       93                ECIIR_A2                    Hybrid Echo Cancelor
        OSC1FREQ,                   //      sramadd       94                OSC1_COEFF_REG              Tone Generator
        OSC1AMP,                    //      sramadd       95                OSC1_X_REG                  Tone Generator
        OSC1PHAS,                   //      sramadd       96                OSC1_Y_REG                  Tone Generator
        OSC2FREQ,                   //      sramadd       97                OSC2_COEFF_REG              Tone Generator
        OSC2AMP,                    //      sramadd       98                OSC2_X_REG                  Tone Generator
        OSC2PHAS,                   //      sramadd       99                OSC2_Y_REG                  Tone Generator
        FSKFREQ0,                   //      sramadd      100                FSK_COEFF0_REG              Caller ID
        FSKFREQ1,                   //      sramadd      101                FSK_COEFF1_REG              Caller ID
        FSKAMP0,                    //      sramadd      102                FSK_X0_REG                  Caller ID
        FSKAMP1,                    //      sramadd      103                FSK_X1_REG                  Caller ID
        FSK01HI,                  //      sramadd      104                FSK_X01H_REG                Caller ID
        FSK01LO,                  //      sramadd      105                FSK_X01L_REG                Caller ID
        FSK10HI,                  //      sramadd      106                FSK_X10H_REG                Caller ID
        FSK10LO,                  //      sramadd      107                FSK_X10L_REG                Caller ID
        DTROW0TH,                     //      sramadd      108                ROW0_THR                    DTMF Detection
        DTROW1TH,                     //      sramadd      109                ROW1_THR                    DTMF Detection
        DTROW2TH,                     //      sramadd      110                ROW2_THR                    DTMF Detection
        DTROW3TH,                     //      sramadd      111                ROW3_THR                    DTMF Detection
        DTCOLTH,                      //      sramadd      112                COL_THR                     DTMF Detection
        DTFTWTH,                      //      sramadd      113                FWD_TW_THR                  DTMF Detection
        DTRTWTH,                      //      sramadd      114                REV_TW_THR                  DTMF Detection
        DTROWRTH,                     //      sramadd      115                ROW_REL_THR                 DTMF Detection
        DTCOLRTH,                     //      sramadd      116                COL_REL_THR                 DTMF Detection
        DTROW2HTH,                    //      sramadd      117                ROW_2ND_THR                 DTMF Detection
        DTCOL2HTH,                    //      sramadd      118                COL_2ND_THR                 DTMF Detection
        DTMINPTH,                   //      sramadd      119                PWR_MIN_THR                 DTMF Detection
        DTHOTTH,                   //      sramadd      120                HOT_LIMIT                   Modem Detection
        RXPWR,                      //      sramadd      121                RX2100_PWR_THR              Modem Detection
        TXPWR,                      //      sramadd      122                TX2100_PWR_THR              Modem Detection
        RXMODPWR,                   //      sramadd      123                RX2100_THR                  Modem Detection
        TXMODPWR,                   //      sramadd      124                TX2100_THR                  Modem Detection
        TESTB0L1,                   //      sramadd      125                TESTFILT_B0L_1              Transmit Diadiagnostfilter
        TESTB0H1,                   //      sramadd      126                TESTFILT_B0H_1              Transmit Diagnostic Filter
        TESTB1L1,                   //      sramadd      127                TESTFILT_B1L_1              Transmit Diagnostic Filter
        TESTB1H1,                   //      sramadd      128                TESTFILT_B1H_1              Transmit Diagnostic Filter
        TESTB2L1,                   //      sramadd      129                TESTFILT_B2L_1              Transmit Diagnostic Filter
        TESTB2H1,                   //      sramadd      130                TESTFILT_B2H_1              Transmit Diagnostic Filter
        TESTA1L1,                   //      sramadd      131                TESTFILT_A1L_1              Transmit Diagnostic Filter
        TESTA1H1,                   //      sramadd      132                TESTFILT_A1H_1              Transmit Diagnostic Filter
        TESTA2L1,                   //      sramadd      133                TESTFILT_A2L_1              Transmit Diagnostic Filter
        TESTA2H1,                   //      sramadd      134                TESTFILT_A2H_1              Transmit Diagnostic Filter
        TESTB0L2,                   //      sramadd      135                TESTFILT_B0L_2              Transmit Diagnostic Filter
        TESTB0H2,                   //      sramadd      136                TESTFILT_B0H_2              Transmit Diagnostic Filter
        TESTB1L2,                   //      sramadd      137                TESTFILT_B1L_2              Transmit Diagnostic Filter
        TESTB1H2,                   //      sramadd      138                TESTFILT_B1H_2              Transmit Diagnostic Filter
        TESTB2L2,                   //      sramadd      139                TESTFILT_B2L_2              Transmit Diagnostic Filter
        TESTB2H2,                   //      sramadd      140                TESTFILT_B2H_2              Transmit Diagnostic Filter
        TESTA1L2,                   //      sramadd      141                TESTFILT_A1L_2              Transmit Diagnostic Filter
        TESTA1H2,                   //      sramadd      142                TESTFILT_A1H_2              Transmit Diagnostic Filter
        TESTA2L2,                   //      sramadd      143                TESTFILT_A2L_2              Transmit Diagnostic Filter
        TESTA2H2,                   //      sramadd      144                TESTFILT_A2H_2              Transmit Diagnostic Filter
        TESTB0L3,                   //      sramadd      145                TESTFILT_B0L_3              Transmit Diagnostic Filter
        TESTB0H3,                   //      sramadd      146                TESTFILT_B0H_3              Transmit Diagnostic Filter
        TESTB1L3,                   //      sramadd      147                TESTFILT_B1L_3              Transmit Diagnostic Filter
        TESTB1H3,                   //      sramadd      148                TESTFILT_B1H_3              Transmit Diagnostic Filter
        TESTB2L3,                   //      sramadd      149                TESTFILT_B2L_3              Transmit Diagnostic Filter
        TESTB2H3,                   //      sramadd      150                TESTFILT_B2H_3              Transmit Diagnostic Filter
        TESTA1L3,                   //      sramadd      151                TESTFILT_A1L_3              Transmit Diagnostic Filter
        TESTA1H3,                   //      sramadd      152                TESTFILT_A1H_3              Transmit Diagnostic Filter
        TESTA2L3,                   //      sramadd      153                TESTFILT_A2L_3              Transmit Diagnostic Filter
        TESTA2H3,                   //      sramadd      154                TESTFILT_A2H_3              Transmit Diagnostic Filter
        TESTPKO,                    //      sramadd      155                TESTFILT_PEAK_O             Transmit Diagnostic Filter
        TESTABO,                    //      sramadd      156                TESTFILT_ABSACCH            Transmit Diagnostic Filter
        TESTWLN,                    //      sramadd      157                TESTFILT_WINLEN             Transmit Diagnostic Filter
        TESTAVBW,                   //      sramadd      158                TESTFILT_AVE_BW             Transmit Diagnostic Filter
        TESTPKFL,                   //      sramadd      159                TESTFILT_PEAK_FLAG          Transmit Diagnostic Filter
        TESTAVFL,                   //      sramadd      160                TESTFILT_AVE_FLAG           Transmit Diagnostic Filter
        TESTPKTH,                   //      sramadd      161                TESTFILT_PEAK_THR           Transmit Diagnostic Filter
        TESTAVTH,                   //      sramadd      162                TESTFILT_AVE_THR
        TXHPF1,                    //      sramadd      163                ADCHPF_A1_1
        TXHPF2,                    //      sramadd      164                ADCHPF_A1_2
        TXHPF3,               //      sramadd      165                ADCHPF_A2_2
		LASTSRAM
        };



//define external registers
#define LFSDELAY 57
#define ZERDELAY 59


//external ringing coefficents
#define       initLFSDELAY           0x50
#define       initZERDELAY            0x181
#define       initRTCOUNT            0x4B0
#define       initextRTDCTH             0x51
#define       initextRTACTH             0x7FFF

//RAM coefficents
#define       initVOC                0x2668
#define       initVOCDELTA           0x592
#define       initVOCLTH             0xF9A2
#define       initVOCHTH             0x197
#define       initVCM                0x268
#define       initVOV                0x32F
#define       initVOVRING            0
#define       initVLOOP              0
#define       initILOOP              0
#define       initILONG              0
#define       initVOCTRACK           0
#define       initVTIP				 0
#define       initVRING				 0
#define       initVBAT				 0
#define       initITIPP				 0
#define       initIRINGP		     0
#define       initIRINGN             0
#define       initITIPN				 0
#define       initIRING              0
#define       initITIP               0
#define       initVRNGNG             0
#define       initIRNGNG             0
#define       initLCROFFHK           0x0F22
#define       initLCRONHK            0xC9C
#define       initLCRDBI             0x20
#define       initLCRLPF             0xA08
#define       initLCRMASK            0x60
#define       initLONGHITH           0xA17
#define       initLONGLOTH           0x8D4
#define       initLONGDBI            0x20
#define       initLONGLPF            0xA08
#define       initBATHTH             0xE54//0x1300//0xE54
#define       initBATLTH             0xD88//0x1300//0xD88
#define       initBSWLPF             0xA08
#define       initBATLPF             0xA08
#define       initCMLOTH             0x7F5
#define       initCMHITH             0xCB

#define       initPTH12             0x54C//PSUM coefficent

#define       initPTH34             0x1B94
#define       initPTH56             0x785

#define       initPLPFQ12               0x10//PSUM coefficent

#define       initPLPFQ34               0x88
#define       initPLPFQ56               0x8
#define       initRB56               0
#define       initPQ1DH              0
#define       initPQ2DH              0
#define       initPQ3DH              0
#define       initPQ4DH              0
#define       initPQ5DH              0
#define       initPQ6DH              0
#define       initPSUM		         0
#define       initDIAGDCTH           0
#define       initDIAGDCCO           0xA08
#define       initDIAGACTH           0
#define       initDIAGACCO           0x7FF8
#define       initDIAGPK             0
#define       initRINGOF             0x0
#define       initRINGFRHI           0x3F78
#define       initRINGFRLO           0x6CE8
#define       initRINGAMP            0xCB
#define       initRINGPHAS           0x0
#define       initRTCOUNT            0x4B0
#define       initRTDCTH             0x7FFF
#define       initRTPER              0x28
#define       initRTACTH             0xFD2
#define       initRTDCDB             0x3
#define       initRTACDB             0x3
#define       initPMFREQ             0x0
#define       initPMAMPL             0x4000
#define       initPMRAMP             0x8A
#define       initPMAMPTH            0xC8
#define       initRXGAIN            0x4000//0x0040//0x4000
#define       initTXGAIN            0x4000//0x4000
#define       initTXEQC03           0x168//0x148
#define       initTXEQC02           0xF860//0xF8B0
#define       initTXEQCO1           0x4A38//0x4A68
#define       initTXEQCO0           0x0
#define       initRXEQCO3           0x0020//0xFFF8
#define       initRXEQCO2           0x0100//0xF0
#define       initRXEQCO1           0xF958//0xFA38
#define       initRXEQCO0           0x57E0//0x4148
#define       initRXIIRPOL           0x3E08//0x3DC0
#define       initECCO1              0x0668//0xF98
#define       initECCO2              0x1A38//0x24D8
#define       initECCO3              0x1110//0xFD0
#define       initECCO4              0xFFC8//0xFEC0
#define       initECCO5              0xFFC0//0x48
#define       initECCO6              0xFFB0//0xFFB8
#define       initECCO7              0xFFB0//xFFE8
#define       initECCO0              0x0028//0xD8
#define       initECIIRB0            0xFFB0//0xFFD8
#define       initECIIRB1            0x0048//0xFFF8
#define       initECIIRA1            0x7800//0xD4F0
#define       initECIIRA2            0xC7E0//0xEB50

//do not change these values till after calibration.  Longitudal Balance relies on oscillator 1.
#define       initOSC1FREQ           0x2C30
#define       initOSC1AMP            0xD9
#define       initOSC1PHAS           0x0

#define       initOSC2FREQ           0x3C38
#define       initOSC2AMP            0x63
#define       initOSC2PHAS           0x0
#define       initFSKFREQ0           0x35B0
#define       initFSKFREQ1           0x3CE0
#define       initFSKAMP0            0x0222
#define       initFSKAMP1            0x0123
#define       initFSK01HI          0x1118
#define       initFSK01LO          0x1d88
#define       initFSK10HI          0x3BE0
#define       initFSK10LO          0x1330
#define       initDTROW0TH             0x2AE1
#define       initDTROW1TH             0x28F3
#define       initDTROW2TH             0x25C2
#define       initDTROW3TH             0x249B
#define       initDTCOLTH              0x1999
#define       initDTFTWTH              0x1013
#define       initDTRTWTH              0x1013
#define       initDTROWRTH             0xcc5
#define       initDTCOLRTH             0xcc5
#define       initDTROW2HTH            0x308c
#define       initDTCOL2HTH            0x1013
#define       initDTMINPTH           0xe5 
#define       initDTHOTTH           0xa1c 






#define       initRXPWR              0
#define       initTXPWR              0
#define       initRXMODPWR           0
#define       initTXMODPWR           0

//test filters coefficents are needed for longitudial balance calibration.
#define       initTESTB0L1           0x3900
#define       initTESTB0H1           0x0138
#define       initTESTB1L1           0
#define       initTESTB1H1           0
#define       initTESTB2L1           0x4700
#define       initTESTB2H1           0xFEC0
#define       initTESTA1L1           0x370
#define       initTESTA1H1           0x5568
#define       initTESTA2L1           0x2100
#define       initTESTA2H1           0xC430
#define       initTESTB0L2           0x460
#define       initTESTB0H2           0x5568
#define       initTESTB1L2           0
#define       initTESTB1H2           0
#define       initTESTB2L2           0
#define       initTESTB2H2           0
#define       initTESTA1L2           0
#define       initTESTA1H2           0
#define       initTESTA2L2           0
#define       initTESTA2H2           0
#define       initTESTB0L3           0
#define       initTESTB0H3           0x4000
#define       initTESTB1L3           0
#define       initTESTB1H3           0
#define       initTESTB2L3           0
#define       initTESTB2H3           0
#define       initTESTA1L3           0
#define       initTESTA1H3           0
#define       initTESTA2L3           0
#define       initTESTA2H3           0
#define       initTESTPKO            0
#define       initTESTABO            0
#define       initTESTWLN            0x0013
#define       initTESTAVBW           0
#define       initTESTPKFL           0
#define       initTESTAVFL           0
#define       initTESTPKTH           0
#define       initTESTAVTH           0
#define       initTXHPF1		     0x3858
#define       initTXHPF2		     0x7748
#define       initTXHPF3		     0xC7A0




#define   szVOC              "VOC"
#define   szVOCDELTA         "VOCDELTA"
#define   szVOCLTH           "VOCLTH"
#define   szVOCHTH           "VOCHTH"
#define   szVCM              "VCM"
#define   szVOV              "VOV"
#define   szVOVRING          "VOVRING"
#define   szVLOOP              "VLOOP"
#define   szILOOP              "ILOOP"
#define   szILONG             "ILONG"
#define   szVOCTRACK         "VOCTRACK"
#define   szVTIP         "VTIP"
#define   szVRING        "VRING"
#define   szVBAT         "VBAT"
#define   szITIPP          "ITIPP"
#define   szIRINGP          "IRINGP"
#define   szIRINGN          "IRINGN"
#define   szITIPN          "ITIPN"
#define   szIRING          "IRING"
#define   szITIP          "ITIP"
#define   szVRNGNG        "VRNGNG"
#define   szIRNGNG        "IRNGNG"
#define   szLCROFFHK          "LCROFFHK"
#define   szLCRONHK           "LCRONHK"
#define   szLCRDBI           "LCRDBI"
#define   szLCRLPF           "LCRLPF"
#define   szLCRMASK          "LCRMASK"
#define   szLONGHITH         "LONGHITH"
#define   szLONGLOTH         "LONGLOTH"
#define   szLONGDBI          "LONGDBI"
#define   szLONGLPF          "LONGLPF"
#define   szBATHTH           "BATHTH"
#define   szBATLTH           "BATLTH"
#define   szBSWLPF           "BSWLPF"
#define   szBATLPF           "BATLPF"
#define   szCMLOTH           "CMLOTH"
#define   szCMHITH           "CMHITH"
#define   szPTH12           "PTH12"
#define   szPTH34           "PTH34"
#define   szPTH56           "PTH56"
#define   szPLPFQ12             "PLPFQ12"
#define   szPLPFQ34             "PLPFQ34"
#define   szPLPFQ56             "PLPFQ56"
#define   szRB56             "RB56"
#define   szPQ1DH            "PQ1DH"
#define   szPQ2DH            "PQ2DH"
#define   szPQ3DH            "PQ3DH"
#define   szPQ4DH            "PQ4DH"
#define   szPQ5DH            "PQ5DH"
#define   szPQ6DH            "PQ6DH"
#define   szPSUM            "PSUM"
#define   szDIAGDCTH         "DIAGDCTH"
#define   szDIAGDCCO         "DIAGDCCO"
#define   szDIAGACTH         "DIAGACTH"
#define   szDIAGACCO         "DIAGACCO"
#define   szDIAGPK           "DIAGPK"
#define   szRINGOF           "RINGOF"
#define   szRINGFRHI         "RINGFRHI"
#define   szRINGFRLO         "RINGFRLO"
#define   szRINGAMP          "RINGAMP"
#define   szRINGPHAS         "RINGPHAS"
#define   szRTCOUNT          "RTCOUNT"
#define   szRTDCTH           "RTDCTH"
#define   szRTPER            "RTPER"
#define   szRTACTH           "RTACTH"
#define   szRTDCDB           "RTDCDB"
#define   szRTACDB           "RTACDB"
#define   szPMFREQ           "PMFREQ"
#define   szPMAMPL           "PMAMPL"
#define   szPMRAMP           "PMRAMP"
#define   szPMAMPTH          "PMAMPTH"
#define   szRXGAIN          "RXGAIN"
#define   szTXGAIN          "TXGAIN"
#define   szTXEQC03         "TXEQC03"
#define   szTXEQC02         "TXEQC02"
#define   szTXEQCO1         "TXEQCO1"
#define   szTXEQCO0         "TXEQCO0"
#define   szRXEQCO3         "RXEQCO3"
#define   szRXEQCO2         "RXEQCO2"
#define   szRXEQCO1         "RXEQCO1"
#define   szRXEQCO0         "RXEQCO0"
#define   szRXIIRPOL         "RXIIRPOL"
#define   szECCO1            "ECCO1"
#define   szECCO2            "ECCO2"
#define   szECCO3            "ECCO3"
#define   szECCO4            "ECCO4"
#define   szECCO5            "ECCO5"
#define   szECCO6            "ECCO6"
#define   szECCO7            "ECCO7"
#define   szECCO0            "ECCO0"
#define   szECIIRB0          "ECIIRB0"
#define   szECIIRB1          "ECIIRB1"
#define   szECIIRA1          "ECIIRA1"
#define   szECIIRA2          "ECIIRA2"
#define   szOSC1FREQ         "OSC1FREQ"
#define   szOSC1AMP          "OSC1AMP"
#define   szOSC1PHAS         "OSC1PHAS"
#define   szOSC2FREQ         "OSC2FREQ"
#define   szOSC2AMP          "OSC2AMP"
#define   szOSC2PHAS         "OSC2PHAS"
#define   szFSKFREQ0         "FSKFREQ0"
#define   szFSKFREQ1         "FSKFREQ1"
#define   szFSKAMP0          "FSKAMP0"
#define   szFSKAMP1          "FSKAMP1"
#define   szFSK01HI        "FSK01HI"
#define   szFSK01LO        "FSK01LO"
#define   szFSK10HI        "FSK10HI"
#define   szFSK10LO        "FSK10LO"
#define   szDTROW0TH           "DTROW0TH"
#define   szDTROW1TH           "DTROW1TH"
#define   szDTROW2TH           "DTROW2TH"
#define   szDTROW3TH           "DTROW3TH"
#define   szDTCOLTH            "DTCOLTH"
#define   szDTFTWTH            "DTFTWTH"
#define   szDTRTWTH            "DTRTWTH"
#define   szDTROWRTH           "DTROWRTH"
#define   szDTCOLRTH           "DTCOLRTH"
#define   szDTROW2HTH          "DTROW2HTH"
#define   szDTCOL2HTH          "DTCOL2HTH"
#define   szDTMINPTH         "DTMINPTH"
#define   szDTHOTTH         "DTHOTTH"
#define   szRXPWR            "RXPWR"
#define   szTXPWR            "TXPWR"
#define   szRXMODPWR         "RXMODPWR"
#define   szTXMODPWR         "TXMODPWR"
#define   szTESTB0L1         "TESTB0L1"
#define   szTESTB0H1         "TESTB0H1"
#define   szTESTB1L1         "TESTB1L1"
#define   szTESTB1H1         "TESTB1H1"
#define   szTESTB2L1         "TESTB2L1"
#define   szTESTB2H1         "TESTB2H1"
#define   szTESTA1L1         "TESTA1L1"
#define   szTESTA1H1         "TESTA1H1"
#define   szTESTA2L1         "TESTA2L1"
#define   szTESTA2H1         "TESTA2H1"
#define   szTESTB0L2         "TESTB0L2"
#define   szTESTB0H2         "TESTB0H2"
#define   szTESTB1L2         "TESTB1L2"
#define   szTESTB1H2         "TESTB1H2"
#define   szTESTB2L2         "TESTB2L2"
#define   szTESTB2H2         "TESTB2H2"
#define   szTESTA1L2         "TESTA1L2"
#define   szTESTA1H2         "TESTA1H2"
#define   szTESTA2L2         "TESTA2L2"
#define   szTESTA2H2         "TESTA2H2"
#define   szTESTB0L3         "TESTB0L3"
#define   szTESTB0H3         "TESTB0H3"
#define   szTESTB1L3         "TESTB1L3"
#define   szTESTB1H3         "TESTB1H3"
#define   szTESTB2L3         "TESTB2L3"
#define   szTESTB2H3         "TESTB2H3"
#define   szTESTA1L3         "TESTA1L3"
#define   szTESTA1H3         "TESTA1H3"
#define   szTESTA2L3         "TESTA2L3"
#define   szTESTA2H3         "TESTA2H3"
#define   szTESTPKO          "TESTPKO"
#define   szTESTABO          "TESTABO"
#define   szTESTWLN          "TESTWLN"
#define   szTESTAVBW         "TESTAVBW"
#define   szTESTPKFL         "TESTPKFL"
#define   szTESTAVFL         "TESTAVFL"
#define   szTESTPKTH         "TESTPKTH"
#define   szTESTAVTH         "TESTAVTH"
#define   szTXHPF1          "TXHPF1"
#define   szTXHPF2          "TXHPF2"
#define   szTXHPF3          "TXHPF3"

#endif /* __SRAM_H_ */