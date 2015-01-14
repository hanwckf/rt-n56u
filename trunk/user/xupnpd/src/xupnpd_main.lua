-- Copyright (C) 2011-2015 Anton Burdinuk
-- clark15b@gmail.com
-- https://tsdemuxer.googlecode.com/svn/trunk/xupnpd

http.sendurl_buffer_size(32768,1);

if cfg.daemon==true then core.detach() end

core.openlog(cfg.log_ident,cfg.log_facility)

if cfg.daemon==true then core.touchpid(cfg.pid_file) end

if cfg.embedded==true then cfg.debug=0 end

function clone_table(t)
    local tt={}
    for i,j in pairs(t) do
        tt[i]=j
    end
    return tt
end

function split_string(s,d)
    local t={}
    d='([^'..d..']+)'
    for i in string.gmatch(s,d) do
        table.insert(t,i)
    end
    return t
end

function load_plugins(path,what)
    local d=util.dir(path)

    if d then
        for i,j in ipairs(d) do
            if string.find(j,'^[%w_-]+%.lua$') then
                if cfg.debug>0 then print(what..' \''..j..'\'') end
                dofile(path..j)
            end
        end
    end
end


-- options for profiles
cfg.dev_desc_xml='/dev.xml'             -- UPnP Device Description XML
cfg.upnp_container='object.container'   -- UPnP class for containers
cfg.upnp_artist=false                   -- send <upnp:artist> / <upnp:actor> in SOAP response
cfg.upnp_feature_list=''                -- X_GetFeatureList response body
cfg.upnp_albumart=0                     -- 0: <upnp:albumArtURI>direct url</upnp:albumArtURI>, 1: <res>direct url<res>, 2: <upnp:albumArtURI>local url</upnp:albumArtURI>, 3: <res>local url<res>
cfg.dlna_headers=true                   -- send TransferMode.DLNA.ORG and ContentFeatures.DLNA.ORG in HTTP response
cfg.dlna_extras=true                    -- DLNA extras in headers and SOAP
cfg.content_disp=false                  -- send Content-Disposition when streaming
cfg.soap_length=true                    -- send Content-Length in SOAP response
cfg.wdtv=false                          -- WDTV Live compatible mode
cfg.sec_extras=false                    -- Samsung extras


update_id=1             -- system update_id

subscr={}               -- event sessions (for UPnP notify engine)
plugins={}              -- external plugins (YouTube, Vimeo ...)
profiles={}             -- device profiles
cache={}                -- real URL cache for plugins
cache_size=0

if not cfg.feeds_path then cfg.feeds_path=cfg.playlists_path end

-- create feeds directory
if cfg.feeds_path~=cfg.playlists_path then os.execute('mkdir -p '..cfg.feeds_path) end

-- load config, plugins and profiles
load_plugins(cfg.plugin_path,'plugin')
load_plugins(cfg.config_path,'config')

dofile('xupnpd_mime.lua')

if cfg.profiles then load_plugins(cfg.profiles,'profile') end

dofile('xupnpd_m3u.lua')
dofile('xupnpd_ssdp.lua')
dofile('xupnpd_http.lua')

-- download feeds from external sources (child process)
function update_feeds_async()
    local num=0
    for i,j in ipairs(feeds) do
        local plugin=plugins[ j[1] ]
        if plugin and plugin.disabled~=true and plugin.updatefeed then
            if plugin.updatefeed(j[2],j[3])==true then num=num+1 end
        end
    end

    if num>0 then core.sendevent('reload') end

end

-- spawn child process for feeds downloading
function update_feeds(what,sec)
    core.fspawn(update_feeds_async)
    core.timer(cfg.feeds_update_interval,what)
end


-- subscribe player for ContentDirectory events
function subscribe(event,sid,callback,ttl)
    local s=nil

    if subscr[sid] then
        s=subscr[sid]
        s.timestamp=os.time()
    else
        if callback=='' then return end
        s={}
        subscr[sid]=s
        s.event=event
        s.sid=sid
        s.callback=callback
        s.timestamp=os.time()
        s.ttl=tonumber(ttl)
        s.seq=0
    end

    if cfg.debug>0 then print('subscribe: '..s.sid..', '..s.event..', '..s.callback) end

end

-- unsubscribe player
function unsubscribe(sid)
    if subscr[sid] then
        subscr[sid]=nil

        if cfg.debug>0 then print('unsubscribe: '..sid) end
    end
end

--store to cache
function cache_store(k,v)
    local time=os.time()

    local cc=cache[k]

    if cc then cc.value=v cc.time=time return end

    if cache_size>=cfg.cache_size then
        local min_k=nil
        local min_time=nil
        for i,j in pairs(cache) do
            if not min_time or min_time>j.time then min_k=i min_time=j.time end
        end
        if min_k then
            if cfg.debug>0 then print('remove URL from cache (overflow): '..min_k) end
            cache[min_k]=nil
            cache_size=cache_size-1
        end
    end

    local t={}
    t.time=time
    t.value=v
    cache[k]=t
    cache_size=cache_size+1
end


-- garbage collection
function sys_gc(what,sec)

    local t=os.time()

    -- force unsubscribe
    local g={}

    for i,j in pairs(subscr) do
        if os.difftime(t,j.timestamp)>=j.ttl then
            table.insert(g,i)
        end
    end

    for i,j in ipairs(g) do
        subscr[j]=nil

        if cfg.debug>0 then print('force unsubscribe (timeout): '..j) end
    end

    -- cache clear
    g={}

    for i,j in pairs(cache) do
        if os.difftime(t,j.time)>=cfg.cache_ttl then
            table.insert(g,i)
        end
    end

    cache_size=cache_size-table.maxn(g)

    for i,j in ipairs(g) do
        cache[j]=nil

        if cfg.debug>0 then print('remove URL from cache (timeout): '..j) end
    end

    core.timer(sec,what)
end


-- ContentDirectory event deliver (child process)
function subscr_notify_iterate_tree(pls,tt)
    if pls.elements then
        table.insert(tt,pls.objid..','..update_id)

        for i,j in ipairs(pls.elements) do
            subscr_notify_iterate_tree(j,tt)
        end
    end    
end

function subscr_notify_async(t)

    local tt={}
    subscr_notify_iterate_tree(playlist_data,tt)

    local data=string.format(
        '<e:propertyset xmlns:e=\"urn:schemas-upnp-org:event-1-0\"><e:property><SystemUpdateID>%s</SystemUpdateID><ContainerUpdateIDs>%s</ContainerUpdateIDs></e:property></e:propertyset>',
        update_id,table.concat(tt,','))

    for i,j in ipairs(t) do
        if cfg.debug>0 then print('notify: '..j.callback..', sid='..j.sid..', seq='..j.seq) end
        http.notify(j.callback,j.sid,data,j.seq)
    end
end


-- reload all playlists
function reload_playlist()
    reload_playlists()
    update_id=update_id+1

    if update_id>100000 then update_id=1 end

    if cfg.debug>0 then print('reload playlist, update_id='..update_id) end

    if cfg.dlna_notify==true then
        local t={}

        for i,j in pairs(subscr) do
            if j.event=='cds' then
                table.insert(t, { ['callback']=j.callback, ['sid']=j.sid, ['seq']=j.seq } )
                j.seq=j.seq+1
                if j.seq>100000 then j.seq=0 end
            end
        end

        if table.maxn(t)>0 then
            core.fspawn(subscr_notify_async,t)
        end
    end
end

-- change child process status (for UI)
function set_child_status(pid,status)
    pid=tonumber(pid)
    if childs[pid] then
        childs[pid].status=status
        childs[pid].time=os.time()
    end
end

function get_drive_state(drive)
    local s

    local f=io.popen('/sbin/hdparm -C '..drive..' 2>/dev/null | grep -i state','r')

    if f then
        s=f:read('*a')
        f:close()
    end

    return string.match(s,'drive state is:%s+(.+)%s+')
end


function profile_change(user_agent,req)
    if not user_agent or user_agent=='' then return end

    for name,profile in pairs(profiles) do
        local match=profile.match

        if profile.disabled~=true and  match and match(user_agent,req) then

            local options=profile.options
            local mtypes=profile.mime_types

            if options then for i,j in pairs(options) do cfg[i]=j end end

            if mtypes then
                if profile.replace_mime_types==true then
                    mime=mtypes
                else
                    for i,j in pairs(mtypes) do mime[i]=j end
                end
            end

            return name
        end
    end
    return nil
end


-- event handlers
events['SIGUSR1']=reload_playlist
events['reload']=reload_playlist
events['store']=cache_store
events['sys_gc']=sys_gc
events['subscribe']=subscribe
events['unsubscribe']=unsubscribe
events['update_feeds']=update_feeds
events['status']=set_child_status
events['config']=function() load_plugins(cfg.config_path,'config') cache={} cache_size=0 end
events['remove_feed']=function(id) table.remove(feeds,tonumber(id)) end
events['add_feed']=function(plugin,feed,name) table.insert(feeds,{[1]=plugin,[2]=feed,[3]=name}) end
events['plugin']=function(name,status) if status=='on' then plugins[name].disabled=false else plugins[name].disabled=true end end
events['profile']=function(name,status) if status=='on' then profiles[name].disabled=false else profiles[name].disabled=true end end
events['bookmark']=function(objid,pos) local pls=find_playlist_object(objid) if pls then pls.bookmark=pos end end

events['update_playlists']=
function(what,sec)
    if cfg.drive and cfg.drive~='' then
        if get_drive_state(cfg.drive)=='active/idle' then
            reload_playlist()
        end
    else
        reload_playlist()
    end

    core.timer(cfg.playlists_update_interval,what)
end


if cfg.embedded==true then print=function () end end

-- start garbage collection system
core.timer(300,'sys_gc')

http.timeout(cfg.http_timeout)
http.user_agent(cfg.user_agent)

-- start feeds update system
if cfg.feeds_update_interval>0 then
    core.timer(3,'update_feeds')
end

if cfg.playlists_update_interval>0 then
    core.timer(cfg.playlists_update_interval,'update_playlists')
end

load_plugins(cfg.config_path..'postinit/','postinit')

print("start "..cfg.log_ident)

core.mainloop()

print("stop "..cfg.log_ident)

if cfg.daemon==true then os.execute('rm -f '..cfg.pid_file) end
