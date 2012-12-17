profiles['MediaPlayer']=
{
    ['desc']='Windows Media Player',

    -- Microsoft-Windows/6.1 UPnP/1.0 Windows-Media-Player-DMS/12.0.7601.17514 DLNADOC/1.50
    -- Microsoft-Windows/6.1 UPnP/1.0 Windows-Media-Player/12.0.7601.17514 DLNADOC/1.50 (MS-DeviceCaps/1024)
    -- NSPlayer/12.00.7601.17514 WMFSDK/12.00.7601.17514
    ['match']=function(user_agent)
                if string.find(user_agent,'Windows-Media-Player',1,true) or string.find(user_agent,'NSPlayer',1,true) then
                    return true
                else
                    return false
                end
            end,

    ['options']=
    {
        ['upnp_artist']=true
    },

    ['mime_types']={}
}
