-- Copyright (C) 2011-2013 Anton Burdinuk
-- clark15b@gmail.com
-- https://tsdemuxer.googlecode.com/svn/trunk/xupnpd

-- 720pd        - 720p 60fps
-- 720p         - 720p
-- 400pd        - 400p 60fps
-- 400p         - 400p
-- 200p         - 200p
cfg.ag_fmt='400p'

-- videos, videos/top100pop, videos/top100best, videos/selected_by_ag
function ag_updatefeed(feed,friendly_name)
    local rc=false

    local feed_url='http://www.ag.ru/files/'..feed
    local feed_name='ag_'..string.gsub(feed,'/','_')
    local feed_m3u_path=cfg.feeds_path..feed_name..'.m3u'
    local tmp_m3u_path=cfg.tmp_path..feed_name..'.m3u'

    local feed_data=http.download(feed_url)

    if feed_data then
        local dfd=io.open(tmp_m3u_path,'w+')
        if dfd then
            dfd:write('#EXTM3U name=\"',friendly_name or feed_name,'\" type=mp4 plugin=ag\n')

            for game,id,name in string.gmatch(feed_data,'href=/games/([%w_%-]+)/videos#([%w_%-]+)>(.-)</a>') do
                local url=string.format('http://www.ag.ru/games/%s/videos/%s/',game,id)
                dfd:write('#EXTINF:0,',util.win1251toUTF8(name),'\n',url,'\n')
            end

            dfd:close()

            if util.md5(tmp_m3u_path)~=util.md5(feed_m3u_path) then
                if os.execute(string.format('mv %s %s',tmp_m3u_path,feed_m3u_path))==0 then
                    if cfg.debug>0 then print('AG feed \''..feed_name..'\' updated') end
                    rc=true
                end
            else
                util.unlink(tmp_m3u_path)
            end
        end

        feed_data=nil
    end

    return rc
end

function ag_sendurl(ag_url,range)

    local url=nil

    if plugin_sendurl_from_cache(ag_url,range) then return end

    local clip_page=plugin_download(ag_url)

    if clip_page then
        local playlist_id=string.match(clip_page,'/playlist/(%w+)%.json')
        clip_page=nil

        if playlist_id then

            clip_page=plugin_download(string.format('http://www.ag.ru/playlist/%s.json',playlist_id))

            if clip_page then
                local o=json.decode(clip_page)
                clip_page=nil

                local url_200p=nil

                if o and o['resolutionswitcherplugin.streams'] then
                    for i,j in ipairs(o['resolutionswitcherplugin.streams']) do
                        if j.id==cfg.ag_fmt then url=j.file break elseif j.id=='200p' then url_200p=j.file end
                    end
                end

                if not url then url=url_200p end

            end
        end
    else
        if cfg.debug>0 then print('AG clip is not found') end
    end

    if url then
        if cfg.debug>0 then print('AG Real URL: '..url) end

        plugin_sendurl(ag_url,url,range)
    else
        if cfg.debug>0 then print('AG Real URL is not found') end

        plugin_sendfile('www/corrupted.mp4')
    end
end

plugins['ag']={}
plugins.ag.name="AG.ru"
plugins.ag.desc="<i>channel</i>"..
"<br/><b>AG.ru channels</b>: videos, videos/top100pop, videos/top100best, videos/selected_by_ag"
plugins.ag.sendurl=ag_sendurl
plugins.ag.updatefeed=ag_updatefeed

plugins.ag.ui_config_vars=
{
    { "select", "ag_fmt" },
}

--ag_updatefeed('videos')
