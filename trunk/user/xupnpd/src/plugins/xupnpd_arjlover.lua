-- Copyright (C) 2011-2013 Anton Burdinuk
-- clark15b@gmail.com
-- https://tsdemuxer.googlecode.com/svn/trunk/xupnpd

-- multiki, film, filmiki
function arjlover_updatefeed(feed,friendly_name)
    local rc=false

    local feed_url=string.format('http://%s.arjlover.net/%s/',feed,feed)
    local feed_name='arjlover_'..string.gsub(feed,'/','_')
    local feed_m3u_path=cfg.feeds_path..feed_name..'.m3u'
    local tmp_m3u_path=cfg.tmp_path..feed_name..'.m3u'

    local feed_data=http.download(feed_url)

    if feed_data then
        local dfd=io.open(tmp_m3u_path,'w+')
        if dfd then
            dfd:write('#EXTM3U name=\"',friendly_name or feed_name,'\" type=avi plugin=arjlover\n')

            local base=string.format('http://%s.arjlover.net',feed)
            local pattern=string.format('<td.-href="/info/.-.html" >(.-)</a></td>.-<td><a href="(/%s/.-)"',feed)

            for name,url in string.gmatch(feed_data,pattern) do
                local t='' if string.find(url,'%.mpg$') then t=' type=mpeg' end
                dfd:write('#EXTINF:0',t,',',util.win1251toUTF8(name),'\n',base,url,'\n')
            end

            dfd:close()

            if util.md5(tmp_m3u_path)~=util.md5(feed_m3u_path) then
                if os.execute(string.format('mv %s %s',tmp_m3u_path,feed_m3u_path))==0 then
                    if cfg.debug>0 then print('arjlover feed \''..feed_name..'\' updated') end
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

function arjlover_strip_url(url)
    local n=string.find(url,'#')
    if not n then return url end
    return string.sub(url,0,n-1)
end

function arjlover_sendurl(arjlover_url,range)
    if plugin_sendurl_from_cache(arjlover_url,range) then return end

    plugin_sendurl(arjlover_url,arjlover_url,range)
end

plugins['arjlover']={}
plugins.arjlover.name="arjlover.net"
plugins.arjlover.desc="multiki, film, filmiki"
plugins.arjlover.sendurl=arjlover_sendurl
plugins.arjlover.updatefeed=arjlover_updatefeed

--arjlover_updatefeed('multiki')
--arjlover_updatefeed('film')
--arjlover_updatefeed('filmiki')
