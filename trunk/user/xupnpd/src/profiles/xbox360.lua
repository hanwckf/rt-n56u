profiles['Xbox360']=
{
    ['disabled']=false,

    ['desc']='Microsoft Xbox 360',

    -- Xbox/2.0.12611.0 UPnP/1.0 Xbox/2.0.12611.0
    ['match']=function(user_agent) if string.find(user_agent,'Xbox',1,true) then return true else return false end end,

    ['options']=
    {
        ['dev_desc_xml']='/wmc.xml',
        ['dlna_extras']=true,
        ['upnp_container']='object.container.storageFolder',
        ['upnp_artist']=true
    },

    ['mime_types']={}
}
