-- Copyright (C) 2012 I. Sokolov, 2013 Anton Burdinuk
-- happy.neko@gmail.com
-- Licensed under GNU GPL version 2 
-- https://www.gnu.org/licenses/gpl-2.0.html

-- config example:
--feeds=
--{
--    { "giantbomb", "all", "Gaint Bomb - All Videos" },
--    { "giantbomb", "quicklook", "Gaint Bomb - QuickLooks" },
--    { "giantbomb", "review", "Gaint Bomb - Reviews" },
--    { "giantbomb", "feature", "Gaint Bomb - Features" },
--    { "giantbomb", "trailer", "Gaint Bomb - Trailers" },
--    { "giantbomb", "event", "Gaint Bomb - Events" },
--    { "giantbomb", "endurance", "Gaint Bomb - Endurance Run" },
--    { "giantbomb", "tang", "Gaint Bomb - TANG" }
--}

gb_videos_url='http://www.giantbomb.com/videos'
gb_domain_url='http://www.giantbomb.com'
gb_video_types={ ['all']='popular/', ['quicklook']='quick-looks/', ['review']='reviews/', ['feature']='features/',
                 ['trailer']='trailers/', ['event']='events/', ['endurance']='endurance-run/', ['tang']='tang/' }

function gb_updatefeed(feed,friendly_name)
    local rc=false

    local feed_name='giantbomb_'..string.gsub(feed,'/','_')
    local feed_m3u_path=cfg.feeds_path..feed_name..'.m3u'
    local tmp_m3u_path=cfg.tmp_path..feed_name..'.m3u'

    local feed_url=nil
    local tfeed=split_string(feed,'/')
    if gb_video_types[tfeed[1]] then
        feed_url=gb_videos_url..'/'..gb_video_types[tfeed[1]]
    else
        feed_url=gb_videos_url..'/'
    end

    local feed_data=http.download(feed_url)
    if feed_data then
        local dfd=io.open(tmp_m3u_path,'w+')
        if dfd then
            dfd:write('#EXTM3U name=\"',friendly_name or feed_name,'\" type=mp4 plugin=giantbomb\n')
            for link,logo,title in string.gmatch(feed_data, '<li>%s*<a href="(/videos/[%w%-/]+)">.-<img src="(http://static%.giantbomb%.com/uploads/screen_medium/[%w/%.%-_]+)"%s*/>.- class="title">(.-)</h3>' ) do
                dfd:write('#EXTINF:0 logo=',logo,',',title,'\n',gb_domain_url..link,'\n')
            end
            dfd:close()
            feed_data=nil

            if util.md5(tmp_m3u_path)~=util.md5(feed_m3u_path) then
                if os.execute(string.format('mv %s %s',tmp_m3u_path,feed_m3u_path))==0 then
                    if cfg.debug>0 then print('Giant Bomb feed \''..feed_name..'\' updated') end
                    rc=true
                end
            else
                util.unlink(tmp_m3u_path)
            end
        end
    end

    return rc
end

function gb_sendurl(gb_page_url,range)
    local url=nil
    if plugin_sendurl_from_cache(gb_page_url,range) then return end

    local clip_page=http.download(gb_page_url)
    if clip_page then
        local youtube_id=string.match(clip_page,'youtube%.com/embed/([%w%-_]+)"')
        clip_page=nil
        if youtube_id then
            url=plugins.youtube.getvideourl('http://www.youtube.com/watch?v='..youtube_id..'&feature=youtube_gdata')
            if url then
                if cfg.debug>0 then print('Giant Bomb Real URL: '..url) end
                plugin_sendurl(gb_page_url,url,range)
            end
        end
    end

    if not url then
        if cfg.debug>0 then print('Giant Bomb clip is not found') end

        plugin_sendfile('www/corrupted.mp4')
    end

end

plugins['giantbomb']={}
plugins.giantbomb.name="Giant Bomb"
plugins.giantbomb.desc="<i>channel</i>"..
"<br/><b>Giant Bomb channels</b>: all, quicklook, review, feature, trailer, event, endurance, tang"
plugins.giantbomb.sendurl=gb_sendurl
plugins.giantbomb.updatefeed=gb_updatefeed
