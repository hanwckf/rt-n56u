/*
** Copyright (c) 2009 Silicon Laboratories, Inc.
** 2009-12-20 13:32:19
**
** Si3217x ProSLIC API Configuration Tool Version 1.00L
*/



#include "vdaa.h"

vdaa_General_Cfg Vdaa_General_Configuration  = {
    INTE_DISABLED,
    INTE_ACTIVE_LOW,
    RES_CAL_ENABLED,
    FS_8KHZ,
    FOH_128,
    LVS_FORCE_ENABLED,
    CVS_CURRENT,
    CVP_ABOVE,
    GCE_DISABLED,
    IIR_DISABLED,
    FULL_DISABLED,
    FULL2_ENABLED,
    FILT_HPF_200HZ,
    RG1_DISABLED,
    PWM_DELTA_SIGMA,
    PWM_DISABLED,
    SPIM_TRI_CS
};

vdaa_Country_Cfg Vdaa_Country_Presets[] ={
    {
    RZ_MAX,
    DC_50,
    AC_600,
    DCV3_5,
    MINI_10MA,
    ILIM_DISABLED,
    OHS_LESS_THAN_0_5MS,
    HYBRID_ENABLED
    },  /* COU_USA */
    {
    RZ_MAX,
    DC_50,
    AC_270__750_150,
    DCV3_5,
    MINI_10MA,
    ILIM_ENABLED,
    OHS_3MS,
    HYBRID_ENABLED
    },  /* COU_GERMANY */
    {
    RZ_MAX,
    DC_50,
    AC_200__680_100,
    DCV3_5,
    MINI_10MA,
    ILIM_DISABLED,
    OHS_LESS_THAN_0_5MS,
    HYBRID_ENABLED
    },  /* COU_CHINA */
    {
    RZ_MAX,
    DC_50,
    AC_220__820_120,
    DCV3_2,
    MINI_12MA,
    ILIM_DISABLED,
    OHS_26MS,
    HYBRID_ENABLED
    }   /* COU_AUSTRALIA */
};

vdaa_audioGain_Cfg Vdaa_audioGain_Presets[] ={
    {
    0,    /* mute */
    XGA_GAIN,    /* xXGA2 */
    0,    /* xXG2 */
    XGA_GAIN,    /* xXGA3 */
    0,    /* xXG3 */
    64,    /* AxM */
    0     /* cpEn */
    },  /* AUDIO_GAIN_0DB */
    {
    0,    /* mute */
    XGA_ATTEN,    /* xXGA2 */
    4,    /* xXG2 */
    XGA_GAIN,    /* xXGA3 */
    0,    /* xXG3 */
    64,    /* AxM */
    0     /* cpEn */
    },  /* AUDIO_ATTEN_4DB */
    {
    0,    /* mute */
    XGA_ATTEN,    /* xXGA2 */
    6,    /* xXG2 */
    XGA_GAIN,    /* xXGA3 */
    0,    /* xXG3 */
    64,    /* AxM */
    0     /* cpEn */
    },  /* AUDIO_ATTEN_6DB */
    {
    0,    /* mute */
    XGA_ATTEN,    /* xXGA2 */
    11,    /* xXG2 */
    XGA_GAIN,    /* xXGA3 */
    0,    /* xXG3 */
    64,    /* AxM */
    0     /* cpEn */
    }   /* AUDIO_ATTEN_11DB */
};

vdaa_Ring_Detect_Cfg Vdaa_Ring_Detect_Presets[] ={
    {
    RDLY_512MS,
    RT__13_5VRMS_16_5VRMS,
    12,    /* RMX */
    RTO_1408MS,
    RCC_640MS,
    RNGV_DISABLED,
    17,    /* RAS */
    RFWE_HALF_WAVE,
    RDI_BEG_END_BURST, 
    RGDT_ACTIVE_LOW 
    },  /* RING_DET_NOVAL_LOWV */
    {
    RDLY_512MS,
    RT__40_5VRMS_49_5VRMS,
    12,    /* RMX */
    RTO_1408MS,
    RCC_640MS,
    RNGV_ENABLED,
    17,    /* RAS */
    RFWE_RNGV_RING_ENV,
    RDI_BEG_END_BURST, 
    RGDT_ACTIVE_LOW 
    }   /* RING_DET_VAL_HIGHV */
};

vdaa_PCM_Cfg Vdaa_PCM_Presets[] ={
    {
    A_LAW,
    PCLK_1_PER_BIT,
    TRI_POS_EDGE
    },  /* PCM_ALAW */
    {
    U_LAW,
    PCLK_1_PER_BIT,
    TRI_POS_EDGE
    },  /* PCM_ULAW */
    {
    LINEAR_16_BIT,
    PCLK_1_PER_BIT,
    TRI_POS_EDGE
    }   /* PCM_LINEAR16 */
};

vdaa_Hybrid_Cfg Vdaa_Hybrid_Presets[] ={
    {
       0,    /* HYB1 */
     254,    /* HYB2 */
       0,    /* HYB3 */
       1,    /* HYB4 */
     255,    /* HYB5 */
       1,    /* HYB6 */
       0,    /* HYB7 */
       0     /* HYB8 */
    },  /* HYB_600_0_0_500FT_24AWG */
    {
       4,    /* HYB1 */
     246,    /* HYB2 */
     242,    /* HYB3 */
       4,    /* HYB4 */
     254,    /* HYB5 */
     255,    /* HYB6 */
       1,    /* HYB7 */
     255     /* HYB8 */
    },  /* HYB_270_750_150_500FT_24AWG */
    {
       4,    /* HYB1 */
     245,    /* HYB2 */
     243,    /* HYB3 */
       7,    /* HYB4 */
     253,    /* HYB5 */
       0,    /* HYB6 */
       1,    /* HYB7 */
     255     /* HYB8 */
    },  /* HYB_200_680_100_500FT_24AWG */
    {
       4,    /* HYB1 */
     244,    /* HYB2 */
     241,    /* HYB3 */
       6,    /* HYB4 */
     253,    /* HYB5 */
     255,    /* HYB6 */
       2,    /* HYB7 */
     255     /* HYB8 */
    }   /* HYB_220_820_120_500FT_24AWG */
};

