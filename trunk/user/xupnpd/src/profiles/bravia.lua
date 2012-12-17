profiles['BRAVIA']=
{
    ['desc']='Sony BRAVIA',

--X-AV-Physical-Unit-Info: pa="BRAVIA KDL-32W5500     ";
--X-AV-Client-Info: av=5.0; cn="Sony Corporation"; mn="BRAVIA KDL-32W5500     "; mv="1.7";
    ['match']=function(user_agent,req) local s=req['X-AV-Client-Info'] if s and string.find(s,'BRAVIA ',1,true) then return true else return false end end,

    ['options']={},

    ['mime_types']={}
}
