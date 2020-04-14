-- Copyright (C) 2011-2012 Anton Burdinuk
-- clark15b@gmail.com
-- https://tsdemuxer.googlecode.com/svn/trunk/xupnpd


ui_args=nil
ui_data=nil
dofile(cfg.ui_path.."helper.lua")
function ui_main()
    http.sendtfile(cfg.ui_path..'ui_main.html',http_vars)
end

function ui_error()
    http.send('<h2>Error occurred</h3>')
    http.send('<br/><a class="btn btn-info" href="/ui/">Back</a>')
end

function ui_downloads()
    http.send('<h3>Downloads</h3>')
    http.send('<br/><table class="table">')
    if playlist_data.elements[1] then
        for i,j in ipairs(playlist_data.elements[1].elements) do
            http.send(string.format('<tr><td><a href="/ui/%s.m3u">%s</a></td></tr>',j.objid,j.name))
        end
    end
    http.send('</table>')
    http.send('<br/><a class="btn btn-info" href="/ui">Back</a>')
end

function ui_playlist_get_url(pls)
        return string.format('%s/proxy/%s.%s',cfg.extern_url or www_location,pls.objid,pls.type)
end

function ui_download(name)
    name=util.urldecode(string.match(name,'(.+)%.m3u$'))

    local pls=find_playlist_object(name)

    if not pls then
        http.send(
            string.format(
                'HTTP/1.1 404 Not found\r\nPragma: no-cache\r\nCache-control: no-cache\r\nDate: %s\r\nServer: %s\r\n'..
                'Connection: close\r\n\r\n',os.date('!%a, %d %b %Y %H:%M:%S GMT'),ssdp_server)
        )
        return
    end

    http.send(
        string.format(
            'HTTP/1.1 200 Ok\r\nPragma: no-cache\r\nCache-control: no-cache\r\nDate: %s\r\nServer: %s\r\nAccept-Ranges: none\r\n'..
            'Connection: close\r\nContent-Type: audio/x-mpegurl\r\nContent-Disposition: attachment; filename=\"%s.m3u\"\r\n\r\n',os.date('!%a, %d %b %Y %H:%M:%S GMT'),ssdp_server,pls.name)
    )

    http.send('#EXTM3U\n')
    for i,j in ipairs(pls.elements) do
        http.send('#EXTINF:0,'..j.name..'\n'..ui_playlist_get_url(j)..'\n')
    end
end

function ui_playlists()
    http.send('<h3>Playlists</h3>')
    http.send('<br/><table class="table">')

    function f(path,args)
        local d=util.dir(path)
        if d then
            table.sort(d)
            for i,j in ipairs(d) do
                if string.find(j,'.+%.m3u$') then
                    local fname=util.urlencode(j)
                    http.send(string.format('<tr><td><a href=\'/ui/show?fname=%s&%s\'>%s</a> [<a href=\'/ui/remove?fname=%s&%s\'>x</a>]</td></tr>\n',fname,args,j,fname,args))
                end
            end
        end
    end

    f(cfg.playlists_path,'')

    if cfg.feeds_path~=cfg.playlists_path then f(cfg.feeds_path,'feed=1') end

    http.send('</table>')

    http.send('<br/><h3>Upload *.m3u file</h3>')
    http.send('<form method=post action="/ui/upload" enctype="multipart/form-data">')
    http.send('<input type=file name=m3ufile><br /><br />')
    http.send('<input class="btn btn-primary" type=submit value=Send>')
    http.send('</form><hr/>')
    http.send('<br/><a class="btn btn-primary" href="/ui/reload">Reload</a> <a class="btn btn-primary" href="/ui/reload_feeds?return_url=/ui/playlists">Reload feeds</a> <a class="btn btn-info" href="/ui">Back</a>')
end

function ui_feeds()
    http.send('<h3>Feeds</h3>')
    http.send('<br/><table class="table">')

    for i,j in ipairs(feeds) do
        http.send(string.format('<tr><td>%s [<a href="/ui/remove_feed?id=%s">x</a>]</td></tr>\n',j[3],i))
    end

    http.send('</table>')

    http.send('<h3>Add feed</h3>')

    http.send('<form method=get action="/ui/add_feed">')

    http.send('<div class="controls controls-row"><div class="span2">Plugin</div><select class="span4" name="plugin">')

    for plugin_name,plugin in pairs(plugins) do
        if plugin.name and plugin.disabled~=true then
            http.send(string.format('<option value="%s">%s</option>',plugin_name,plugin.name))
        end
    end

    http.send('</select></div>')

    http.send('<div class="controls controls-row"><div class="span2">Feed data</div><input class="span4" type="text" name="feed"><div class="span1"><a href="/ui/fhelp" target="_blank">?</a></div></div>')

    http.send('<div class="controls controls-row"><div class="span2">Name</div><input class="span4" type="text" name="name"></div>')

    http.send('<br/><input class="btn btn-primary" type=submit value=Add> <a class="btn btn-info" href="/ui/fhelp" target="_blank">Help</a>')
    http.send('</form><hr/>')

    http.send('<br/><a class="btn btn-primary" href="/ui/save_feeds">Save</a> <a class="btn btn-primary" href="/ui/reload_feeds?return_url=/ui/feeds">Reload feeds</a> <a class="btn btn-info" href="/ui">Back</a>')
end

function ui_fhelp()

    http.send('<br/>')

    for plugin_name,plugin in pairs(plugins) do
        if plugin.name and plugin.desc and plugin.disabled~=true then
            http.send(string.format('<b>%s</b>: ',plugin.name))
            http.send(plugin.desc)
            http.send('<br/><br/>\n\n')
        end
    end
end

function ui_mhelp()

    http.send('<br/><h3>MIME-Types</h3>\n<table class="table">\n<tr><th>File Type</th><th>MIME-Type</th><th>UPnP Proto</th><th>DLNA.ORG Profile</th></tr>\n')

    for i,j in pairs(mime) do
        if j then
            local ext=string.match(j[5],'^(DLNA.ORG_PN=[%w_]+);')

            http.send(string.format('<tr><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr>\n',i,j[3],j[4],ext or j[5]))
        end
    end

    http.send('</table>\n\n<a class="btn btn-info" href="/ui/ehelp">DLNA Extras</a> <a class="btn btn-info" href="/ui/config">Back</a>')
end

function ui_ehelp()
    http.send('<br/><h3>DLNA Extras</h3>\n<table class="table">\n<tr><th>Name</th><th>DLNA.ORG Extras</th></tr>\n')

    for i,j in pairs(dlna_org_extras) do
        http.send(string.format('<tr><td>%s</td><td>%s</td></tr>\n',i,j))
    end

    http.send('</table>\n\n<a class="btn btn-info" href="/ui/mhelp">Back</a>')
end

function ui_plugins()

    if ui_args.n and ui_args.s then
        local p=plugins[ui_args.n]

        if p then
            if ui_args.s=='on' then p.disabled=false elseif ui_args.s=='off' then p.disabled=true end
            core.sendevent('plugin',ui_args.n,ui_args.s)
        end
    end

    http.send('<br/><h3>Plugins</h3>\n<table class="table">\n<tr><th>Name</th><th>Status</th></tr>\n')

    for i,j in pairs(plugins) do
        if j.name then
            local status

            if j.disabled==true then
                status=string.format('<a href="/ui/plugins?n=%s&s=on">on</a> | <b>off</b>',util.urlencode(i))
            else
                status=string.format('<b>on</b> | <a href="/ui/plugins?n=%s&s=off">off</a>',util.urlencode(i))
            end

            http.send(string.format('<tr><td>%s</td><td>%s</td></tr>\n',j.name,status))
        end
    end

    http.send('</table>\n\n<a class="btn btn-primary" href="/ui/plapply">Save</a> <a class="btn btn-info" href="/ui/config">Back</a>')
end

function ui_plapply()
    local f=io.open(cfg.config_path..'postinit/plugins.lua','w')
    if f then
        for i,j in pairs(plugins) do
            if j.name then
                f:write(string.format('plugins["%s"].disabled=%s\n',i,tostring(j.disabled or false)))
            end
        end
        f:close()
    end

    http.send('<h3>OK</h3>')
    http.send('<br/><a class="btn btn-info" href="/ui/plugins">Back</a>')
end


function ui_profiles()

    if ui_args.n and ui_args.s then
        local p=profiles[ui_args.n]

        if p then
            if ui_args.s=='on' then p.disabled=false elseif ui_args.s=='off' then p.disabled=true end
            core.sendevent('profile',ui_args.n,ui_args.s)
        end
    end

    http.send('<br/><h3>Profiles</h3>\n<table class="table">\n<tr><th>Name</th><th>Status</th></tr>\n')

    for i,j in pairs(profiles) do
        local status

        if j.disabled==true then
            status=string.format('<a href="/ui/profiles?n=%s&s=on">on</a> | <b>off</b>',util.urlencode(i))
        else
            status=string.format('<b>on</b> | <a href="/ui/profiles?n=%s&s=off">off</a>',util.urlencode(i))
        end

        http.send(string.format('<tr><td>%s</td><td>%s</td></tr>\n',j.desc,status))
    end

    http.send('</table>\n\n<a class="btn btn-primary" href="/ui/prapply">Save</a> <a class="btn btn-info" href="/ui/config">Back</a>')
end

function ui_prapply()
    local f=io.open(cfg.config_path..'postinit/profiles.lua','w')
    if f then
        for i,j in pairs(profiles) do
            f:write(string.format('profiles["%s"].disabled=%s\n',i,tostring(j.disabled or false)))
        end
        f:close()
    end

    http.send('<h3>OK</h3>')
    http.send('<br/><a class="btn btn-info" href="/ui/profiles">Back</a>')
end

function ui_show()
    if ui_args.fname then
        local real_name=util.urldecode(ui_args.fname)
        if string.find(real_name,'^[^/\\]+%.m3u$') then

            local path=cfg.playlists_path
            if ui_args.feed=='1' then path=cfg.feeds_path end

            local pls=m3u.parse(path..real_name)

            if pls then
                http.send('<h3>'..pls.name..'</h3>')
                http.send('<br/><table class="table">')
                for i,j in ipairs(pls.elements) do
                    http.send(string.format('<tr><td><a href="%s">%s</a></td></tr>',j.url,j.name))
                end
                http.send('</table>')
            end
        end
    end

    http.send('<br/><a class="btn btn-info" href="/ui/playlists">Back</a>')
end

function ui_remove()
    if ui_args.fname then
        local real_name=util.urldecode(ui_args.fname)
        if string.find(real_name,'^[^-/\\]+%.m3u$') then

            local path=cfg.playlists_path
            if ui_args.feed=='1' then path=cfg.feeds_path end

--            if os.remove(path..real_name) then
            if os.execute(string.format('rm -f %s%s',path,real_name)) then
                core.sendevent('reload')
                http.send('<h3>OK</h3>')
            else
                http.send('<h3>Fail</h3>')
            end
        end
    end

    http.send('<br/><a class="btn btn-info" href="/ui/playlists">Back</a>')
end

function ui_remove_feed()
    if ui_args.id and feeds[tonumber(ui_args.id)] then
        core.sendevent('remove_feed',ui_args.id)
        http.send('<h3>OK</h3>')
    else
        http.send('<h3>Fail</h3>')
    end

    http.send('<br/><a class="btn btn-info" href="/ui/feeds">Back</a>')
end

function ui_add_feed()
    if ui_args.plugin and ui_args.feed then
        if not ui_args.name or string.len(ui_args.name)==0 then ui_args.name=ui_args.plugin..' '..string.gsub(ui_args.feed,'/',' ') end
        core.sendevent('add_feed',ui_args.plugin,ui_args.feed,ui_args.name)
        http.send('<h3>OK</h3>')
    else
        http.send('<h3>Fail</h3>')
    end

    http.send('<br/><a class="btn btn-info" href="/ui/feeds">Back</a>')
end

function save_feeds()

    local f=io.open(cfg.config_path..'feeds.lua','w')
    if f then
        f:write('feeds=\n{\n')

        for i,j in ipairs(feeds) do
            f:write(string.format('   { "%s", "%s", "%s" },\n',j[1],j[2],j[3]))
        end

        f:write('}\n')

        f:close()
        return true
    end

    return false
end


function ui_save_feeds()
    if save_feeds() then http.send('<h3>OK</h3>') else http.send('<h3>Fail</h3>') end

    http.send('<br/><a class="btn btn-info" href="/ui/feeds">Back</a>')
end

function ui_reload()
    core.sendevent('reload')
    http.send('<h3>OK</h3>')
    http.send('<br/><a class="btn btn-info" href="/ui/playlists">Back</a>')
end

function ui_reload_feeds()
    update_feeds_async()
    http.send('<h3>OK</h3>')
    http.send('<br/><a class="btn btn-info" href="'.. (ui_args.return_url or '/ui') ..'">Back</a>')
end

function ui_config()
    for plugin_name,plugin in pairs(plugins) do
        if plugin.ui_vars then
            for i,var in ipairs(plugin.ui_vars) do
                http_vars[ var[1] ]=var[2]
            end
        end
    end


    http.sendtfile(cfg.ui_path..'ui_config.html',http_vars)

    http.send('<script>\n')
    http.send('function set_select_value(id,value)\n')
    http.send('    { var obj=document.getElementById(id); for(i=0;i<obj.length;i++) { if(obj.options[i].value==value) { obj.options[i].selected=true; break; } } }\n')
    http.send('function set_input_value(id,value)\n')
    http.send('    { var obj=document.getElementById(id); if(obj) { obj.value=value } }\n')

    for plugin_name,plugin in pairs(plugins) do
        if plugin.ui_config_vars then
            for i,var in ipairs(plugin.ui_config_vars) do
                http.send(string.format('set_%s_value("%s","%s");\n',var[1],var[2],tostring(cfg[ var[2] ])))
            end
        end
    end

    http.send('</script>\n')

end

function ui_apply()

    local args=util.parse_postdata(ui_data)

    local f=io.open(cfg.config_path..'common.lua','w')
    if f then

        for plugin_name,plugin in pairs(plugins) do
            if plugin.ui_config_vars then
                for i,var in ipairs(plugin.ui_config_vars) do
                    local v=args[ var[2] ]
                    local t=var[3]

                    if not v then if t=="int" then v=0 elseif t=="bool" then v=false else v="" end end

                    if t=="int" or t=="bool" then
                        f:write(string.format('cfg["%s"]=%s\n',var[2],tostring(v)))
                    else
                        f:write(string.format('cfg["%s"]="%s"\n',var[2],v))
                    end
                end
            end
        end

        f:close()
        core.sendevent('config')
    end

    http.send('<h3>OK</h3>')
    http.send('<br/><a class="btn btn-info" href="/ui/config">Back</a>')
end

function ui_status()
    http.send('<h3>Status</h3>')
    http.send('<br/><table class="table">')

    for i,j in pairs(childs) do
        if j.status then
            http.send(string.format('<tr><td>%s [<a href="/ui/kill?pid=%s">x</a>]</td></tr>',j.status,i))
        end
    end

    http.send('</table>')

    http.send('<br/><a class="btn btn-primary" href="/ui/status">Refresh</a> <a class="btn btn-info" href="/ui">Back</a>')
end

function ui_kill()
    if ui_args.pid and childs[tonumber(ui_args.pid)] then
        util.kill(ui_args.pid)
        http.send('<h3>OK</h3>')
    else
        http.send('<h3>Fail</h3>')
    end
    http.send('<br/><a class="btn btn-info" href="/ui/status">Back</a>')
end

function ui_upload()
    local tt=util.multipart_split(ui_data)
    ui_data=nil

    if tt and tt[1] then
        local n,m=string.find(tt[1],'\r?\n\r?\n')

        if n then
            local fname=string.match(string.sub(tt[1],1,n-1),'filename=\"(.+)\"')

            if fname and string.find(fname,'.+%.m3u$') then
                local tfname=cfg.tmp_path..fname

                local fd=io.open(tfname,'w+')
                if fd then
                    fd:write(string.sub(tt[1],m+1))
                    fd:close()
                end

                local pls=m3u.parse(tfname)

                if pls then
                    if os.execute(string.format('mv "%s" "%s"',tfname,cfg.playlists_path..fname))~=0 then
                        os.remove(tfname)
                        http.send('<h3>Fail</h3>')
                    else
                        core.sendevent('reload')
                        http.send('<h3>OK</h3>')
                    end
                else
                    os.remove(tfname)
                    http.send('<h3>Fail</h3>')
                end
            else
                http.send('<h3>Fail</h3>')
            end
        end
    end

    http.send('<br/><a class="btn btn-info" href="/ui/playlists">Back</a>')
end

function ui_api_call(args)
    http_send_headers(200,'txt')

    if args.action=='feeds' then
        for i,j in ipairs(feeds) do http.send(string.format('%s;%s\r\n',i,j[3])) end
    elseif args.action=='reload' then
        core.sendevent('reload')
        http.send('OK\r\n')
    elseif args.action=='add_feed' then
        core.sendevent('add_feed',args.plugin or '',args.feed or '',args.name or '')
        http.send('OK\r\n')
    elseif args.action=='remove_feed' then
        core.sendevent('remove_feed',args.id)
        http.send('OK\r\n')
    elseif args.action=='save_feeds' then
        save_feeds()
        update_feeds_async()
        http.send('OK\r\n')
    elseif args.action=='update_feeds' then
        update_feeds_async()
        http.send('OK\r\n')
    elseif args.action=='status' then
        for i,j in pairs(childs) do
            if j.status then
                http.send(string.format('%s;%s\r\n',i,j.status))
            end
        end
    elseif args.action=='kill' then
        if args.pid and childs[tonumber(args.pid)] then
            util.kill(args.pid)
            http.send('OK\r\n')
        else
            http.send('ERR\r\n')
        end
    elseif args.action=='playlists' then
        for i,j in ipairs(playlist_data.elements[1].elements) do
            http.send(string.format('%s;%s\r\n',i,j.name))
        end
    elseif args.action=='playlist' then
        local pls=playlist_data.elements[1].elements[tonumber(args.id)]
        if pls then
            for i,j in ipairs(pls.elements) do
                http.send(string.format('%s;%s;%s;%s\r\n',i,j.logo or '',j.name,playlist_get_url(j)))
            end
        end
    else
        http.send('ERR\r\n')
    end
end

function ui_restart()
    if core.restart(cfg.pid_file,"./xupnpd") then http.send('<h3>Attempt to restart...</h3>') else http.send('<h3>Unable to restart.</h3>') end

    http.send('<br/><form method=get action="/ui"><input class="btn btn-primary" type=submit value=OK></form>')

    http.send('<script>setTimeout("document.forms[0].submit()",3000)</script>')
end

function ui_logout()
    os.remove(cfg.ui_session_file)
    http.send('<meta http-equiv="refresh" content="0;URL=/"ui//"" />')
end

function ui_login()
    password_enc = util.md5_string_hash(ui_data)
    if password_enc == auth then
        compare_session_id = 'session_id=' .. generate_session_id(35)
        write_file(cfg.ui_session_file, compare_session_id)
        http_data=string.format('<script>document.cookie="%s"</script>', compare_session_id)
	http.send(http_data)
        http.send('<meta http-equiv="refresh" content="0;URL=/"ui//"" />')
    else
        http.send('<h2>Wrong password</h2>')
    end
end

function ui_set_password()
    if ui_data and ui_data ~= '' then
        password_enc = util.md5_string_hash(ui_data)
        write_file(cfg.ui_auth_file, password_enc)
        os.remove(cfg.ui_session_file)
        http.send('<meta http-equiv="refresh" content="3;URL=/"ui//"" />')
        http.send('<h4>Password changed, you will be redirected in 3 secons, please login again.</h4>')
    else
        http.send('<h3>Set / change access password.</h3>')
        http.send('<form class="form-inline my-2 my-lg-0" method="post" action="/ui/set_password">')
        http.send('<input class="form-control mr-sm-2" name="password" type="password" placeholder="New Password"> ')
        http.send('<button class="btn btn-success my-2 my-sm-0" type="submit">Change</button>')
        http.send('</form>')
    end
end

function generate_session_id(length)
        local res = ""
        for i = 1, length do
                res = res .. string.char(math.random(97, 122))
        end
        return res
end

function read_file (file)
    local content
    local f = io.open(file, "r")
    if f then
        local fa = assert(f)
        content = string.gsub(fa:read("*all"),'\n','')
        f:close()
    end
    return content
end

function write_file (file, data)
  local write_handle = io.open(file, "w")
  write_handle:write(data)
  write_handle:close()
end

ui_actions=
{
    ['main']            = { 'xupnpd', ui_main },
    ['error']           = { 'xupnpd - error', ui_error },
    ['downloads']       = { 'xupnpd - downloads', ui_downloads },
    ['playlists']       = { 'xupnpd - playlists', ui_playlists },
    ['feeds']           = { 'xupnpd - feeds', ui_feeds },
    ['show']            = { 'xupnpd - show', ui_show },
    ['remove']          = { 'xupnpd - remove', ui_remove },
    ['remove_feed']     = { 'xupnpd - remove feed', ui_remove_feed },
    ['reload']          = { 'xupnpd - reload', ui_reload },
    ['reload_feeds']    = { 'xupnpd - reload feeds', ui_reload_feeds },
    ['save_feeds']      = { 'xupnpd - save feeds', ui_save_feeds },
    ['add_feed']        = { 'xupnpd - add feed', ui_add_feed },
    ['config']          = { 'xupnpd - config', ui_config },
    ['status']          = { 'xupnpd - status', ui_status },
    ['kill']            = { 'xupnpd - kill', ui_kill },
    ['upload']          = { 'xupnpd - upload', ui_upload },
    ['apply']           = { 'xupnpd - apply', ui_apply },
    ['plugins']         = { 'xupnpd - plugins', ui_plugins },
    ['plapply']         = { 'xupnpd - plugins apply', ui_plapply },
    ['profiles']        = { 'xupnpd - profiles', ui_profiles },
    ['prapply']         = { 'xupnpd - profiles apply', ui_prapply },
    ['fhelp']           = { 'xupnpd - feeds help', ui_fhelp },
    ['mhelp']           = { 'xupnpd - mimes help', ui_mhelp },
    ['ehelp']           = { 'xupnpd - extras help', ui_ehelp },
    ['restart']         = { 'xupnpd - restart', ui_restart },
    ['login']           = { 'xupnpd - login', ui_login },
    ['logout']          = { 'xupnpd - logout', ui_logout },
    ['set_password']    = { 'xupnpd - set password', ui_set_password}
}

function ui_handler(args,data,ip,url,methtod,cookie)
    for plugin_name,plugin in pairs(plugins) do
        if plugin.ui_actons then
            for act_name,act in pairs(plugin.ui_actons) do
                ui_actions[act_name]=act
            end
        end
    end

    local action=string.match(url,'^/ui/(.+)$')

    if action then
        local  path_file , file_format =string.match(action, "(.+%.(%w+))[%?]?.*$")

	if  file_format == 'm3u' then
		ui_download(action)
		return
	elseif file_format then
		http_send_headers(200,file_format)
		http.sendfile(cfg.ui_path..path_file)
		return
	end

	if action == "api" then
	 	ui_api_call(args)
		return
	end

	if string.find(action,"api_v2") then
		dofile(cfg.ui_path.."api_v2.lua")

		ui_api_v_2_call(args,data,ip, string.gsub(url, "/ui/api_v2/", ''),methtod)
		return
	end

    else
	action='main'
    end

    http_send_headers(200,'html')

    local act=ui_actions[action]

    if not act then act=ui_actions['error'] end

    auth=read_file(cfg.ui_auth_file)

    if auth then
        local compare_session_id=read_file(cfg.ui_session_file)
        if cookie == compare_session_id and compare_session_id ~= nil then
            is_logged_in = true
        else
            is_logged_in = false
        end
    else
        -- if auth isn't there everybody is logged in
        is_logged_in = true
    end

    if is_logged_in == false and action ~= 'login' then act=ui_actions['main'] end

    http_vars.title=act[1]
    http_vars.content=act[2]
    if is_logged_in == true then
        http_vars.login = 'none'
        http_vars.logout = 'block'
    else
        http_vars.login = 'block'
        http_vars.logout = 'none'
    end

    ui_args=args
    ui_data=data

    http.sendtfile(cfg.ui_path..'ui_template.html',http_vars)
end


plugins["ui"]={}
plugins.ui.ui_config_vars=
{
    { "input",  "ssdp_interface" },
    { "input",  "ssdp_notify_interval", "int" },
    { "input",  "ssdp_max_age", "int" },
    { "input",  "http_port", "int" },
    { "input",  "mcast_interface" },
    { "select", "proxy", "int" },
    { "input",  "user_agent" },
    { "input",  "http_timeout", "int" },
    { "select", "dlna_notify", "bool"},
    { "input",  "dlna_subscribe_ttl", "int"},
    { "select", "group", "bool" },
    { "select", "sort_files", "bool" },
    { "input",  "name" },
    { "input",  "uuid" },
    { "input",  "default_mime_type" },
    { "input",  "feeds_update_interval", "int" },
    { "input",  "playlists_update_interval", "int" },
    { "input",  "drive" }
}
