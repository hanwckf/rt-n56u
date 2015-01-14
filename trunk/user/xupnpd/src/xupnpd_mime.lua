-- Copyright (C) 2011-2015 Anton Burdinuk
-- clark15b@gmail.com
-- https://tsdemuxer.googlecode.com/svn/trunk/xupnpd

upnp_type=
{
    ['video'] = 1,
    ['audio'] = 2,
    ['image'] = 3
}

upnp_class=
{
    ['video']     = 'object.item.videoItem',
    ['audio']     = 'object.item.audioItem.musicTrack'
}                


upnp_proto=
{
    ['avi']   = 'http-get:*:video/avi:',
    ['asf']   = 'http-get:*:video/x-ms-asf:',
    ['wmv']   = 'http-get:*:video/x-ms-wmv:',
    ['mp4']   = 'http-get:*:video/mp4:',
    ['mpeg']  = 'http-get:*:video/mpeg:',
    ['mpeg2'] = 'http-get:*:video/mpeg2:',
    ['mp2t']  = 'http-get:*:video/mp2t:',
    ['mp2p']  = 'http-get:*:video/mp2p:',
    ['mov']   = 'http-get:*:video/quicktime:',
    ['mkv']   = 'http-get:*:video/x-matroska:',
    ['mka']   = 'http-get:*:audio/x-matroska:',
    ['3gp']   = 'http-get:*:video/3gpp:',
    ['flv']   = 'http-get:*:video/x-flv:',
    ['aac']   = 'http-get:*:audio/x-aac:',
    ['ac3']   = 'http-get:*:audio/x-ac3:',
    ['mp3']   = 'http-get:*:audio/mpeg:',
    ['ogg']   = 'http-get:*:audio/x-ogg:',
    ['wma']   = 'http-get:*:audio/x-ms-wma:'
}

-- DLNA.ORG_PN, DLNA.ORG_OP, DLNA.ORG_CI, DLNA.ORG_FLAGS
-- DLNA.ORG_OP=00 - no seek; DLNA.ORG_OP=01 - seek
dlna_org_extras=
{
    ['none']                  = '*',

    -- video
    ['mpeg_ps_pal']           = 'DLNA.ORG_PN=MPEG_PS_PAL;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['mpeg_ps_pal_ac3']       = 'DLNA.ORG_PN=MPEG_PS_PAL_XAC3;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['mpeg_ps_ntsc']          = 'DLNA.ORG_PN=MPEG_PS_NTSC;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['mpeg_ps_ntsc_ac3']      = 'DLNA.ORG_PN=MPEG_PS_NTSC_XAC3;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['mpeg1']                 = 'DLNA.ORG_PN=MPEG1;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['mpeg_ts_sd']            = 'DLNA.ORG_PN=MPEG_TS_SD_NA_ISO;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['mpeg_ts_hd']            = 'DLNA.ORG_PN=MPEG_TS_HD_NA;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['avchd']                 = 'DLNA.ORG_PN=AVC_TS_HD_50_AC3;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['wmv_med_base']          = 'DLNA.ORG_PN=WMVMED_BASE;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['wmv_med_full']          = 'DLNA.ORG_PN=WMVMED_FULL;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['wmv_med_pro']           = 'DLNA.ORG_PN=WMVMED_PRO;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['wmv_high_full']         = 'DLNA.ORG_PN=WMVHIGH_FULL;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['wmv_high_pro']          = 'DLNA.ORG_PN=WMVHIGH_PRO;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['asf_mpeg4_sp']          = 'DLNA.ORG_PN=MPEG4_P2_ASF_SP_G726;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['asf_mpeg4_asp_l4']      = 'DLNA.ORG_PN=MPEG4_P2_ASF_ASP_L4_SO_G726;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['asf_mpeg4_asp_l5']      = 'DLNA.ORG_PN=MPEG4_P2_ASF_ASP_L5_SO_G726;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['asf_vc1_l1']            = 'DLNA.ORG_PN=VC1_ASF_AP_L1_WMA;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['mp4_avc_sd_mp3']        = 'DLNA.ORG_PN=AVC_MP4_MP_SD_MPEG1_L3;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['mp4_avc_sd_ac3']        = 'DLNA.ORG_PN=AVC_MP4_MP_SD_AC3;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['mp4_avc_hd_ac3']        = 'DLNA.ORG_PN=AVC_MP4_MP_HD_AC3;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['mp4_avc_sd_aac']        = 'DLNA.ORG_PN=AVC_MP4_MP_SD_AAC_MULT5;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000', 
    ['mpeg_ts_hd_mp3']        = 'DLNA.ORG_PN=AVC_TS_MP_HD_MPEG1_L3;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['mpeg_ts_hd_ac3']        = 'DLNA.ORG_PN=AVC_TS_MP_HD_AC3;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['mpeg_ts_mpeg4_asp_mp3'] = 'DLNA.ORG_PN=MPEG4_P2_TS_ASP_MPEG1_L3;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['mpeg_ts_mpeg4_asp_ac3'] = 'DLNA.ORG_PN=MPEG4_P2_TS_ASP_AC3;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['avi']                   = 'DLNA.ORG_PN=AVI;DLNA.ORG_OP=11;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['divx5']                 = 'DLNA.ORG_PN=PV_DIVX_DX50;DLNA.ORG_OP=11;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',

    -- audio
    ['mp3']                   = 'DLNA.ORG_PN=MP3;DLNA.ORG_OP=11;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['ac3']                   = 'DLNA.ORG_PN=AC3;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['wma_base']              = 'DLNA.ORG_PN=WMABASE;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['wma_full']              = 'DLNA.ORG_PN=WMAFULL;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000',
    ['wma_pro']               = 'DLNA.ORG_PN=WMAPRO;DLNA.ORG_OP=00;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000'
}

mime=
{
    ['avi']    = { upnp_type.video, upnp_class.video, 'video/avi',       upnp_proto.avi,   dlna_org_extras.divx5 },
    ['asf']    = { upnp_type.video, upnp_class.video, 'video/x-ms-asf',  upnp_proto.asf,   dlna_org_extras.asf_mpeg4_sp },
    ['wmv']    = { upnp_type.video, upnp_class.video, 'video/x-ms-wmv',  upnp_proto.wmv,   dlna_org_extras.wmv_med_full },
    ['mp4']    = { upnp_type.video, upnp_class.video, 'video/mp4',       upnp_proto.mp4,   dlna_org_extras.none },
    ['mpeg']   = { upnp_type.video, upnp_class.video, 'video/mpeg',      upnp_proto.mpeg,  dlna_org_extras.mpeg_ps_pal },        -- video/x-mpeg
    ['mpeg_ts']= { upnp_type.video, upnp_class.video, 'video/mpeg',      upnp_proto.mpeg,  dlna_org_extras.mpeg_ts_sd },         -- for Sharp
    ['mpeg1']  = { upnp_type.video, upnp_class.video, 'video/mpeg',      upnp_proto.mpeg,  dlna_org_extras.mpeg1 },
    ['mpeg2']  = { upnp_type.video, upnp_class.video, 'video/mpeg2',     upnp_proto.mpeg2, dlna_org_extras.mpeg_ps_pal },
    ['ts']     = { upnp_type.video, upnp_class.video, 'video/mp2t',      upnp_proto.mp2t,  dlna_org_extras.mpeg_ts_hd },
    ['mp2t']   = { upnp_type.video, upnp_class.video, 'video/mp2t',      upnp_proto.mp2t,  dlna_org_extras.mpeg_ts_hd },
    ['mp2p']   = { upnp_type.video, upnp_class.video, 'video/mp2p',      upnp_proto.mp2p,  dlna_org_extras.mpeg_ps_pal },
    ['mov']    = { upnp_type.video, upnp_class.video, 'video/quicktime', upnp_proto.mov,   dlna_org_extras.none },
    ['mkv']    = { upnp_type.video, upnp_class.video, 'video/x-mkv',     upnp_proto.mkv,   dlna_org_extras.none },               -- video/x-matroska
    ['3gp']    = { upnp_type.video, upnp_class.video, 'video/3gpp',      upnp_proto['3gp'],dlna_org_extras.none },
    ['flv']    = { upnp_type.video, upnp_class.video, 'video/x-flv',     upnp_proto.flv,   dlna_org_extras.none },
    ['aac']    = { upnp_type.audio, upnp_class.audio, 'audio/x-aac',     upnp_proto.aac,   dlna_org_extras.none },
    ['ac3']    = { upnp_type.audio, upnp_class.audio, 'audio/x-ac3',     upnp_proto.ac3,   dlna_org_extras.ac3 },
    ['mp3']    = { upnp_type.audio, upnp_class.audio, 'audio/mpeg',      upnp_proto.mp3,   dlna_org_extras.mp3 },
    ['ogg']    = { upnp_type.audio, upnp_class.audio, 'application/ogg', upnp_proto.ogg,   dlna_org_extras.none },
    ['wma']    = { upnp_type.audio, upnp_class.audio, 'audio/x-ms-wma',  upnp_proto.wma,   dlna_org_extras.wma_full }
}
