profiles['WDTV-Live']=
{
    ['disabled']=true,

    ['desc']='WD TV Live',

    -- INTEL_NMPR/2.1 DLNADOC/1.50 Intel MicroStack/1.0.1423
    ['match']=function(user_agent) if string.find(user_agent,'MicroStack/1.0.1423',1,true) then return true else return false end end,

    ['options']=
    {
        ['wdtv']=true
    },

    ['mime_types']={}
}
