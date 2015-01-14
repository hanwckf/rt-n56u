-- Copyright (C) 2011-2015 Anton Burdinuk
-- clark15b@gmail.com
-- https://tsdemuxer.googlecode.com/svn/trunk/xupnpd

http_mime={}
http_err={}
http_vars={}

-- http_mime types
http_mime['html']='text/html'
http_mime['htm']='text/html'
http_mime['xml']='text/xml; charset="UTF-8"'
http_mime['txt']='text/plain'
http_mime['srt']='video/subtitle'
http_mime['cpp']='text/plain'
http_mime['h']='text/plain'
http_mime['lua']='text/plain'
http_mime['jpg']='image/jpeg'
http_mime['png']='image/png'
http_mime['ico']='image/vnd.microsoft.icon'
http_mime['mpeg']='video/mpeg'
http_mime['css']='text/css'
http_mime['json']='application/json'
http_mime['js']='application/javascript'
http_mime['m3u']='audio/x-mpegurl'

-- http http_error list
http_err[100]='Continue'
http_err[101]='Switching Protocols'
http_err[200]='OK'
http_err[201]='Created'
http_err[202]='Accepted'
http_err[203]='Non-Authoritative Information'
http_err[204]='No Content'
http_err[205]='Reset Content'
http_err[206]='Partial Content'
http_err[300]='Multiple Choices'
http_err[301]='Moved Permanently'
http_err[302]='Moved Temporarily'
http_err[303]='See Other'
http_err[304]='Not Modified'
http_err[305]='Use Proxy'
http_err[400]='Bad Request'
http_err[401]='Unauthorized'
http_err[402]='Payment Required'
http_err[403]='Forbidden'
http_err[404]='Not Found'
http_err[405]='Method Not Allowed'
http_err[406]='Not Acceptable'
http_err[407]='Proxy Authentication Required'
http_err[408]='Request Time-Out'
http_err[409]='Conflict'
http_err[410]='Gone'
http_err[411]='Length Required'
http_err[412]='Precondition Failed'
http_err[413]='Request Entity Too Large'
http_err[414]='Request-URL Too Large'
http_err[415]='Unsupported Media Type'
http_err[416]='Requested range not satisfiable'
http_err[500]='Internal Server http_error'
http_err[501]='Not Implemented'
http_err[502]='Bad Gateway'
http_err[503]='Out of Resources'
http_err[504]='Gateway Time-Out'
http_err[505]='HTTP Version not supported'

http_vars['fname']=cfg.name
http_vars['manufacturer']=util.xmlencode('Anton Burdinuk <clark15b@gmail.com>')
http_vars['manufacturer_url']=''
http_vars['description']=ssdp_server
http_vars['name']='xupnpd'
http_vars['version']=cfg.version
http_vars['url']='http://xupnpd.org'
http_vars['uuid']=ssdp_uuid
http_vars['interface']=ssdp.interface()
http_vars['port']=cfg.http_port
http_vars['uptime']=core.uptime

http_templ=
{
    '/dev.xml',
    '/wmc.xml',
    '/index.html'
}

dofile('xupnpd_soap.lua')
dofile('xupnpd_webapp.lua')

function compile_templates()
    local path=cfg.tmp_path..'xupnpd-cache'

    os.execute('mkdir -p '..path)

    for i,fname in ipairs(http_templ) do
        http.compile_template(cfg.www_root..fname,path..fname,http_vars)
    end
end

function http_send_headers(err,ext,len)
    http.send(
        string.format(
            'HTTP/1.1 %i %s\r\nPragma: no-cache\r\nCache-control: no-cache\r\nDate: %s\r\nServer: %s\r\nAccept-Ranges: none\r\n'..
            'Connection: close\r\nContent-Type: %s\r\nEXT:\r\n',
            err,http_err[err] or 'Unknown',os.date('!%a, %d %b %Y %H:%M:%S GMT'),ssdp_server,http_mime[ext] or 'application/x-octet-stream')
    )
    if len then http.send(string.format("Content-Length: %s\r\n",len)) end
    http.send("\r\n")

    if cfg.debug>0 and err~=200 then print('http error '..err) end

end

function get_soap_method(s)
    local i=string.find(s,'#',1,true)
    if not i then return s end
    return string.sub(s,i+1)
end

function plugin_sendurl_from_cache(url,range)
    local c=cache[url]

    if c==nil or c.value==nil then return false end

    if cfg.debug>0 then print('Cache URL: '..c.value) end

    local rc,location,l

    location=c.value

    for i=1,5,1 do
        rc,l=http.sendurl(location,1,range)

        if l then
            location=l
            core.sendevent('store',url,location)
            if cfg.debug>0 then print('Redirect #'..i..' to: '..location) end
        else
            if rc~=0 then return true end

            if cfg.debug>0 then print('Retry #'..i..' location: '..location) end
        end
    end

    return false
end

function plugin_sendurl(url,real_url,range)
    local rc,location,l

    location=real_url

    core.sendevent('store',url,real_url)

    for i=1,5,1 do
        rc,l=http.sendurl(location,1,range)

        if l then
            location=l
            core.sendevent('store',url,location)
            if cfg.debug>0 then print('Redirect #'..i..' to: '..location) end
        else
            if rc~=0 then return true end

            if cfg.debug>0 then print('Retry #'..i..' location: '..location) end
        end
    end

    return false
end

function plugin_sendfile(path)
    local len=util.getflen(path)
    if len then
        http.send(string.format('Content-Length: %s\r\n\r\n',len))
        http.sendfile(path)
    else
        http.send('\r\n')
    end
end

function plugin_download(url)
    local data,location

    location=url

    for i=1,5,1 do
        data,location=http.download(location)

        if not location then
            return data
        else
            if cfg.debug>0 then print('Redirect #'..i..' to: '..location) end
        end
    end

    return nil
end

function plugin_get_length(url)
    local len,location

    location=url

    for i=1,5,1 do
        len,location=http.get_length(location)

        if not location then
            return len
        else
            if cfg.debug>0 then print('Redirect #'..i..' to: '..location) end
        end
    end

    return 0
end

function http_get_action(url)

    local t=split_string(url,'/')

    return t[1] or '', string.match(t[2] or '','^([%w_]+)%.?%w*') or ''

end

local http_ui_main=cfg.ui_path..'xupnpd_ui.lua'

if not util.getflen(http_ui_main) then
    http_ui_main=nil
end

function http_handler(what,from,port,msg)

    if not msg or not msg.reqline then return end

    local pr_name=nil

    if cfg.profiles then
        pr_name=profile_change(msg['user-agent'],msg)

        if msg.reqline[2]=='/dev.xml' then msg.reqline[2]=cfg.dev_desc_xml end
    end

    if msg.reqline[2]=='/' then
        if http_ui_main then msg.reqline[2]='/ui' else msg.reqline[2]='/index.html' end
    end

    local head=false

    local f=util.geturlinfo(cfg.www_root,msg.reqline[2])

    if not f or (msg.reqline[3]~='HTTP/1.0' and msg.reqline[3]~='HTTP/1.1') then
        http_send_headers(400)
        return
    end

    if cfg.debug>0 then print(string.format('%s %s %s "%s" [%s]',from,msg.reqline[1],msg.reqline[2],msg['user-agent'] or '',pr_name or 'generic')) end

    local from_ip=string.match(from,'(.+):.+')

    if string.find(f.url,'^/ui/?') then
        if not http_ui_main then
            http_send_headers(404)
        else
            dofile(http_ui_main)
            ui_handler(f.args,msg.data or '',from_ip,f.url)
        end
        return
    elseif string.find(f.url,'^/app/?') then
        webapp_handler(f.args,msg.data or '',from_ip,f.url)
        return
    end

    if msg.reqline[1]=='HEAD' then head=true msg.reqline[1]='GET' end

    local url,object=http_get_action(f.url)

    -- UPnP SOAP Exchange
    if url=='soap' then

        if not msg.soapaction then msg.soapaction=f.args.action end

        if cfg.debug>0 then print(from..' SOAP '..(msg.soapaction or '')) end

        local err=true

        local s=services[object ]

        if s then
            local func_name=get_soap_method(msg.soapaction or '')
            local func=s[func_name]

            if func then

                if cfg.debug>1 then print(msg.data) end

                local r=soap.find('Envelope/Body/'..func_name,soap.parse(msg.data or ''))

                if not r then r=f.args end

                r=func(r or {},from_ip)

                if r then
                    local resp=
                        string.format(
                            '<?xml version=\"1.0\" encoding=\"utf-8\"?>'..
                            '<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">'..
                            '<s:Body><u:%sResponse xmlns:u=\"%s\">%s</u:%sResponse></s:Body></s:Envelope>',
                                func_name,s.schema,soap.serialize_vector(r),func_name)

                    local resp_len=resp:len() if cfg.soap_length==false then resp_len=nil end

                    http_send_headers(200,'xml',resp_len)

                    http.send(resp)

                    if cfg.debug>2 then print(resp) end

                    err=false
                end
            end
        end

        if err==true then
            local resp=
            '<?xml version=\"1.0\" encoding=\"utf-8\"?>'..
            '<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">'..
               '<s:Body>'..
                  '<s:Fault>'..
                     '<faultcode>s:Client</faultcode>'..
                     '<faultstring>UPnPError</faultstring>'..
                     '<detail>'..
                        '<u:UPnPError xmlns:u=\"urn:schemas-upnp-org:control-1-0\">'..
                           '<u:errorCode>501</u:errorCode>'..
                           '<u:errorDescription>Action Failed</u:errorDescription>'..
                        '</u:UPnPError>'..
                     '</detail>'..
                  '</s:Fault>'..
               '</s:Body>'..
            '</s:Envelope>'

            local resp_len=resp:len() if cfg.soap_length==false then resp_len=nil end

            http_send_headers(200,'xml',resp_len)

            http.send(resp)

            if cfg.debug>0 then print('upnp error 501') end

        end

    -- UPnP Events
    elseif url=='event' then

        if msg.reqline[1]=='SUBSCRIBE' then
            local ttl=cfg.dlna_subscribe_ttl
            local sid=nil
            local callback=nil

            if msg.sid then sid=string.match(msg.sid,'uuid:(.+)') else sid=core.uuid() end
            if msg.callback then callback=string.match(msg.callback,'<(.+)>') end

            if object~='' then
                core.sendevent('subscribe',object,sid,callback,ttl)
            end

            http.send(
                string.format(
                    'HTTP/1.1 200 OK\r\nPragma: no-cache\r\nCache-control: no-cache\r\nDate: %s\r\nServer: %s\r\nAccept-Ranges: none\r\n'..
                    'Connection: close\r\nEXT:\r\nSID: uuid:%s\r\nTIMEOUT: Second-%d\r\n\r\n',
                    os.date('!%a, %d %b %Y %H:%M:%S GMT'),ssdp_server,sid,ttl))
        elseif msg.reqline[1]=='UNSUBSCRIBE' then

            if msg.sid then
                core.sendevent('unsubscribe',string.match(msg.sid,'uuid:(.+)'))
            end

            http.send(
                string.format(
                    'HTTP/1.1 200 OK\r\nPragma: no-cache\r\nCache-control: no-cache\r\nDate: %s\r\nServer: %s\r\nAccept-Ranges: none\r\n'..
                    'Connection: close\r\nEXT:\r\n\r\n',
                    os.date('!%a, %d %b %Y %H:%M:%S GMT'),ssdp_server))
        else
            http_send_headers(404)
        end

    -- UPnP Streaming
    elseif url=='proxy' then

        local pls=find_playlist_object(object)

        if not pls then http_send_headers(404) return end

        local mtype,extras=playlist_item_type(pls)

        http.send(string.format(
            'HTTP/1.1 200 OK\r\nPragma: no-cache\r\nCache-control: no-cache\r\nDate: %s\r\nServer: %s\r\n'..
            'Connection: close\r\nContent-Type: %s\r\nEXT:\r\n',
            os.date('!%a, %d %b %Y %H:%M:%S GMT'),ssdp_server,mtype[3]))

        if cfg.dlna_headers==true then http.send('TransferMode.DLNA.ORG: Streaming\r\nContentFeatures.DLNA.ORG: '..extras..'\r\n') end

        if cfg.content_disp==true then
            http.send(string.format('Content-Disposition: attachment; filename=\"%s.%s\"\r\n',pls.objid,pls.type))      -- string.gsub(pls.name,"[\/#|@&*`']","_")
        end

        if head==true then
            http.send('\r\n')
            http.flush()
        else
            if pls.event then core.sendevent(pls.event,pls.url) end

            if cfg.debug>0 then print(from..' PROXY '..pls.url..' <'..mtype[3]..'>') end

            core.sendevent('status',util.getpid(),from_ip..' '..pls.name)

            if pls.plugin then
                http.send('Accept-Ranges: bytes\r\n')
                http.flush()

                local p=plugins[pls.plugin]

                if p and p.disabled~=true then p.sendurl(pls.url,msg.range) end
            else
                if cfg.wdtv==true then
                    http.send('Content-Size: 65535\r\n')
                    http.send('Content-Length: 65535\r\n')
                end

                http.send('Accept-Ranges: none\r\n\r\n')

                if string.find(pls.url,'^udp://@') then
                    http.sendmcasturl(string.sub(pls.url,8),cfg.mcast_interface,2048)
                else
                    local rc,location
                    location=pls.url
                    for i=1,5,1 do
                        rc,location=http.sendurl(location)
                        if not location then
                            break
                        else
                            if cfg.debug>0 then print('Redirect #'..i..' to: '..location) end
                        end
                    end
                end
            end
        end

    -- UPnP AlbumArt
    elseif url=='logo' then

        local pls=find_playlist_object(object)

        if not pls or not pls.logo then http_send_headers(404) return end

        http.send(string.format(
            'HTTP/1.1 200 OK\r\nDate: %s\r\nServer: %s\r\nConnection: close\r\nAccept-Ranges: none\r\nContent-Type: %s\r\nEXT:\r\n',
            os.date('!%a, %d %b %Y %H:%M:%S GMT'),ssdp_server,http_mime['jpg']))

        if cfg.dlna_headers==true then http.send('ContentFeatures.DLNA.ORG: DLNA.ORG_PN=JPEG_TN\r\n') end

        if head==true then
            http.send('\r\n')
        else
            if cfg.debug>0 then print(from..' LOGO '..pls.logo) end

            http.sendurl(pls.logo,1)
        end

    -- Subtitle
    elseif url=='sub' then

        local pls=find_playlist_object(object)

        if not pls or not pls.path then http_send_headers(404) return end

        local path=string.gsub(pls.path,'.%w+$','.srt')

        local flen=util.getflen(path)

        if not flen then http_send_headers(404) return end

        http.send(string.format(
            'HTTP/1.1 200 OK\r\nDate: %s\r\nServer: %s\r\nConnection: close\r\nAccept-Ranges: none\r\nContent-Type: %s\r\nContent-Length: %s\r\nEXT:\r\n\r\n',
            os.date('!%a, %d %b %Y %H:%M:%S GMT'),ssdp_server,http_mime['srt'],flen))

        if head~=true then
            if cfg.debug>0 then print(from..' SUB '..path) end

            http.sendfile(path)
        end

    -- UPnP Local files streaming
    elseif url=='stream' then

        local pls=find_playlist_object(object)

        if not pls or not pls.path then http_send_headers(404) return end

        local flen=pls.length

        local ffrom=0
        local flen_total=flen

        if msg.range and flen and flen>0 then
            local f,t=string.match(msg.range,'bytes=(.*)-(.*)')

            f=tonumber(f)
            t=tonumber(t)

            if not f then f=0 end
            if not t then t=flen-1 end

            if f>t or t+1>flen then http_send_headers(416) return end

            ffrom=f
            flen=t-f+1
        end

        local mtype,extras=playlist_item_type(pls)

        http.send(string.format(
            'HTTP/1.1 200 OK\r\nPragma: no-cache\r\nCache-control: no-cache\r\nDate: %s\r\nServer: %s\r\n'..
            'Connection: close\r\nContent-Type: %s\r\nEXT:\r\n',
            os.date('!%a, %d %b %Y %H:%M:%S GMT'),ssdp_server,mtype[3]))

        if flen then
            http.send(string.format('Accept-Ranges: bytes\r\nContent-Length: %s\r\n',flen))
        else
            http.send('Accept-Ranges: none\r\n')
        end

        if cfg.dlna_headers==true then http.send('TransferMode.DLNA.ORG: Streaming\r\nContentFeatures.DLNA.ORG: '..extras..'\r\n') end

        if cfg.content_disp==true then
            http.send(string.format('Content-Disposition: attachment; filename=\"%s.%s\"\r\n',pls.objid,pls.type))
        end

        if msg.range and flen and flen>0 then
            http.send(string.format('Content-Range: bytes %s-%s/%s\r\n',ffrom,ffrom+flen-1,flen_total))
        end

        http.send('\r\n')
        http.flush()

        if head~=true then
            if pls.event then core.sendevent(pls.event,pls.path) end

            if cfg.debug>0 then print(from..' STREAM '..pls.path..' <'..mtype[3]..'>') end

            core.sendevent('status',util.getpid(),from_ip..' '..pls.name)

            http.sendfile(pls.path,ffrom,flen)
        end

    else
        if f.type=='none' then http_send_headers(404) return end
        if f.type~='file' then http_send_headers(403) return end

        local tmpl_name=nil

        for i,fname in ipairs(http_templ) do
            if f.url==fname then tmpl_name=cfg.tmp_path..'xupnpd-cache'..fname break end
        end

        local len=nil

        if not tmpl_name then len=f.length else len=util.getflen(tmpl_name) end

        http.send(
            string.format(
                'HTTP/1.1 200 OK\r\nDate: %s\r\nServer: %s\r\nAccept-Ranges: none\r\nConnection: close\r\nContent-Type: %s\r\nEXT:\r\n',
                os.date('!%a, %d %b %Y %H:%M:%S GMT'),ssdp_server,http_mime[f.ext] or 'application/x-octet-stream'))

        if len then
            http.send(string.format('Content-Length: %s\r\n',len))
        end

        http.send('\r\n')

        if head~=true then
            if cfg.debug>0 then print(from..' FILE '..f.path) end

            if tmpl_name~=nil then
                if len then http.sendfile(tmpl_name) else http.sendtfile(f.path,http_vars) end
            else
                http.sendfile(f.path)
            end
        end
    end

    http.flush()
end

compile_templates()

events["http"]=http_handler

http.listen(cfg.http_port,"http")
