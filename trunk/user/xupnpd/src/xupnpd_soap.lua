-- Copyright (C) 2011-2015 Anton Burdinuk
-- clark15b@gmail.com
-- https://tsdemuxer.googlecode.com/svn/trunk/xupnpd

services={}
services.cds={}
services.cms={}
services.msr={}

function playlist_item_type(pls)
    local mtype=mime[pls.type]
    local extras=nil

    if cfg.dlna_extras==false then
        extras='*'
    else
        if pls.dlna_extras then extras=dlna_org_extras[pls.dlna_extras] end
        if not extras then extras=mtype[5] end
        if pls.path and extras~='*' then extras=string.gsub(extras,'DLNA.ORG_OP=%d%d','DLNA.ORG_OP=01') end
    end

    return mtype,extras
end

function playlist_get_url(pls)
    local url=pls.url
    if pls.path then
        url=string.format('%s/stream/%s.%s',www_location,pls.objid,pls.type)
    elseif pls.plugin then
        url=string.format('%s/proxy/%s.%s',www_location,pls.objid,pls.type)
    elseif cfg.proxy>0 then
        if cfg.proxy>1 or mtype[1]==2 then
            url=string.format('%s/proxy/%s.%s',www_location,pls.objid,pls.type)
        end
    end
    return url
end

function get_duration(n)
    local seconds=math.fmod(n,60) n=(n-seconds)/60
    local minutes=math.fmod(n,60) n=(n-minutes)/60

    return string.format(' duration="%.2d:%.2d:%.2d.000"',n,minutes,seconds)
end

function playlist_item_to_xml(id,parent_id,pls)
    local logo=''
    local sec_extras=''
    local objid=pls.objid

    if pls.logo then
        local l

        if cfg.upnp_albumart<2 then
            l=pls.logo
        else
            l=string.format('%s/logo/%s.jpeg',www_location,objid)
        end

        if cfg.upnp_albumart==0 or cfg.upnp_albumart==2 then
            logo=string.format('<upnp:albumArtURI dlna:profileID=\"JPEG_TN\">%s</upnp:albumArtURI>',l)
        else
            logo=string.format('<res protocolInfo="http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_TN">%s</res>',l)
        end
    end

    if cfg.sec_extras then
        if pls.path then
            for i, n in ipairs(util.dir(pls.path:sub(1, pls.path:len() - pls.url:len()))) do
                if n:sub(n:len() - 4+1) == ".srt" then
                    sec_extras=string.format('<sec:CaptionInfoEx sec:type="srt">%s/sub/%s%s</sec:CaptionInfoEx>',www_location,objid,util.urlencode(n:sub(pls.url:len() - 4+1)))
                end
            end
        end

        if pls.bookmark then
            sec_extras=sec_extras..string.format('<sec:dcmInfo>BM=%s</sec:dcmInfo>',pls.bookmark)
        end
    end

    if pls.elements then
        return string.format(
            '<container id=\"%s\" childCount=\"%i\" parentID=\"%s\" restricted=\"true\"><dc:title>%s</dc:title><upnp:class>%s</upnp:class></container>',
            id,pls.size or 0,parent_id,util.xmlencode(pls.name),cfg.upnp_container)
    else
        local mtype,extras=playlist_item_type(pls)

        local artist=''
        local url=pls.url or ''

        if cfg.upnp_artist==true and pls.parent then
            if mtype[1]==1 then
                artist=string.format('<upnp:actor>%s</upnp:actor>',util.xmlencode(pls.parent.name))
            elseif mtype[1]==2 then
                artist=string.format('<upnp:artist>%s</upnp:artist>',util.xmlencode(pls.parent.name))
            end
        end

        url=playlist_get_url(pls)

        local duration=''

        if(pls.duration) then duration=get_duration(tonumber(pls.duration)) end

        return string.format(
            '<item id=\"%s" parentID=\"%s\" restricted=\"true\"><dc:title>%s</dc:title><upnp:class>%s</upnp:class>%s%s<res size=\"%s\" protocolInfo=\"%s%s\"%s>%s</res>%s</item>',
            id,parent_id,util.xmlencode(pls.name),mtype[2],artist,logo,pls.length or 0,mtype[4],extras,duration,util.xmlencode(url),sec_extras)

    end
end

function get_playlist_item_parent(s)
    if s=='0' then return '-1' end

    local t={}

    for i in string.gmatch(s,'(%w+)_') do table.insert(t,i) end

    return table.concat(t,'_')
end


function xml_serialize(r)
    local t={}

    for i,j in pairs(r) do
        table.insert(t,'<') table.insert(t,j[1]) table.insert(t,'>')
        table.insert(t,j[2])
        table.insert(t,'</') table.insert(t,j[1]) table.insert(t,'>')
    end

    return table.concat(t)
end

function acl_validate(acl,ip)
    for i in string.gmatch(acl,'([^;]+)') do
        if ip==i then return true end
    end

    return false
end


services.cds.schema='urn:schemas-upnp-org:service:ContentDirectory:1'

function services.cds.GetSystemUpdateID()
    return {{'Id',update_id}}
end

function services.cds.GetSortCapabilities()
    return {{'SortCaps','dc:title'}}
end

function services.cds.GetSearchCapabilities()
    return {{'SearchCaps','upnp:class'}}
end

-- wget -O - "http://127.0.0.1:4044/soap/cds?action=X_GetFeatureList"
function services.cds.X_GetFeatureList()
    return {{'FeatureList',util.xmlencode(cfg.upnp_feature_list)}}
end

-- wget -O - "http://127.0.0.1:4044/soap/cds?action=X_SetBookmark&ObjectID=0_2_1&PosSecond=1234"
function services.cds.X_SetBookmark(args)
    core.sendevent('bookmark',args.ObjectID or '',args.PosSecond or '')
    return {}
end

-- wget -O - "http://127.0.0.1:4044/soap/cds?action=Browse&ObjectID=0&StartingIndex=0&RequestedCount=100"
function services.cds.Browse(args,ip)
    local items={}
    local count=0
    local total=0

    table.insert(items,'<DIDL-Lite xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns:dlna=\"urn:schemas-dlna-org:metadata-1-0\">')

    local objid=args.ObjectID or args.ContainerID

    local pls=find_playlist_object(objid)

    if pls then

        if args.BrowseFlag=='BrowseMetadata' then
            table.insert(items,playlist_item_to_xml(objid,get_playlist_item_parent(objid),pls))
            count=1
            total=1
        else
            local from=tonumber(args.StartingIndex)
            local to=from+tonumber(args.RequestedCount)

            if to==from then to=from+pls.size end

            if pls.elements then
                for i,j in ipairs(pls.elements) do
                    if i>from and i<=to then
                        if not j.acl or acl_validate(j.acl,ip) then
                            table.insert(items,playlist_item_to_xml(string.format('%s_%s',objid,i),objid,j))
                            count=count+1
                        end
                    end
                end
                total=pls.size
            end
        end

    end

    table.insert(items,'</DIDL-Lite>')

    return {{'Result',util.xmlencode(table.concat(items))}, {'NumberReturned',count}, {'TotalMatches',total}, {'UpdateID',update_id}}

end

-- wget -O - "http://127.0.0.1:4044/soap/cds?action=Search&ContainerID=0&StartingIndex=0&RequestedCount=100&SearchCriteria=*"
function services.cds.Search(args,ip)
    local items={}
    local count=0
    local total=0

    local from=tonumber(args.StartingIndex)
    local to=from+tonumber(args.RequestedCount)
    local what=util.upnp_search_object_type(args.SearchCriteria)

    if to==from then to=from+10000 end

    table.insert(items,'<DIDL-Lite xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns:dlna=\"urn:schemas-dlna-org:metadata-1-0\">')

    local pls=find_playlist_object(args.ContainerID)

    if pls then
        function __search(id,parent_id,p)
            if p.elements then
                if not p.virtual and (not p.acl or acl_validate(p.acl,ip)) then
                    for i,j in pairs(p.elements) do
                        __search(string.format('%s_%s',id,i),id,j)
                    end
                end
            else
                if what==0 or mime[p.type][1]==what then
                    total=total+1

                    if total>from and total<=to then
                        table.insert(items,playlist_item_to_xml(id,parent_id,p))
                        count=count+1
                    end
                end
            end
        end

        __search(args.ContainerID,get_playlist_item_parent(args.ContainerID),pls)
    end

    table.insert(items,'</DIDL-Lite>')

    return {{'Result',util.xmlencode(table.concat(items))}, {'NumberReturned',count}, {'TotalMatches',total}, {'UpdateID',update_id}}

end


services.cms.schema='urn:schemas-upnp-org:service:ConnectionManager:1'

function services.cms.GetCurrentConnectionInfo(args)
    return {{'ConnectionID',0}, {'RcsID',-1}, {'AVTransportID',-1}, {'ProtocolInfo',''},
        {'PeerConnectionManager',''}, {'PeerConnectionID',-1}, {'Direction','Output'}, {'Status','OK'}}
end

function services.cms.GetProtocolInfo()
    local protocols={}

    for i,j in pairs(upnp_proto) do table.insert(protocols,j..'*') end

    return {{'Sink',''}, {'Source',table.concat(protocols,',')}}
end

function services.cms.GetCurrentConnectionIDs()
    return {{'ConnectionIDs',''}}
end


services.msr.schema='urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1'

function services.msr.IsAuthorized(args)
    return {{'Result',1}}
end

function services.msr.RegisterDevice(args)
    return nil
end

function services.msr.IsValidated(args)
    return {{'Result',1}}
end
