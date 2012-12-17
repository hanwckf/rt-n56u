-- Plugin for converting channels lists from DVB receivers based on Enigma2
-- Copyright (C) 2012 Igor Drach
-- leaigor@gmail.com


function dreambox_url_encode(str)
  if (str) then
    str = string.gsub (str, " ", "%%20")
    str = string.gsub (str, "\"", "%%22")
  end
  return str
end

function xml_to_tables(file,label,started)
--    print("label=",label)
    local xmltable={}
    repeat
	local string=file:read()
	if string then
--	    print("string="..string)
	    local n,m,i,field,j=string.find(string,"<(%w+)>(.*)<%/(%w+)>")
--	    print(i,field,j)
	    if i==j and field and started then
--		print("add to table",i,field)
--		table.insert(xmltable,{i,field})
		xmltable[i]=field
--		table.insert(xmltable,{"value",{i,field}})
	    else
		local i,j,field=string.find(string,"<(%w+)>")
--		print(field)
		if field then
		    if  started or not label then
--		    print("start "..field)
--			local insertedtable=xml_to_tables(file,field,true)
--			print("insertedtable=",insertedtable)
			local insertedvalue={}
			insertedvalue[field]=xml_to_tables(file,field,true)
			table.insert(xmltable,insertedvalue)
		    elseif label==field then
			label=field
			started=true
		    end
		else
		    local i,j,field=string.find(string,"<%/(%w+)>")
		    if field==label and field  then
--		    print("close "..field)
--			print("table=",xmltable)
--			dreambox_table_print(xmltable)
			return xmltable
		    end
		end
	    end
	end
    until not string
--  return xmltable
end

function dreambox_table_print (tt, indent, done)
  done = done or {}
  indent = indent or 0
  if type(tt) == "table" then
    for key, value in pairs (tt) do
      io.write(string.rep (" ", indent)) -- indent it
      if type (value) == "table" and not done [value] then
        done [value] = true
        io.write(string.format("[%s] => table\n", tostring (key)));
        io.write(string.rep (" ", indent+4)) -- indent it
        io.write("(\n");
        dreambox_table_print (value, indent + 7, done)
        io.write(string.rep (" ", indent+4)) -- indent it
        io.write(")\n");
      else
        io.write(string.format("[%s] => %s\n",
            tostring (key), tostring(value)))
      end
    end
  else
    io.write(tt .. "\n")
  end
end


function dreambox_updatefeed(feed,friendly_name)
    local rc=false
    local feedspath=cfg.feeds_path
    if not friendly_name then
	friendly_name=feed
    end
    local wget="wget -q -O- "
    local dreambox_url='http://'..feed..'/web/'
--local dreambox_m3u_url='http://'..feed..'/web/stream.m3u?ref='
    local bouquets={}
    print(wget..dreambox_url.."getservices")
    local xmlbouquetsfile=io.popen(wget..dreambox_url.."getservices")
    bouquets=xml_to_tables(xmlbouquetsfile,"e2servicelist")
    xmlbouquetsfile:close()
    if bouquets then
        local bindex
        local bouquett={}
        local bouquet={}
        for bindex,bouquett in pairs(bouquets) do
	    bouquet=bouquett["e2service"]
	    if bouquet then
		local bouquetname=bouquet["e2servicename"]
		local bouquetreference=bouquet["e2servicereference"]
		local cindex
		local channelt={}
		local channel={}
		print(wget..dreambox_url_encode(dreambox_url.."getservices?sRef="..bouquetreference))
		local xmlbouquetfile=io.popen(wget..dreambox_url_encode(dreambox_url.."getservices?sRef="..bouquetreference))
	        bouquet=xml_to_tables(xmlbouquetfile,"e2servicelist")
	        xmlbouquetfile:close()
	        if bouquet then
	    	    local m3ufilename=cfg.tmp_path.."dreambox_"..friendly_name.."_bouquet"..bindex..".m3u"
		    print(m3ufilename)
	    	    local m3ufile=io.open(m3ufilename,"w")
		    m3ufile:write("#EXTM3U name=\""..bouquetname.."\" plugin=dreambox type=ts\n")
		    for cindex,channelt in pairs(bouquet) do
			channel=channelt["e2service"]
			local channelname=channel["e2servicename"]
			local channelreference=channel["e2servicereference"]
			m3ufile:write("#EXTINF:0,"..channelname.."\n")
			m3ufile:write(dreambox_url.."stream.m3u?ref="..channelreference.."\n")
		    end
		    m3ufile:close()
		    os.execute(string.format('mv %s %s',m3ufilename,feedspath))
		    rc=true
		end
	    end
	end
	local m3ufilename=cfg.tmp_path.."dreambox_"..friendly_name.."_controls"..".m3u"
	local m3ufile=io.open(m3ufilename,"w")
	m3ufile:write("#EXTM3U name=\"Управление\" plugin=dreambox type=ts\n")
	m3ufile:write("#EXTINF:0,1-Текущий канал\n")
	m3ufile:write(dreambox_url.."command=current\n")
	m3ufile:write("#EXTINF:0,2-Проснуться + Текущий канал\n")
	m3ufile:write(dreambox_url.."command=on\n")
	m3ufile:write("#EXTINF:0,3-Заснуть\n")
	m3ufile:write(dreambox_url.."command=off\n")
	m3ufile:close()
	os.execute(string.format('mv %s %s',m3ufilename,feedspath))
    end
return rc
end


function dreambox_sendurl(dreambox_url,range)
    print("range=",range)
    local url
    local file
    local wget="wget -q -O- "
    local wgetn="wget -q -O /dev/null "
    local i,j,urlbase=string.find(dreambox_url,"(.+/web/).+")
    if urlbase then
	print("BaseURL=",urlbase)
    end
    local i,j,command=string.find(dreambox_url,".+command=(%w+)")
    local standby={}
    file=io.popen(wget..urlbase..'powerstate')
    standby=xml_to_tables(file,"e2powerstate")
    file:close()
    print("standby=",standby["e2instandby"])
--    dreambox_table_print(standby)
    if command then
	print(command)
	if command=="on" then
	    if standby["e2instandby"]=='true' then
		print(wget..urlbase..'remotecontrol?command=116')
		file=io.popen(wget..urlbase..'remotecontrol?command=116')
		file:read()
		file:close()
	    end
        end
    end
    if standby["e2instandby"]=='false' and command~="on" and command~="current" then
	print(wget..urlbase..'powerstate?newstate=0')
	file=io.popen(wget..urlbase..'powerstate?newstate=0')
	file:read()
	file:close()
    end
    local currentservices={}
    local currentstreamservices={}

    print(wget..urlbase.."streamsubservices")
    file=io.popen(wget..urlbase.."streamsubservices")
    currentstreamservices=xml_to_tables(file,"e2servicelist")
    file:close()
--    dreambox_table_print(currentstreamservices)

    print(wget..urlbase.."subservices")
    file=io.popen(wget..urlbase.."subservices")
    currentservices=xml_to_tables(file,"e2servicelist")
    file:close()
--    dreambox_table_print(currentservices)

    local index
    local currentservice={}
    local service={}
    local streamservice={}
    local busy=true
    for index,currentservice in pairs(currentstreamservices) do
	service=currentservice["e2service"]
	print("service=",service["e2servicereference"])
      if not url then
	if service["e2servicereference"]=="N/A" and not command then
		busy=false
		url='wget -q -O- '..dreambox_url
	elseif service["e2servicereference"]~="N/A" then
	    url='wget -q -O- '..urlbase.."stream.m3u?ref="..service["e2servicereference"]
	end
      end
    end    
    for index,currentservice in pairs(currentservices) do
	service=currentservice["e2service"]
	print("service=",service["e2servicereference"])
      if not url then
	if service["e2servicereference"]=="N/A" and not command then
		busy=false
		url='wget -q -O- '..dreambox_url
	elseif service["e2servicereference"]~="N/A" then
	    url='wget -q -O- '..urlbase.."stream.m3u?ref="..service["e2servicereference"]
	end
      end
    end    

    print("busy=",busy)
    if url then
	file=io.popen(url)
	file:read()
	file:read()
	url=file:read()
    	file:close()
    	print("url=",url)
	plugin_sendurl(dreambox_url,url,range)
    else
	plugin_sendfile('www/corrupted.mp4')
    end
end

--dreambox_updatefeed("10.36.40.5","dreambox1")
--dreambox_sendurl("http://10.36.40.5/web/stream.m3u?ref=1:0:1:BBB:CB:FFFF:1682DF6:0:0:0:",0)

plugins['dreambox']={}
plugins.dreambox.name="DreamBox"
plugins.dreambox.desc="IP address (example: <i>192.168.0.1</i>)"
plugins.dreambox.updatefeed=dreambox_updatefeed
plugins.dreambox.sendurl=dreambox_sendurl
