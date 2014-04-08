-- Copyright (C) 2011-2013 Anton Burdinuk
-- clark15b@gmail.com
-- https://tsdemuxer.googlecode.com/svn/trunk/xupnpd

-- MP4-lo
-- MP4-hi
cfg.ivi_fmt='MP4-hi'
cfg.ivi_video_count=100

-- genre:
-- arthouse, boeviki, voennye, detective, detskiy, documentary, drama, comedy, korotkometrazhki, melodramy, zolotaya_klassika, adventures, sovetskoe_kino, thriller, horror, fantastika, erotika
function ivi_updatefeed(feed,friendly_name)
    local rc=false

    local feed_url=nil
    local scroll=false
    local feed_name='ivi_'..string.gsub(feed,'/','_')
    local feed_m3u_path=cfg.feeds_path..feed_name..'.m3u'
    local tmp_m3u_path=cfg.tmp_path..feed_name..'.m3u'

    local tfeed=split_string(feed,'/')

    if table.maxn(tfeed)>1 then
        if tfeed[1]=='genre' then
            feed_url='http://www.ivi.ru/videos/all/movies/'..tfeed[2]..'/by_year/?'
            scroll=true
        else
            feed_url=string.format('http://www.ivi.ru/watch/%s/%s',tfeed[1],tfeed[2])
        end
    else
        if tfeed[1]=='new' then
            local year=tonumber(os.date('%Y'))
            feed_url='http://www.ivi.ru/videos/all/all/all/by_year/?year_from='..(year-1)..'&year_to='..year
            scroll=true
        else
            feed_url='http://www.ivi.ru/watch/'..tfeed[1]
        end
    end

    if feed_url then
        local dfd=io.open(tmp_m3u_path,'w+')

        if dfd then
            dfd:write('#EXTM3U name=\"',friendly_name or feed_name,'\" type=mp4 plugin=ivi\n')

            local count=0

            while(count<cfg.ivi_video_count) do
                local url=nil

                if scroll and count~=0 then url=feed_url..'&offset='..count else url=feed_url end

                if cfg.debug>0 then print('IVI try url '..url) end

                local feed_data=http.download(url)

                if feed_data then
                    if not scroll then
                        for urn,logo,name in string.gmatch(feed_data,'<li>%s*<span class="image">%s*<a href="/watch/([%w_%-/]+)">%s*<img src="(.-)">%s*</a>%s*</span>%s*<strong>%s*<a href=".-">(.-)</a>%s*</strong>%s*</li>') do
                            dfd:write('#EXTINF:0 logo=',logo,' ,',name,'\n','http://www.ivi.ru/watch/',urn,'\n')
                        end
                    else
                        local n=0

                        for id,logo,name in string.gmatch(feed_data,'<a href="/watch/([%w_%-/]+)"%s*>%s*<img src="(.-)" alt="(.-)"%s*>') do
                            dfd:write('#EXTINF:0 logo=',logo,' ,',name,'\n','http://www.ivi.ru/watch/',id,'\n')
                            n=n+1
                        end

                        if n<1 then scroll=false else count=count+n end
                    end

                    feed_data=nil
                else
                    scroll=false
                end

                if not scroll then break end
            end

            dfd:close()

            if util.md5(tmp_m3u_path)~=util.md5(feed_m3u_path) then
                if os.execute(string.format('mv %s %s',tmp_m3u_path,feed_m3u_path))==0 then
                    if cfg.debug>0 then print('IVI feed \''..feed_name..'\' updated') end
                    rc=true
                end
            else
                util.unlink(tmp_m3u_path)
            end

        end
    end

    return rc
end

function ivi_sendurl(ivi_url,range)
    local url=nil

    if plugin_sendurl_from_cache(ivi_url,range) then return end

    local ivi_id=string.match(ivi_url,'/?id=(%d+)$')

    if not ivi_id then
        ivi_id=string.match(ivi_url,'/(%d+)$')
    end

    if not ivi_id then
        if cfg.debug>0 then print('Invalid IVI URL format, example - http://www.ivi.ru/video/view/?id=12345 or http://www.ivi.ru/watch/12345') end
        return
    end

    local clip_page=http.download('http://api.digitalaccess.ru/api/json',nil,
        string.format('{"method":"da.content.get","params":["%s",{"site":"s183","contentid":"%s","referrer":"%s"}]}',ivi_id or '',ivi_id or '', ivi_url))

    if clip_page then
        local x=json.decode(clip_page)

        clip_page=nil

        if x.result and x.result.files then
            local url_lo=nil
            for i,j in ipairs(x.result.files) do
                if j.content_format==cfg.ivi_fmt then
                    url=j.url
                    break
                elseif j.content_format=='MP4-lo' then
                    url_lo=j.url
                end
            end
            if not url then url=url_lo end
        end
    else
        if cfg.debug>0 then print('IVI clip '..ivi_id..' is not found') end
    end

    if url then
        if cfg.debug>0 then print('IVI Real URL: '..url) end

        plugin_sendurl(ivi_url,url,range)
    else
        if cfg.debug>0 then print('IVI Real URL is not found') end

        plugin_sendfile('www/corrupted.mp4')
    end
end

plugins['ivi']={}
plugins.ivi.name="ivi.ru"
plugins.ivi.desc="new, <i>serial_name</i> or <i>serial_name</i>/<i>season</i> or genre/<i>genre</i>"..
"<br/><b>genres</b>: arthouse, boeviki, voennye, detective, detskiy, documentary, drama, comedy, korotkometrazhki, melodramy, zolotaya_klassika, adventures, sovetskoe_kino, thriller, horror, fantastika, performance, erotika"..
"<br/><b>example</b>: new, luntik, luntik/season2, genre/horror"
plugins.ivi.sendurl=ivi_sendurl
plugins.ivi.updatefeed=ivi_updatefeed

plugins.ivi.ui_config_vars=
{
    { "select", "ivi_fmt" },
    { "input", "ivi_video_count", "int" }
}

--ivi_updatefeed('luntik','Лунтик')
--ivi_updatefeed('luntik/season2','Лунтик')
--ivi_updatefeed('genre/horror','Horrors')
--ivi_updatefeed('new','new')
