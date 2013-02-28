-- Plugin for Netstreamsat
-- Copyright (C) 2012-2013 Mario Stephan
-- mstephan@shared-files.de

-- #EXTM3U name="Live TV" type=mpeg plugin=netstreamsat dlna_extras=mpeg_ts
-- #EXTINF:0,Das Erste
-- http://netstreamsat/stream/nameDasErste
-- #EXTINF:0,ZDF
-- http://netstreamsat/stream/nameZDF
-- #EXTINF:0,RTL Television
-- http://netstreamsat/stream/nameRTLTelevision
-- #EXTINF:0,SAT.1
-- http://netstreamsat/stream/nameSat1


function read_url()
  local f = io.open('last_url')
  if f == nil then
  	io.close()
	return '',os.time(),''
  end
  local u = f:read("*all")
  t={} ; i=1
  for str in string.gmatch(u, "[^%s]+") do
	t[i] = str
	i = i + 1
  end

  f:close()
  return t[1],t[2],t[3]

end

function save_url(url, stream )
  local f = io.open('last_url', 'w')
  	f:write(url, "\n")
  	f:write(stream, "\n")
  	f:write(os.time())
  	f:close()
end


function netstreamsat_updatefeed(feed,friendly_name)
    return true
end


function netstreamsat_sendurl(netstreamsat_url,range)
	
	local urlbase=string.match(netstreamsat_url,"(http://.-)/.-")
	if urlbase==nil then
		urlbase="http://netstreamsat"
	end
	local stream=""
	local switch=""
    
    local l_url, l_stream, l_time = read_url()
	
	if l_url == netstreamsat_url and l_time+15 > os.time() then
		-- re-use of last known stream
		stream = l_stream
	else
		-- select a free tuner    
		local status = http.download(urlbase.."/status")
		local tuner1 = string.match(status,'.*Tuner 1.-Channel Number <b>(.-)</b>,.-')
		local tuner2 = string.match(status,'.*Tuner 2.-Channel Number <b>(.-)</b>,.-')
		
		if tuner1==nil then
			tuner1="not available"
		end
		if tuner2==nil then
			tuner2="not available"
		end
		
		print("Status Tuner1: "..tuner1:gsub('0','streaming'):gsub('-1','free'))
		print("Status Tuner2: "..tuner2:gsub('0','streaming'):gsub('-1','free'))
		
		-- switch channel
		if tuner1=="-1" or tuner2=="not available" then                                                                                               
		   stream=urlbase..":10001"
		   switch=netstreamsat_url:gsub('/stream/','/stream/tuner1/')
		else                                                                                                                          
		   stream=urlbase..":10002"
		   switch=netstreamsat_url:gsub('/stream/','/stream/tuner2/')
		end 	
		
		print('switch device to new channel: '..switch)
		local s = http.download(switch)
	end
	
	
	print('send this stream: '..stream)
	save_url(netstreamsat_url, stream)
	plugin_sendurl(netstreamsat_url,stream,range)
  
end

plugins['netstreamsat']={}
plugins.netstreamsat.name="NetstreamSat"
plugins.netstreamsat.desc="NetstreamSat via upnp"
plugins.netstreamsat.updatefeed=netstreamsat_updatefeed
plugins.netstreamsat.sendurl=netstreamsat_sendurl

