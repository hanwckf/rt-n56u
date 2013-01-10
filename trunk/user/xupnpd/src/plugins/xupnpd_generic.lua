-- Copyright (C) 2011-2013 Anton Burdinuk
-- clark15b@gmail.com
-- https://tsdemuxer.googlecode.com/svn/trunk/xupnpd

function generic_updatefeed(feed,friendly_name)

    local feed_name=string.match(feed,'.+/(.+).m3u$')
    local feed_path=cfg.feeds_path..feed_name..'.m3u'
    local tmp_path=cfg.tmp_path..feed_name..'.m3u'

    if http.download(feed,tmp_path)<=0 then
        return false
    end

    if util.md5(tmp_path)~=util.md5(feed_path) then
        if os.execute(string.format('mv %s %s',tmp_path,feed_path))==0 then
            if cfg.debug>0 then print('feed \''..feed_name..'\' updated') end
            return true
        end
    else
        util.unlink(tmp_path)
    end

    return false
end

function generic_sendurl(generic_url,range)
    local rc,location

    location=generic_url

    for i=1,5,1 do
        rc,location=http.sendurl(location,1,range)

        if not location then
            break
        else
            if cfg.debug>0 then print('Redirect #'..i..' to: '..location) end
        end
    end
end

plugins['generic']={}
plugins.generic.name="Generic"
plugins.generic.desc="<i>m3u_url</i>"
plugins.generic.sendurl=generic_sendurl
plugins.generic.updatefeed=generic_updatefeed
