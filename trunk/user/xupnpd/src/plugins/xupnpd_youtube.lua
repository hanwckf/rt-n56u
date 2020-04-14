-- Copyright (C) 2011-2013 Anton Burdinuk
-- clark15b@gmail.com
-- https://tsdemuxer.googlecode.com/svn/trunk/xupnpd

-- sysmer add support api v3 (need install curl with ssl)

-- 20150524 AnLeAl changes:
-- in url fetch, added docs section

-- 20150527 AnLeAl changes:
-- fixed for video get when less than 50
-- returned ui config for user amount video
-- add possibility get more than 50 videos

-- 20150527 MejGun changes:
-- code refactoring for feed update

-- 20150530 AnLeAl changes:
-- small code cleanup
-- added 'channel/mostpopular' for youtube mostpopular videos (it's only 30 from api), also region code from ui working
-- added favorites/username to get favorites
-- added search function

-- 20150531 AnLeAl changes:
-- fixed error when only first feed can get all videos for cfg.youtube_video_count and other no more 50
-- ui help updated
-- curl settings from cycles was moved to variables

-- 20170321 vidok changes:
-- added vlc descrambler
-- added youtube_preferred_resolution parameter

-- README
-- This is YouTube api v3 plugin for xupnpd.
-- For now only search username supported.
-- Quickstart:
-- 1. Place this file into xupnpd plugin directory.
-- 2. Go to google developers console: https://developers.google.com/youtube/registering_an_application?hl=ru
-- 3. You need API Key, choose Browser key: https://developers.google.com/youtube/registering_an_application?hl=ru#Create_API_Keys
-- 4. Don't use option: only allow referrals from domains.
-- 5. Replace '***' with your new key in section '&key=***' in this file. Save file.
-- 6. Restart xupnpd, remove any old feeds that was made for youtube earlier. Add new one based on username.
-- 7. Enjoy!


-- 18 - 360p  (MP4,h.264/AVC)
-- 22 - 720p  (MP4,h.264/AVC) hd
-- 37 - 1080p (MP4,h.264/AVC) hd
-- 82 - 360p  (MP4,h.264/AVC)    stereo3d
-- 83 - 480p  (MP4,h.264/AVC) hq stereo3d
-- 84 - 720p  (MP4,h.264/AVC) hd stereo3d
-- 85 - 1080p (MP4,h.264/AVC) hd stereo3d

cfg.youtube_preferred_resolution=1080
cfg.youtube_fmt=37
cfg.youtube_region='*'
cfg.youtube_video_count=100
-- cfg.youtube_api_key=123

youtube_api_url='https://www.googleapis.com/youtube/v3/'

function youtube_updatefeed(feed,friendly_name)

  local function isempty(s)
    return s == nil or s == ''
  end

  local rc=false

  local feed_url=nil
  local feed_urn=nil

  local tfeed=split_string(feed,'/')

  local feed_name='youtube_'..string.lower(string.gsub(feed,"[/ :\'\"]",'_'))

  local feed_m3u_path=cfg.feeds_path..feed_name..'.m3u'
  local tmp_m3u_path=cfg.tmp_path..feed_name..'.m3u'

  local dfd=io.open(tmp_m3u_path,'w+')

  if dfd then
    dfd:write('#EXTM3U name=\"',friendly_name or feed_name,'\" type=mp4 plugin=youtube\n')

    --------------------------------------------------------------------------------------------------
    --local getopt = '/mnt/sda1/iptv/curl/curl -k '
    local count = 0
    local totalres = 0
    local numA = 50 -- show 50 videos per page 0..50 from youtube api v3
    local pagetokenA = ''
    local nextpageA = ''
    local allpages = math.ceil(cfg.youtube_video_count/numA) -- check how much pages per 50 videos there can be
    local lastpage = allpages - 1
    local maxA = '&maxResults=' .. numA
    if cfg.youtube_video_count < numA then
      maxA = '&maxResults=' .. cfg.youtube_video_count
      numA = cfg.youtube_video_count
    end

    local keyA = '&key=***' -- change *** to your youtube api key from: https://console.developers.google.com
    local cA = ''
    local iA = ''
    local userA = tfeed[1]
    local uploads = ''
    local region = ''
    local enough = false
    
    -- Get what exactly user wants to get.
    if tfeed[1]=='channel' and tfeed[2]=='mostpopular' then
            if cfg.youtube_region and cfg.youtube_region~='*' then uploads='&regionCode='..cfg.youtube_region end
            iA = youtube_api_url..'videos?part=snippet&chart=mostPopular'

    elseif tfeed[1]=='favorites' then
            cA = youtube_api_url..'channels?part=contentDetails&forUsername='
            iA = youtube_api_url..'playlistItems?part=snippet&playlistId='
            userA = tfeed[2]
            local jsonA = cA .. userA .. keyA
	    --local url_data = io.popen(getopt .. jsonA)
	    --local user_data = url_data:read('*all')
	    --url_data:close()
        local user_data = http.download(jsonA)
	    local x=json.decode(user_data)
	    uploads = x['items'][1]['contentDetails']['relatedPlaylists']['favorites']
	    x=nil

    elseif tfeed[1]=='playlist' then
            uploads = tfeed[2]
            iA = youtube_api_url..'playlistItems?part=snippet&playlistId='

    elseif tfeed[1]=='channel' then
            cA = youtube_api_url..'channels?part=contentDetails&id='
            iA = youtube_api_url..'playlistItems?part=snippet&playlistId='
            local channel_id = tfeed[2]
            local jsonA = cA .. channel_id .. keyA
	    --local url_data = io.popen(getopt .. jsonA)
	    --local user_data = url_data:read('*all')
	    --url_data:close()
        local user_data = http.download(jsonA)
	    local x=json.decode(user_data)
	    uploads = x['items'][1]['contentDetails']['relatedPlaylists']['uploads']
	    x=nil

    elseif tfeed[1]=='search' then
            -- feed_urn='videos?vq='..util.urlencode(tfeed[2])..'&alt=json'
            if cfg.youtube_region and cfg.youtube_region~='*' then region='&regionCode='..cfg.youtube_region end
            iA = youtube_api_url..'search?type=video&part=snippet&order=date&q=' .. util.urlencode(tfeed[2]) .. '&videoDefinition=high&videoDimension=2d' .. region
            uploads = ''

    else
            cA = youtube_api_url..'channels?part=contentDetails&forUsername='
            iA = youtube_api_url..'playlistItems?part=snippet&playlistId='
            userA = tfeed[1]
            local jsonA = cA .. userA .. keyA
	    --local url_data = io.popen(getopt .. jsonA)
	    --local user_data = url_data:read('*all')
	    --url_data:close()
        local user_data = http.download(jsonA)
	    local x=json.decode(user_data)
	    uploads = x['items'][1]['contentDetails']['relatedPlaylists']['uploads']
	    x=nil
    end



    while true do
      local jsonA = iA .. uploads .. maxA .. pagetokenA .. keyA
      --local url_data = io.popen(getopt .. jsonA)
      --local item_data = url_data:read('*all')
      --url_data:close()
      local item_data = http.download(jsonA)
      if item_data == nil then
          if cfg.debug>0 then print('YouTube feed \''..feed_name..'\' NOT updated') end
          return rc
      end
      local x=json.decode(item_data)
      if isempty(x['pageInfo']) then
        break
      end
      totalres = x['pageInfo']['totalResults']
      local realpages = math.ceil(totalres/numA)
      local prelastpage = realpages - 1

      local items = nil
      local title = nil
      local url = nil
      local img = nil
      local countx = 0

      for key,value in pairs(x['items']) do
        count = count + 1
        if count > cfg.youtube_video_count then 
          enough = true
          break
        end
    	    if tfeed[1]=='channel' and tfeed[2]=='mostpopular' then
    		    items = value['id']
    		elseif tfeed[1]=='search' then
    		    items = value['id']['videoId']
        	else
    		    items = value['snippet']['resourceId']['videoId']
    	    end
        title = value['snippet']['title']
        url = 'https://www.youtube.com/watch?v=' .. items .. '&feature=youtube_gdata'
        img = 'http://i.ytimg.com/vi/' .. items .. '/mqdefault.jpg'
        dfd:write('#EXTINF:0 logo=',img,' ,',title,'\n',url,'\n')
      end

      if isempty(x['nextPageToken']) or enough then
        break 
      else
        nextpageA = x['nextPageToken']
        pagetokenA = '&pageToken=' .. nextpageA
      end
      x=nil
--    enough=nil
    end
    
    
    dfd:close()
    ---------------------------------------------------------------------------------------------------------

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

-- Helper to search and extract code from javascript stream
function js_extract( js, pattern )
    --js.i = 0 -- Reset to beginning
    --for line in buf_iter, js do
    for line in string.gmatch(js.stream,"(.-};)\r?\n" ) do
        local ex = string.match( line, pattern )
        if ex then
            return ex
        end
    end
    if cfg.debug>0 then print("Youtube.js_extract(pattern="..pattern.."). Couldn't process youtube video URL." ) end
    return nil
end

-- Descramble the URL signature using the javascript code that does that
-- in the web page
function js_descramble( sig, js_url )

--print("Youtube.js_descramble stream="..stream)
    -- Fetch javascript code
    local js = { stream = plugin_download(js_url) }
--print("Youtube.js_descramble js.stream="..js.stream)
    if not js.stream then
        if cfg.debug>0 then print("Youtube.js_descramble("..sig..", "..js_url.."). Couldn't process youtube video JS URL." ) end
        return sig
    end

    -- Look for the descrambler function's name
    -- c&&a.set("signature",br(c));
    local descrambler = js_extract( js, "%.set%(\"signature\",([^)]-)%(" )
--print("Youtube.js_descramble descrambler = "..descrambler)
    if not descrambler then
        if cfg.debug>0 then print ( "Youtube.js_descramble. ("..js.."). Couldn't extract youtube video URL signature descrambling function name" ) end
        js.stream=nil
        return sig
    end

    -- Fetch the code of the descrambler function
    -- Go=function(a){a=a.split("");Fo.sH(a,2);Fo.TU(a,28);Fo.TU(a,44);Fo.TU(a,26);Fo.TU(a,40);Fo.TU(a,64);Fo.TR(a,26);Fo.sH(a,1);return a.join("")};
    local rules = js_extract( js, "^"..descrambler.."=function%([^)]*%){(.-)};" )
--print("Youtube.js_descramble rules = "..rules)
    if not rules then
        if cfg.debug>0 then print ( "Youtube.js_descramble. Couldn't extract youtube video URL signature descrambling rules" ) end
        js.stream=nil
        return sig
    end

    -- Get the name of the helper object providing transformation definitions
    local helper = string.match( rules, ";(..)%...%(" )
--print("Youtube.js_descramble helper = "..helper)
    if not helper then
        if cfg.debug>0 then print ( "Youtube.js_descramble. Couldn't extract youtube video URL signature transformation helper name" ) end
        js.stream=nil
        return sig
    end

    -- Fetch the helper object code
    -- var Fo={TR:function(a){a.reverse()},TU:function(a,b){var c=a[0];a[0]=a[b%a.length];a[b]=c},sH:function(a,b){a.splice(0,b)}};
    local transformations = js_extract( js, "[ ,]"..helper.."={(.-)};" )
--print("Youtube.js_descramble transformations = "..transformations)
    js.stream=nil
    if not transformations then
        if cfg.debug>0 then print ( "Youtube.js_descramble. Couldn't extract youtube video URL signature transformation code" ) end
        return sig
    end

    -- Parse the helper object to map available transformations
    local trans = {}
    for meth,code in string.gmatch( transformations, "(..):function%([^)]*%){([^}]*)}" ) do
        -- a=a.reverse()
        if string.match( code, "%.reverse%(" ) then
          trans[meth] = "reverse"

        -- a.splice(0,b)
        elseif string.match( code, "%.splice%(") then
          trans[meth] = "slice"

        -- var c=a[0];a[0]=a[b%a.length];a[b]=c
        elseif string.match( code, "var c=" ) then
          trans[meth] = "swap"
        else
            if cfg.debug>0 then print ( "Youtube.js_descramble. Couldn't parse unknown youtube video URL signature transformation") end
        end
    end

    -- Parse descrambling rules, map them to known transformations
    -- and apply them on the signature
    local missing = false
    for meth,idx in string.gmatch( rules, "..%.(..)%([^,]+,(%d+)%)" ) do
        idx = tonumber( idx )

        if trans[meth] == "reverse" then
            sig = string.reverse( sig )

        elseif trans[meth] == "slice" then
            sig = string.sub( sig, idx + 1 )

        elseif trans[meth] == "swap" then
            if idx > 1 then
                sig = string.gsub( sig, "^(.)("..string.rep( ".", idx - 1 )..")(.)(.*)$", "%3%2%1%4" )
            elseif idx == 1 then
                sig = string.gsub( sig, "^(.)(.)", "%2%1" )
            end
        else
            if cfg.debug>0 then print ( "Youtube.js_descramble. Couldn't apply unknown youtube video URL signature transformation. missing = true") end
            missing = true
        end
    end
    if missing then
        if cfg.debug>0 then print ( "Youtube.js_descramble. Couldn't process youtube video URL. missing=true" ) end
    end
--print("Youtube.js_descramble sig = "..sig)

    return sig
end

-- decode URL
function unescape (s)
    s = string.gsub(s,"%%(%x%x)", function (h)
          return string.char(tonumber(h, 16))
        end)
    return s
end

-- Parse and pick our video URL
function pick_url( url_map, fmt, js_url )
    local path = nil
    for stream in string.gmatch( url_map, "[^,]+" ) do
        -- Apparently formats are listed in quality order,
        -- so we can afford to simply take the first one
        local itag = string.match( stream, "itag=(%d+)" )
        if not fmt or not itag or tonumber( itag ) == tonumber( fmt ) then
            local url = string.match( stream, "url=([^&,]+)" )
            if url then
--                url = vlc.strings.decode_uri( url )
                url = unescape (url)
--print( "Youtube.pick_url  unescape url="..url)

                local sig = string.match( stream, "sig=([^&,]+)" )
                if not sig then
                    -- Scrambled signature
                    sig = string.match( stream, "s=([^&,]+)" )
                    if sig then
                        if cfg.debug>0 then print( "Youtube.pick_url Found "..string.len( sig ).."-character scrambled signature for youtube video URL, attempting to descramble... " ) end
                        if js_url then
                            sig = js_descramble( sig, js_url )
                        else
                            if cfg.debug>0 then print("Youtube.pick_url "..js_url..". Couldn't process youtube video URL" ) end
                        end
                    end
                end
                local signature = ""
                if sig then
                    signature = "&signature="..sig
                end

--print( "Youtube.pick_url signature="..signature)
                path = url..signature
--print( "Youtube.pick_url path="..path)
                break
            end
        end
    end
    return path
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

-- Pick the most suited format available
function get_fmt( fmt_list )

    local prefres = cfg.youtube_preferred_resolution
    if prefres < 0 then
        return nil
    end

    local fmt = nil
    for itag,height in string.gmatch( fmt_list, "(%d+)/%d+x(%d+)/[^,]+" ) do
        -- Apparently formats are listed in quality
        -- order, so we take the first one that works,
        -- or fallback to the lowest quality
        fmt = itag
        -- remove WebM itag=43
        if tonumber(height) <= prefres and fmt~='43' then
            break
        end
    end
    return fmt
end

function youtube_get_video_url(youtube_url)
    local clip_page=plugin_download(youtube_url)
    if clip_page then
        local line=string.match(clip_page,'ytplayer.config%s*=%s*({.-});')
        clip_page=nil
                local js_url = string.match( line, "\"js\": *\"(.-)\"" )
                if js_url then
                    js_url = string.gsub( js_url, "\\/", "/" )
                    -- Resolve JS URL
                    if string.match( js_url, "^/[^/]" ) then
                        local authority = string.match( youtube_url, "://([^/]*)/" )
                        js_url = "//"..authority..js_url
                    end
                    js_url = string.gsub( js_url, "^//", string.match( youtube_url, ".-://" ) )
                end
                    fmt_list = string.match( line, "\"fmt_list\": *\"(.-)\"" )
                    if fmt_list then
                        fmt_list = string.gsub( fmt_list, "\\/", "/" )
                        fmt = get_fmt( fmt_list )
                    end

--    print ("fmt\r\n"..fmt)
                url_map = string.match( line, "\"url_encoded_fmt_stream_map\": *\"(.-)\"" )
                if url_map then
                    -- FIXME: do this properly
                    url_map = string.gsub( url_map, "\\u0026", "&" )
                    path = pick_url( url_map, fmt, js_url )
                end
--    print ("path\r\n"..path)

                if not path then
                    -- If this is a live stream, the URL map will be empty
                    -- and we get the URL from this field instead
                    local hlsvp = string.match( line, "\"hlsvp\": *\"(.-)\"" )
                    if hlsvp then
                        hlsvp = string.gsub( hlsvp, "\\/", "/" )
                        path = hlsvp
                    end
                end
    else
        if cfg.debug>0 then print('YouTube clip is not found') end
        return nil
    end
    return path;
end

function youtube_old_get_video_url(youtube_url)
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
      --print('old url='..url)
    end

    return url
  else
    if cfg.debug>0 then print('YouTube clip is not found') end
    return nil
  end
end

plugins['youtube']={}
plugins.youtube.name="YouTube"
plugins.youtube.desc="<i>username</i>, favorites/<i>username</i>, playlist/<i>idplaylist</i>, search/<i>search_string</i>"..
"<br/><b>YouTube channels</b>: channel/mostpopular, channel/<i>idchannel</i>"
plugins.youtube.sendurl=youtube_sendurl
plugins.youtube.updatefeed=youtube_updatefeed
plugins.youtube.getvideourl=youtube_get_video_url

plugins.youtube.ui_config_vars=
{
  { "select", "youtube_fmt", "int" },
  { "select", "youtube_region" },
  { "input",  "youtube_video_count", "int" }
  --    { "input",  "youtube_api_key", "int" }
}

--youtube_updatefeed('channel/top_rated','')
