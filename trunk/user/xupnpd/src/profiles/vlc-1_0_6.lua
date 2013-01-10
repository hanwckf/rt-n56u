-- for VLC 1.0.6

profiles['VLC-1.0.6']=
{
    ['desc']='VideoLAN Player 1.0.6',

    -- Linux/2.6.32-42-generic, UPnP/1.0, Portable SDK for UPnP devices/1.6.6
    -- VLC media player - version 1.0.6 Goldeneye - (c) 1996-2010 the VideoLAN team
    -- Linux/3.2.0-4-686-pae, UPnP/1.0, Portable SDK for UPnP devices/1.6.17" 
    ['match']=function(user_agent)
                if string.find(user_agent,'UPnP devices/1.6.',1,true) or string.find(user_agent,'VideoLAN',1,true) then
                    return true
                else
                    return false
                end
            end,

    ['options']=
    {
        ['soap_length']=false
    },

    ['mime_types']={}
}
