-- Copyright (C) 2011-2013 Anton Burdinuk
-- clark15b@gmail.com
-- https://tsdemuxer.googlecode.com/svn/trunk/xupnpd

-- 18 - 360p  (MP4,h.264/AVC)
-- 22 - 720p  (MP4,h.264/AVC) hd
-- 37 - 1080p (MP4,h.264/AVC) hd
-- 82 - 360p  (MP4,h.264/AVC)    stereo3d
-- 83 - 480p  (MP4,h.264/AVC) hq stereo3d
-- 84 - 720p  (MP4,h.264/AVC) hd stereo3d
-- 85 - 1080p (MP4,h.264/AVC) hd stereo3d

cfg.youtube_fmt=22
cfg.youtube_region='*'
cfg.youtube_video_count=100

youtube_api_url='http://gdata.youtube.com/feeds/mobile/'

function youtube_find_playlist(user,playlist)
    if string.sub(playlist,1,3)=='id:' then
        return string.sub(playlist,4)
    end

    local start_index=1
    local max_results=50

    while(true) do
        local feed_data=http.download(youtube_api_url..'users/'..user..'/playlists?alt=json&start-index='..start_index..'&max-results='..max_results)

        if not feed_data then break end

        local x=json.decode(feed_data)
        feed_data=nil

        if not x or not x.feed or not x.feed.entry then break end

        for i,j in ipairs(x.feed.entry) do
            if j.title['$t']==playlist then
                return string.match(j.id['$t'],'.+/([^/]+)$')
            end
        end
        start_index=start_index+max_results
    end

    return nil
end

-- username, favorites/username, playlist/username/playlistname, playlist/username/id:playlistid, channel/channelname, search/searchstring
-- channels: top_rated, top_favorites, most_viewed, most_recent, recently_featured
function youtube_updatefeed(feed,friendly_name)
    local rc=false

    local feed_url=nil
    local feed_urn=nil

    local tfeed=split_string(feed,'/')

    local feed_name='youtube_'..string.lower(string.gsub(feed,"[/ :\'\"]",'_'))

    if tfeed[1]=='channel' then
        local region=''
        if cfg.youtube_region and cfg.youtube_region~='*' then region=cfg.youtube_region..'/' end
        feed_urn='standardfeeds/'..region..tfeed[2]..'?alt=json'
    elseif tfeed[1]=='favorites' then
        feed_urn='users/'..tfeed[2]..'/favorites'..'?alt=json'
    elseif tfeed[1]=='playlist' then
        local playlist_id=youtube_find_playlist(tfeed[2],tfeed[3])

        if not playlist_id then return false end

        feed_urn='playlists/'..playlist_id..'?alt=json'

    elseif tfeed[1]=='search' then
        feed_urn='videos?vq='..util.urlencode(tfeed[2])..'&alt=json'
    else
        feed_urn='users/'..tfeed[1]..'/uploads?orderby=published&alt=json'
    end

    local feed_m3u_path=cfg.feeds_path..feed_name..'.m3u'
    local tmp_m3u_path=cfg.tmp_path..feed_name..'.m3u'

    local dfd=io.open(tmp_m3u_path,'w+')

    if dfd then
        dfd:write('#EXTM3U name=\"',friendly_name or feed_name,'\" type=mp4 plugin=youtube\n')

        local start_index=1
        local max_results=50
        local count=0

        while(count<cfg.youtube_video_count) do

            local url=youtube_api_url..feed_urn..'&start-index='..start_index..'&max-results='..max_results

            if cfg.debug>0 then print('YouTube try url '..url) end

            local feed_data=http.download(url)

            if not feed_data then break end

            local x=json.decode(feed_data)

            feed_data=nil

            if not x or not x.feed or not x.feed.entry then break end

            local n=0

            for i,j in ipairs(x.feed.entry) do
                local title=j.title['$t']

                local url=nil
                for ii,jj in ipairs(j.link) do
                    if jj['type']=='text/html' then url=jj.href break end
                end

                local logo=nil

                local thumb=j['media$group']['media$thumbnail']

                if thumb and thumb[1] then
                    logo=thumb[1].url
                end

                if logo and title and url then
                    dfd:write('#EXTINF:0 logo=',logo,' ,',title,'\n',url,'\n')

                    n=n+1
                end

            end

            if n<1 then break else count=count+n end

            start_index=start_index+max_results
        end

        dfd:close()

        if util.md5(tmp_m3u_path)~=util.md5(feed_m3u_path) then
            if os.execute(string.format('mv %s %s',tmp_m3u_path,feed_m3u_path))==0 then
                if cfg.debug>0 then print('YouTube feed \''..feed_name..'\' updated') end
                rc=true
            end
        else
            util.unlink(tmp_m3u_path)
        end
    end

    return rc
end

function youtube_sendurl(youtube_url,range)
    local url=nil
    if plugin_sendurl_from_cache(youtube_url,range) then return end

    url=youtube_get_video_url(youtube_url)
    if url then
        if cfg.debug>0 then print('YouTube Real URL: '..url) end

        plugin_sendurl(youtube_url,url,range)
    else
        if cfg.debug>0 then print('YouTube clip is not found') end

        plugin_sendfile('www/corrupted.mp4')
    end
end

function youtube_get_best_fmt(urls,fmt)
    if fmt>81 and fmt<86 then -- 3d
        local i=fmt while(i>81) do
            if urls[i] then return urls[i] end
            i=i-1
        end

        local t={ [82]=18, [83]=18, [84]=22, [85]=37 }

        fmt=t[fmt]
    end

    local t={ 37,22,18 }
    local t2={ [18]=true, [22]=true, [37]=true }

    for i=1,3,1 do
        local u=urls[ t[i] ]

        if u and t2[fmt] and t[i]<=fmt then return u end
    end

    return urls[18]
end

function youtube_get_video_url(youtube_url)
    local url=nil

    local clip_page=plugin_download(youtube_url)
    if clip_page then
        local s=json.decode(string.match(clip_page,'ytplayer.config%s*=%s*({.-});'))

        clip_page=nil

        local stream_map=nil

-- s.args.adaptive_fmts
-- itag 137: 1080p
-- itag 136: 720p
-- itag 135: 480p
-- itag 134: 360p
-- itag 133: 240p
-- itag 160: 144

--        local player_url=nil if s.assets then player_url=s.assets.js end if player_url and string.sub(player_url,1,2)=='//' then player_url='http:'..player_url end

        if s.args then stream_map=s.args.url_encoded_fmt_stream_map end

        local fmt=string.match(youtube_url,'&fmt=(%w+)$')

        if not fmt then fmt=cfg.youtube_fmt end

        if stream_map then
            local urls={}

            for i in string.gmatch(stream_map,'([^,]+)') do
                local item={}
                for j in string.gmatch(i,'([^&]+)') do
                    local name,value=string.match(j,'(%w+)=(.+)')
                    if name then
                        --print(name,util.urldecode(value))
                        item[name]=util.urldecode(value)
                    end
                end

                local sig=item['sig'] or item['s']
                local u=item['url']
                if sig then u=u..'&signature='..sig end
                --print(item['itag'],u)
                urls[tonumber(item['itag'])]=u

                --print('\n')
            end

            url=youtube_get_best_fmt(urls,tonumber(fmt))
        end

        return url
    else
        if cfg.debug>0 then print('YouTube clip is not found') end
        return nil
    end
end

plugins['youtube']={}
plugins.youtube.name="YouTube"
plugins.youtube.desc="<i>username</i>, favorites/<i>username</i>, playlist/<i>username</i>/<i>playlistname</i>, playlist/<i>username</i>/id:<i>playlistid</i>, channel/<i>channelname</i>, search/<i>search_string</i>"..
"<br/><b>YouTube channels</b>: top_rated, top_favorites, most_viewed, most_recent, recently_featured"
plugins.youtube.sendurl=youtube_sendurl
plugins.youtube.updatefeed=youtube_updatefeed
plugins.youtube.getvideourl=youtube_get_video_url

plugins.youtube.ui_config_vars=
{
    { "select", "youtube_fmt", "int" },
    { "select", "youtube_region" },
    { "input",  "youtube_video_count", "int" }
}

--youtube_updatefeed('channel/top_rated','')
