lg_dlna_org_extras='DLNA.ORG_OP=01;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000'

profiles['LG']=
{
    ['desc']='LG TV',

    -- Linux/2.6.31-1.0 UPnP/1.0 DLNADOC/1.50 INTEL_NMPR/2.0 LGE_DLNA_SDK/1.5.0
    -- Mozilla/5.0 (compatible; MSIE 8.0; Windows NT 5.1; LG_UA; AD_LOGON=LGE.NET; .NET CLR 2.0.50727; .NET CLR 3.0.04506.30; .NET CLR 3.0.04506.648; LG_UA; AD_LOGON=LGE.NET; LG NetCast.TV-2010)
    ['match']=function(user_agent)
                if string.find(user_agent,'LGE_DLNA_SDK',1,true) or string.find(user_agent,'LG NetCast',1,true)
                    then return true
                else
                    return false
                end
            end,

    ['options']=
    {
        ['content_disp']=true
    },

    ['replace_mime_types']=true,

    ['mime_types']=
    {
        ['ogm']   = { upnp_type.video,  upnp_class.video,  'video/ogg',             'http-get:*:video/ogg:',            lg_dlna_org_extras },
        ['oga']   = { upnp_type.audio,  upnp_class.audio,  'audio/ogg',             'http-get:*:audio/ogg:',            lg_dlna_org_extras },
        ['aac']   = { upnp_type.audio,  upnp_class.audio,  'audio/x-aac',           upnp_proto.aac,                     lg_dlna_org_extras },
        ['asx']   = { upnp_type.video,  upnp_class.video,  'video/x-ms-asf',        upnp_proto.asf,                     lg_dlna_org_extras },
        ['mka']   = { upnp_type.audio,  upnp_class.audio,  'audio/x-matroska',      upnp_proto.mka,                     lg_dlna_org_extras },
        ['avi']   = { upnp_type.video,  upnp_class.video,  'video/x-mpeg',          'http-get:*:video/x-mpeg:',         lg_dlna_org_extras },
        ['m2ts']  = { upnp_type.video,  upnp_class.video,  'video/x-matroska',      upnp_proto.mkv,                     lg_dlna_org_extras },
        ['txt']   = { upnp_type.video,  upnp_class.video,  'video/subtitle',        'http-get:*:video/subtitle:',       lg_dlna_org_extras },
        ['mpe']   = { upnp_type.video,  upnp_class.video,  'video/mpeg',            upnp_proto.mpeg,                    lg_dlna_org_extras },
        ['m3u']   = { upnp_type.audio,  upnp_class.audio,  'audio/x-mpegurl',       'http-get:*:audio/x-mpegurl:',      lg_dlna_org_extras },
        ['3gp']   = { upnp_type.video,  upnp_class.video,  'video/3gpp',            upnp_proto['3gp'],                  lg_dlna_org_extras },
        ['mp2p']  = { upnp_type.video,  upnp_class.video,  'video/x-matroska',      upnp_proto.mkv,                     lg_dlna_org_extras },
        ['srt']   = { upnp_type.video,  upnp_class.video,  'video/subtitle',        'http-get:*:video/subtitle:',       lg_dlna_org_extras },
        ['wvx']   = { upnp_type.video,  upnp_class.video,  'video/x-ms-wvx',        'http-get:*:video/x-ms-wvx:',       lg_dlna_org_extras },
        ['trp']   = { upnp_type.video,  upnp_class.video,  'video/x-msvideo',       'http-get:*:video/x-msvideo:',      lg_dlna_org_extras },
        ['flv']   = { upnp_type.video,  upnp_class.video,  'video/x-flv',           upnp_proto.flv,                     lg_dlna_org_extras },
        ['ogv']   = { upnp_type.video,  upnp_class.video,  'video/ogg',             'http-get:*:video/ogg:',            lg_dlna_org_extras },
        ['mpg']   = { upnp_type.video,  upnp_class.video,  'video/x-mpeg',          'http-get:*:video/x-mpeg:',         lg_dlna_org_extras },
        ['mkv']   = { upnp_type.video,  upnp_class.video,  'video/x-mkv',           'http-get:*:video/x-mkv:',          lg_dlna_org_extras },
        ['divx']  = { upnp_type.video,  upnp_class.video,  'video/x-msvideo',       'http-get:*:video/x-msvideo:',      lg_dlna_org_extras },
        ['mp3']   = { upnp_type.audio,  upnp_class.audio,  'audio/mpeg',            upnp_proto.mp3,                     lg_dlna_org_extras },
        ['wmv']   = { upnp_type.video,  upnp_class.video,  'video/x-ms-wmv',        upnp_proto.wmv,                     lg_dlna_org_extras },
        ['asf']   = { upnp_type.video,  upnp_class.video,  'video/x-ms-asf',        upnp_proto.asf,                     lg_dlna_org_extras },
        ['wm']    = { upnp_type.video,  upnp_class.video,  'video/x-ms-wm',         'http-get:*:video/x-ms-wm:',        lg_dlna_org_extras },
        ['wax']   = { upnp_type.audio,  upnp_class.audio,  'audio/x-ms-wax',        'http-get:*:audio/x-ms-wax:',       lg_dlna_org_extras },
        ['wma']   = { upnp_type.audio,  upnp_class.audio,  'audio/x-ms-wma',        upnp_proto.wma,                     lg_dlna_org_extras },
        ['sub']   = { upnp_type.video,  upnp_class.video,  'video/subtitle',        'http-get:*:video/subtitle:',       lg_dlna_org_extras },
        ['ts']    = { upnp_type.video,  upnp_class.video,  'video/x-matroska',      upnp_proto.mkv,                     lg_dlna_org_extras },
        ['vob']   = { upnp_type.video,  upnp_class.video,  'video/mpeg',            upnp_proto.mpeg,                    lg_dlna_org_extras },
        ['mpeg']  = { upnp_type.video,  upnp_class.video,  'video/x-mpeg',          'http-get:*:video/x-mpeg:',         lg_dlna_org_extras },
        ['mp2t']  = { upnp_type.video,  upnp_class.video,  'video/x-matroska',      upnp_proto.mkv,                     lg_dlna_org_extras },
        ['wmx']   = { upnp_type.video,  upnp_class.video,  'video/x-ms-wmx',        'http-get:*:video/x-ms-wmx:',       lg_dlna_org_extras },
        ['ogg']   = { upnp_type.audio,  upnp_class.audio,  'audio/ogg',             'http-get:*:audio/ogg:',            lg_dlna_org_extras },
        ['m4a']   = { upnp_type.audio,  upnp_class.audio,  'audio/mp4',             'http-get:*:audio/mp4:',            lg_dlna_org_extras },
        ['m4v']   = { upnp_type.video,  upnp_class.video,  'video/mp4',             upnp_proto.mp4,                     lg_dlna_org_extras },
        ['mp4']   = { upnp_type.video,  upnp_class.video,  'video/mpeg',            upnp_proto.mpeg,                    lg_dlna_org_extras },
        ['mpeg2'] = { upnp_type.video,  upnp_class.video,  'video/x-mpeg',          'http-get:*:video/x-mpeg:',         lg_dlna_org_extras },
        ['tp']    = { upnp_type.video,  upnp_class.video,  'video/x-msvideo',       'http-get:*:video/x-msvideo:',      lg_dlna_org_extras },
        ['pls']   = { upnp_type.audio,  upnp_class.audio,  'audio/x-scpls',         'http-get:*:audio/x-scpls:',        lg_dlna_org_extras },
        ['mpeg1'] = { upnp_type.video,  upnp_class.video,  'video/x-mpeg',          'http-get:*:video/x-mpeg:',         lg_dlna_org_extras },
        ['ac3']   = { upnp_type.audio,  upnp_class.audio,  'audio/x-ac3',           upnp_proto.ac3,                     lg_dlna_org_extras },
        ['mov']   = { upnp_type.video,  upnp_class.video,  'video/x-quicktime',     'http-get:*:video/x-quicktime:',    lg_dlna_org_extras }
    }
}

profiles['LG'].mime_types['mpeg']=profiles['LG'].mime_types['avi']
