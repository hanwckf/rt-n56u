profiles['BRAVIA']=
{
    ['disabled']=false,

    ['desc']='Sony BRAVIA',

--X-AV-Physical-Unit-Info: pa="BRAVIA KDL-22EX553";
--X-AV-Client-Info: av=5.0; cn="Sony Corporation"; mn="BRAVIA KDL-22EX553"; mv="1.7";
--User-Agent: UPnP/1.0 DLNADOC/1.50

    ['match']=function(user_agent,req)
        local s = req['x-av-client-info'] 
        if s and string.find(s,'BRAVIA ',1,true) then 
            return true 
        else 
            return false 
            end 
        end,

    ['options']=
    {
        ['upnp_albumart'] = 1, -- 1: <res>direct url<res> OR 3: <res protocolInfo="http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_TN">http://127.0.0.1:4044/logo?s=0%2F1%2F14%2F33</res>
        ['dlna_headers']=true,
        ['dlna_extras']=true
    },

    ['mime_types']=
    {
        -- video streams
        ['xvid']   =  { upnp_type.video, upnp_class.video,  'video/avi',        upnp_proto.avi,         dlna_org_extras.none},
        ['divx']   =  { upnp_type.video, upnp_class.video,  'video/avi',        upnp_proto.avi,         dlna_org_extras.none},
        ['avi']    =  { upnp_type.video, upnp_class.video,  'video/avi',        upnp_proto.avi,         dlna_org_extras.none},
        ['mpeg']   =  { upnp_type.video, upnp_class.video,  'video/mpeg',       upnp_proto.mpeg,        dlna_org_extras.none},
        ['mpg']    =  { upnp_type.video, upnp_class.video,  'video/mpeg',       upnp_proto.mpeg,        dlna_org_extras.none}
    }
}
