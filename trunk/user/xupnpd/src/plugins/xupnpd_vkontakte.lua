-- Copyright (C) 2011-2012 I. Sokolov
-- happy.neko@gmail.com
-- Licensed under GNU GPL version 2 
-- https://www.gnu.org/licenses/gpl-2.0.html

-- config example:
--feeds=
--{
--    { "vkontakte", "my", "VK - Videos from my page" },
--    { "vkontakte", "group/group_id", "VK - Videos from group" },
--    { "vkontakte", "group/group_id/album_id", "VK - Videos from group's album" },
--    { "vkontakte", "user/user_id", "VK - Videos from user" },
--    { "vkontakte", "user/user_id/album_id", "VK - Videos from user's album" },
--    { "vkontakte", "search/searchstring", "VK - Search videos" },
--    { "vkontakte", "search_hd/searchstring", "VK - Search only HD videos" }
--}

-- Workaround for private videos and groups
cfg.vk_private_workaround=true
-- How many videos to pull from the feed (max 200)
cfg.vk_video_count=100

vk_app_id='2432090'
vk_app_scope='groups,friends,video,offline,nohttps'
vk_api_server='http://api.vk.com'
vk_api_request_video_get='/method/video.get?width=160'
vk_api_request_video_add='/method/video.add?'
vk_api_request_video_delete='/method/video.delete?'
vk_api_request_video_search='/method/video.search?'
vk_api_request_get_user_info='/method/getProfiles?'
vk_api_request_get_user_groups='/method/groups.get?'
vk_api_request_get_user_friends='/method/friends.get?'

max_video_title_length=120
private_or_disabled_video_matcher='(position:absolute; top:50%%; text%-align:center;)'
-- ffmpeg -loop_input -f image2 -i vk-flash-format-only.png -r 25 -vframes 25 -an -vcodec libx264 -crf 32 -threads 0 vk-flash-format-only.mp4
error_file_private_video='www/vk-private-video.mp4'
error_file_video_flash_format_only='www/vk-flash-format-only.mp4'


function vk_updatefeed(feed,friendly_name)
    local rc=false

    if not vk_is_signed_in() then
        return false -- TODO show nice please sign-in message
    end

    local feed_name='vk_'..string.gsub(feed,'[/%s]','_')
    local feed_m3u_path=cfg.feeds_path..feed_name..'.m3u'
    local tmp_m3u_path=cfg.tmp_path..feed_name..'.m3u'

    local feed_url=nil
    local vk_api_request=nil
    local tfeed=split_string(feed,'/')

    local logo_label="thumb"
    if tfeed[1]=='group' then
        if tfeed[3] then
            feed_url=vk_api_format_url(vk_api_request_video_get,{['count']=cfg.vk_video_count,['gid']=tfeed[2],['aid']=tfeed[3]})
        else
            feed_url=vk_api_format_url(vk_api_request_video_get,{['count']=cfg.vk_video_count,['gid']=tfeed[2]})
        end     
        logo_label="image"
    elseif tfeed[1]=='user' then
        if tfeed[3] then
            feed_url=vk_api_format_url(vk_api_request_video_get,{['count']=cfg.vk_video_count,['uid']=tfeed[2],['aid']=tfeed[3]})
        else
            feed_url=vk_api_format_url(vk_api_request_video_get,{['count']=cfg.vk_video_count,['uid']=tfeed[2]})
        end     
        logo_label="image"
    elseif tfeed[1]=='search' then
        feed_url=vk_api_format_url(vk_api_request_video_search,
            {['count']=cfg.vk_video_count, ['sort']=get_sort_order(tfeed[2]), ['q']=tfeed[3] })
    elseif tfeed[1]=='search_hd' then
        feed_url=vk_api_format_url(vk_api_request_video_search,
            {['count']=cfg.vk_video_count, ['hd']='1', ['sort']=get_sort_order(tfeed[2]), ['q']='"'..tfeed[3]..'"' })
    else
        -- my
        feed_url=vk_api_format_url(vk_api_request_video_get,{['count']=cfg.vk_video_count,['uid']=cfg.vk_api_user_id})
        logo_label="image"
    end

    local feed_data=http.download(feed_url)
    if feed_data then
        local x=json.decode(feed_data)
        feed_data=nil

        if vk_check_response_for_errors(x) then
            return false
        end

        if x and x.response then
            local dfd=io.open(tmp_m3u_path,'w+')
            if dfd then
                dfd:write('#EXTM3U name=\"',friendly_name or feed_name,'\" type=mp4 plugin=vkontakte\n')

                for i,j in pairs(x.response) do
                    if type(j) == 'table' then
                        dfd:write('#EXTINF:0 logo=',string.gsub(j[logo_label],"vkontakte%.ru/","vk.com/"),',',string.sub(unescape_html_string(j.title),1,max_video_title_length),'\n',string.gsub(j.player,"vkontakte%.ru/","vk.com/"),'\n')
                    end
                end
                dfd:close()

                if util.md5(tmp_m3u_path)~=util.md5(feed_m3u_path) then
                    if os.execute(string.format('mv %s %s',tmp_m3u_path,feed_m3u_path))==0 then
                        if cfg.debug>0 then print('VK feed \''..feed_name..'\' updated') end
                        rc=true
                    end
                else
                    util.unlink(tmp_m3u_path)
                end
            end
        end
    end

    return rc
end

function vk_sendurl(vk_page_url,range)
    local url=nil

    if plugin_sendurl_from_cache(vk_page_url,range) then return end

    local clip_page=http.download(vk_page_url)

    if clip_page and string.match(clip_page, private_or_disabled_video_matcher) then
		if cfg.debug>0 then 
			print('VK external player page '..vk_page_url..' is protected by privacy settings or disabled')
		end
		if cfg.vk_private_workaround then
            if cfg.debug>0 then 
				print('Trying workaround for private videos...')
			end
			local private_player_page=nil
	        local oid=string.match(vk_page_url,"oid=([%-%d]+)")
	        local vid=string.match(vk_page_url,"%Aid=(%d+)")
			local added_vid = vk_video_add(vid, oid)
			if added_vid then private_player_page=vk_video_get_external_player_page(added_vid, cfg.vk_api_user_id)	end
			if private_player_page then
			    clip_page=http.download(private_player_page)
				if added_vid then vk_video_delete(added_vid, cfg.vk_api_user_id) end
			else
				if added_vid then vk_video_delete(added_vid, cfg.vk_api_user_id) end
            	if cfg.debug>0 then 
					print('Workaround for private videos failed')
				end
				clip_page=nil
				plugin_sendfile(error_file_private_video)
				return
			end
		else
			clip_page=nil
			plugin_sendfile(error_file_private_video)
			return
		end
	end

    if clip_page then
        local host=string.match(clip_page,"video_host%s*=%s*'(.-)'")
        local uid=string.match(clip_page,"uid%s*=%s*'(%w-)'")
        local vtag=string.match(clip_page,"vtag%s*=%s*'([%w%-_]-)'")
        local no_flv=string.match(clip_page,"video_no_flv%s*=%s*(%d-)%D")
        local max_hd=string.match(clip_page,"video_max_hd%s*=%s*'(%d-)'")	
        url=vk_video_get_direct_download_link(host, uid, vtag, no_flv, max_hd)
    else
        if cfg.debug>0 then print('VK external player page '..vk_page_url..' can not be downloaded or private') end
    end
	clip_page=nil

    if url then		
        if cfg.debug>0 then print('VK Clip download URL: '..url) end
		if url == error_file_video_flash_format_only then
			plugin_sendfile(error_file_video_flash_format_only)
		else
	        plugin_sendurl(vk_page_url,url,range)
		end
    else
        if cfg.debug>0 then print('VK Clip real URL is not found') end
    end
end

function vk_api_format_url(base_url, params, params_noencode)
    local url=nil
    if string.sub(base_url, -1) == '?' then
        url=vk_api_server..base_url..'access_token='..cfg.vk_api_access_token
    else
        url=vk_api_server..base_url..'&access_token='..cfg.vk_api_access_token
    end
    if params then
        for key,value in pairs(params) do
            url=url..'&'..key..'='..util.urlencode(value)
        end    
    end
    if params_noencode then
        for key,value in pairs(params_noencode) do
            url=url..'&'..key..'='..value
        end    
    end

    local params_for_sig=nil
    if string.sub(base_url, -1) == '?' then
        params_for_sig=base_url..'access_token='..cfg.vk_api_access_token
    else
        params_for_sig=base_url..'&access_token='..cfg.vk_api_access_token
    end    
    if params then
        for key,value in pairs(params) do
            params_for_sig=params_for_sig..'&'..key..'='..value
        end    
    end
    if params_noencode then
        for key,value in pairs(params_noencode) do
            params_for_sig=params_for_sig..'&'..key..'='..value
        end    
    end
    url=url..'&sig='..vk_api_request_signature(params_for_sig, cfg.vk_api_secret)
    if cfg.debug>0 then print('VK API request URL '..url) end
    return url
end

function vk_api_request_signature(request, secret)
    return(util.md5_string_hash(request..secret))
end

function unescape_html_string(str)
    if str then
        result = string.gsub(str, "&quot;", '"')
        result = string.gsub(result, "&apos;", "'")
        result = string.gsub(result, "&amp;", '&')
        result = string.gsub(result, "&lt;", '<')
        result = string.gsub(result, "&gt;", '>')
        return result
    else
        return nil
    end
end

function vk_is_signed_in()
    if cfg.vk_api_access_token and cfg.vk_api_secret and cfg.vk_api_user_id then
        return true
    else
        return false
    end
end

function vk_check_response_for_errors(response)
    if response and type(response) == "table" and response.error and type(response.error) == "table" then
        if cfg.debug>0 then 
            print('VK API response error code '..response.error.error_code..': '..response.error.error_msg)
        end
        return response.error.error_code
    elseif not response then
        return "NO RESPONSE"
    else
        return nil
    end
end

function vk_get_name()
    if vk_is_signed_in() then
        local url=vk_api_format_url(vk_api_request_get_user_info, 
            {['uids']=cfg.vk_api_user_id, ['fields']='first_name,last_name'})
        local data=http.download(url)
        if data then
            local response=json.decode(data)
            data=nil
            if vk_check_response_for_errors(response) then
                return nil
            else
                return response.response[1].first_name.." "..response.response[1].last_name
            end
        else
            return nil
        end
    else
        return nil
    end
end

function vk_get_groups()
    if vk_is_signed_in() then
        local url=vk_api_format_url(vk_api_request_get_user_groups, {['extended']='1'})
        local data=http.download(url)
        if data then
            local response=json.decode(data)
            data=nil
            if vk_check_response_for_errors(response) then
                return nil
            elseif response and response.response then
                local groups={}
                for i,value in pairs(response.response) do
                    if type(value) == 'table' then
                        groups[value.gid]=value.name
                    end
                end
                return groups
            else
                return nil
            end
        else
            return nil
        end
    else
        return nil
    end
end

function vk_get_friends()
    if vk_is_signed_in() then
        local url=vk_api_format_url(vk_api_request_get_user_friends, {['fields']='uid,first_name,last_name'})
        local data=http.download(url)
        if data then
            local response=json.decode(data)
            data=nil
            if vk_check_response_for_errors(response) then
                return nil
            elseif response and response.response then
                local friends={}
                for i,value in pairs(response.response) do
                    if type(value) == 'table' then
                        friends[value.uid]=value.first_name..'  '..value.last_name
                    end
                end
                return friends
            else
                return nil
            end
        else
            return nil
        end
    else
        return nil
    end
end

function vk_video_add(vid, oid)
    if vk_is_signed_in() then
        local url=vk_api_format_url(vk_api_request_video_add, {['vid']=vid,['oid']=oid})
        local data=http.download(url)
        if data then
            local response=json.decode(data)
            data=nil
            if vk_check_response_for_errors(response) then
                return nil
            elseif response and response.response then
                local added_vid = response.response
                if cfg.debug>0 then 
                	print('VK API video.add: succesfully added video (vid='..vid..', oid='..oid..') to my page with new vid='..vid)
                end
                return added_vid
            else
                return nil
            end
        else
            return nil
        end
    else
        return nil
    end
end

function vk_video_delete(vid, oid)
    if vk_is_signed_in() then
        local url=vk_api_format_url(vk_api_request_video_delete, {['vid']=vid,['oid']=oid})
        local data=http.download(url)
        if data then
            local response=json.decode(data)
            data=nil
            if vk_check_response_for_errors(response) then
                return nil
            elseif response and response.response then
                if cfg.debug>0 then 
                	print('VK API video.delete: succesfully deleted video (vid='..vid..', oid='..oid..') from my page')
                end
                return response.response
            else
                return nil
            end
        else
            return nil
        end
    else
        return nil
    end
end

function vk_video_get_external_player_page(vid, oid)
    if vk_is_signed_in() then
        local url=vk_api_format_url(vk_api_request_video_get, {['videos']=oid..'_'..vid})
        local data=http.download(url)
        if data then
            local response=json.decode(data)
            data=nil
            if vk_check_response_for_errors(response) then
                return nil
            elseif response and response.response then                
                local player_url = string.gsub(response.response[2].player,"vkontakte%.ru/","vk.com/")
                if cfg.debug>0 then
                	print('VK API video.get: external player page for video (vid='..vid..', oid='..oid..') is '..player_url)
                end
                return player_url
            else
                return nil
            end
        else
            return nil
        end
    else
        return nil
    end
end

function vk_video_get_direct_download_link(host, uid, vtag, no_flv, max_hd)
    if (host and not string.match(host, '/$')) then
        host=host..'/'
    end
    if (host and uid and vtag and max_hd) then
        if tonumber(max_hd)>=3 then
            return host..'u'..uid..'/videos/'..vtag..'.'..'720.mp4'
        end
        if tonumber(max_hd)>=2 then
            return host..'u'..uid..'/videos/'..vtag..'.'..'480.mp4'
        end
        if tonumber(max_hd)>=1 then
            return host..'u'..uid..'/videos/'..vtag..'.'..'360.mp4'
        end
        if (tonumber(max_hd)>=0 and tonumber(no_flv)==1) then
            return host..'u'..uid..'/videos/'..vtag..'.'..'240.mp4'
        end
        return error_file_video_flash_format_only
    end
    return error_file_video_flash_format_only
end

function get_sort_order(label)
    if label=="length" then
        return 1
    elseif label=="rel" then
        return 2
    else
        return 0
    end
end

function vk_api_request_auth(redirect_url)
    return 'http://oauth.vk.com/authorize?client_id='..vk_app_id..'&scope='..vk_app_scope..'&response_type=token&redirect_uri='..util.urlencode(redirect_url..'/ui/vk_landing')
end

function ui_vk_landing()
    http.send("<script>location.href = document.URL.replace('vk_landing', 'vk_update').replace('#', '?');</script>")
end

function ui_vk_update()
    local f=io.open(cfg.config_path..'vkontakte.lua','w')
    if f then
        if ui_args.access_token and ui_args.secret and ui_args.user_id then
            f:write('cfg.vk_api_access_token="',ui_args.access_token,'"\ncfg.vk_api_secret="',ui_args.secret,
                '"\ncfg.vk_api_user_id="',ui_args.user_id,'"\n')
            http.send('<h3>VKontakte sign-in OK</h3>')
        else
            f:write('cfg.vk_api_access_token=""\ncfg.vk_api_secret=""\ncfg.vk_api_user_id=""\n')
            http.send('<h3>VKontakte sign-in FAILED</h3>')
            http.send('Error: '..util.urldecode(ui_args.error)..'<br />Error description: '..util.urldecode(ui_args.error_description)..'<br />')
        end
        f:close()
        core.sendevent('config')
    else
        http.send('<h3>Error opening config file</h3>')
    end
    http.send('<br/><a class="btn btn-info" href="/ui/config">Back</a>')
end

function ui_vk_status()
    http.send('<h3>VKontakte Status</h3>')
    local vk_name=plugins.vkontakte.vk_get_name()
    if vk_name then
        http.send('You are signed as <b>'..vk_name..'</b> - <a href="'..plugins.vkontakte.vk_api_request_auth(www_location)..'">sign-in as another user</a>.')
        http.send('<h4>Groups</h4>')
        http.send('<table class="table"><tr><th>group name</th><th>feed data</th></tr>')
        local vk_groups=plugins.vkontakte.vk_get_groups()
        if vk_groups then
            for group_id,group_name in pairs(vk_groups) do
                http.send(string.format('<tr><td><a href="http://vk.com/club%s">%s</a></td><td>group&#47;%s</td></tr>',group_id,group_name,group_id))
            end
        end
        http.send('</table>')
        http.send('<h4>Firiends</h4>')
        http.send('<table class="table"><tr><th>friend name</th><th>feed data</th></tr>')
        local vk_friends=plugins.vkontakte.vk_get_friends()
        if vk_friends then
            for user_id,user_name in pairs(vk_friends) do
                http.send(string.format('<tr><td><a href="http://vk.com/id%s">%s</a></td><td>user&#47;%s</td></tr>',user_id,user_name,user_id))
            end
        end
        http.send('</table>')
    else
        http.send('You are not signed in - ')
        http.send('<a href="'..plugins.vkontakte.vk_api_request_auth(www_location)..'">sign-in</a>.')
    end

    http.send('<br/><br/><a class="btn btn-primary" href="/ui/vk_status">Refresh</a>')
end


plugins['vkontakte']={}
plugins.vkontakte.name="VKontakte"
plugins.vkontakte.desc="my, group/<i>group_id</i>, group/<i>group_id</i>/<i>album_id</i>, user/<i>user_id</i>, user/<i>user_id</i>/<i>album_id</i>, search/<i>search_order</i>/<i>search_string</i>, search_hd/<i>search_order</i>/<i>search_string</i>"..
"<br/>(<a onclick='window.open(this.href,\'newwin\',\'width=450,scrollbars=yes,toolbar=no,menubar=no\'); return false;' href='/ui/vk_status'>view groups, friends and plugin help</a>)"

plugins.vkontakte.sendurl=vk_sendurl
plugins.vkontakte.updatefeed=vk_updatefeed
plugins.vkontakte.vk_api_request_auth=vk_api_request_auth
plugins.vkontakte.vk_get_name=vk_get_name
plugins.vkontakte.vk_get_groups=vk_get_groups
plugins.vkontakte.vk_get_friends=vk_get_friends

plugins.vkontakte.ui_actons=
{
    ['vk_landing']      = { 'xupnpd - vkontakte sign-in redirect', ui_vk_landing },
    ['vk_update']       = { 'xupnpd - vkontakte sign-in result', ui_vk_update },
    ['vk_status']       = { 'xupnpd - vkontakte status', ui_vk_status }
}

plugins.vkontakte.ui_vars=
{
    { "vk_auth_link",
        function()
            local s

            local vk_name=plugins.vkontakte.vk_get_name()
            if vk_name then
                s='You are signed as <b>'..vk_name..'</b><br/> <a href="'..plugins.vkontakte.vk_api_request_auth(www_location)..'">sign-in as another user</a><br/>'
            else
                s='You are not signed in - <a href="'..plugins.vkontakte.vk_api_request_auth(www_location)..'">sign-in</a><br/>'
            end

            return s..' <a onclick="window.open(this.href,\'newwin\',\'width=450,scrollbars=yes,toolbar=no,menubar=no\'); return false;" href="/ui/vk_status">status and help</a>'
        end
    }
}

plugins.vkontakte.ui_config_vars=
{
    { "select", "vk_private_workaround", "bool" },
    { "input", "vk_video_count", "int" }
}
