-- Copyright (C) 2011-2015 Anton Burdinuk
-- clark15b@gmail.com
-- https://tsdemuxer.googlecode.com/svn/trunk/xupnpd

-- HTML5 Player

webapp_show_playlists=true

function webapp_head()
    http_send_headers(200,'html')

    http.send('<!DOCTYPE html>\r\n<html>\r\n<head><meta http-equiv="Content-type" content="text/html; charset=utf-8"/></head>\r\n<body>\r\n')
end

function webapp_tail()

    http.send('</br><p align=left>\r\n')
    http.send('Copyright (C) 2011-2015 Anton Burdinuk &lt;<a href="mailto:clark15b@gmail.com">clark15b@gmail.com</a>&gt;<br/>\r\n')
    http.send('License: <a href="http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt">GPL-2.0</a><br/>')
    http.send('www: <a href="http://xupnpd.org">http://xupnpd.org</a><br/></p>')

    http.send('</body>\r\n</html>\r\n')
end

function webapp_play(src)
    http.send('<video id="video1" width="640" controls>\r\n')
    http.send(string.format('<source src="%s" type="video/mp4">\r\n',src))
    http.send('Your browser does not support HTML5 video.\r\n</video>\r\n')

    http.send('<script>\r\ndocument.getElementById("video1").play();\r\n</script>\r\n')

end


function webapp_handler(args,data,ip,url)
    
    local action=string.match(url,'^/app/(.+)$')
            
    if not action then action='list' end

    webapp_head()

    if action=='list' then

        http.send('<ul>\r\n')

        if not args.c then
            for i,j in ipairs(playlist_data.elements) do
                if j.filesystem then
                    http.send(string.format('<li><a href="/app/list?c=%s">%s</a></li>\r\n',i,j.name))
                end
            end

            if webapp_show_playlists then
                for i,j in ipairs(playlist_data.elements[1].elements) do
                    if j.plugin and j.type=='mp4' then
                        http.send(string.format('<li><a href="/app/list?c=1&sc=%s">%s</a></li>\r\n',i,j.name))
                    end
                end
            end

        else
            local c=tonumber(args.c)

            if c==1 then
                if args.sc and playlist_data.elements[c] then
                    local pls=playlist_data.elements[c].elements[tonumber(args.sc)]

                    for i,j in ipairs(pls.elements) do
                        if j.type=='mp4' then
                            http.send(string.format('<li><a href="/app/play?c=1&sc=%s&id=%s">%s</a></li>\r\n',args.sc,i,j.name))
                        end
                    end

                end
            else
                if playlist_data.elements[c] and playlist_data.elements[c].filesystem then
                    for i,j in ipairs(playlist_data.elements[c].elements) do
                        if j.type=='mp4' then
                            http.send(string.format('<li><a href="/app/play?c=%s&id=%s">%s</a></li>\r\n',c,i,j.name))
                        end
                    end
                end
            end
        end

        http.send('</ul>')

    elseif action=='play' and args.c and args.id then
        local c=tonumber(args.c)
        local id=tonumber(args.id)

        if c==1 then
            if args.sc and playlist_data.elements[c] then
                local pls=playlist_data.elements[c].elements[tonumber(args.sc)]

                if pls and pls.elements[id] then
                    pls=pls.elements[id]

                    if pls and pls.type=='mp4' and pls.plugin then
                        local u=string.format('/proxy/%s.%s',pls.objid,pls.type)
                        webapp_play(u)
                    end
                end
            end
        else
            if playlist_data.elements[c] and playlist_data.elements[c].filesystem then
                local pls=playlist_data.elements[c].elements[id]

                if pls and pls.type=='mp4' then
                    local u=string.format('/stream/%s.%s',pls.objid,pls.type)
                    webapp_play(u)
                end
            end
        end
    end

    webapp_tail()

end
