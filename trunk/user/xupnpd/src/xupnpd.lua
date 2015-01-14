cfg={}

-- multicast interface for SSDP exchange, 'eth0', 'br0', 'br-lan' for example
cfg.ssdp_interface='br0'

-- 'cfg.ssdp_loop' enables multicast loop (if player and server in one host)
cfg.ssdp_loop=0

-- SSDP announcement interval
cfg.ssdp_notify_interval=10

-- SSDP announcement age
cfg.ssdp_max_age=1800

-- HTTP port for incoming connections
cfg.http_port=4044

-- syslog facility (syslog,local0-local7)
cfg.log_facility='local0'

-- 'cfg.daemon' detach server from terminal
cfg.daemon=true

-- silent mode - no logs, no pid file
cfg.embedded=true

-- 'cfg.debug' enables SSDP debug output to stdout (if cfg.daemon=false)
-- 0-off, 1-basic, 2-messages
cfg.debug=0

-- external 'udpxy' url for multicast playlists (udp://@...)
--cfg.udpxy_url='http://192.168.1.1:4022'

-- downstream interface for builtin multicast proxy (comment 'cfg.udpxy_url' for processing 'udp://@...' playlists)
cfg.mcast_interface='eth2.2'

-- 'cfg.proxy' enables proxy for injection DLNA headers to stream
-- 0-off, 1-radio, 2-radio/TV
cfg.proxy=2

-- User-Agent for proxy
cfg.user_agent='Mozilla/5.0'

-- I/O timeout
cfg.http_timeout=30

-- enables UPnP/DLNA notify when reload playlist
cfg.dlna_notify=true

-- UPnP/DLNA subscribe ttl
cfg.dlna_subscribe_ttl=1800

-- group by 'group-title'
cfg.group=true

-- sort files
cfg.sort_files=true

-- Device name
cfg.name='xUPNP-IPTV'

-- static device UUID, '60bd2fb3-dabe-cb14-c766-0e319b54c29a' for example or nil
cfg.uuid='60bd2fb3-dabe-cb14-c766-0e319b54c29a'

-- max url cache size
cfg.cache_size=8

-- url cache item ttl (sec)
cfg.cache_ttl=900

-- default mime type (mpeg, mpeg_ts, mpeg1, mpeg2, ts, ...)
cfg.default_mime_type='mpeg'

-- feeds update interval (seconds, 0 - disabled)
cfg.feeds_update_interval=7200
cfg.playlists_update_interval=0

-- playlist (m3u file path or path with alias
playlist=
{
--    { './playlists/mozhay.m3u', 'Mozhay.tv' },
--    { '/media', 'Local Media Files' }
--    { '/media', 'Private Media Files', '127.0.0.1;192.168.1.1' }  -- only for 127.0.0.1 and 192.168.1.1
}

-- feeds list (plugin, feed name, feed type)
feeds=
{
    { 'vimeo',          'channel/hd',           'Vimeo HD Channel' },
    { 'vimeo',          'channel/hdxs',         'Vimeo Xtreme sports' },
    { 'vimeo',          'channel/mtb',          'Vimeo MTB Channel' },
    { 'youtube',        'channel/top_rated',    'YouTube Top Rated' },
    { 'youtube',        'Drift0r',              'Drift0r' },
    { 'youtube',        'XboxAhoy',             'XboxAhoy' },
    { 'ag',             'videos',               'AG - New' },
    { 'ivi',            'new',                  'IVI - New' },
    { 'gametrailers',   'ps3',                  'GT - PS3' },
    { 'giantbomb',      'all',                  'GiantBomb - All' },
--    { 'dreambox',       'http://192.168.0.1:8001/','Dreambox1' },
}

-- log ident, pid file end www root
cfg.version='1.033'
cfg.log_ident='xupnpd'
cfg.pid_file='/var/run/'..cfg.log_ident..'.pid'
cfg.www_root='/usr/share/xupnpd/www/'
cfg.ui_path='/usr/share/xupnpd/ui/'
cfg.tmp_path='/tmp/'
cfg.config_path='/etc/storage/xupnpd/config/'
cfg.plugin_path='/etc/storage/xupnpd/plugins/'
cfg.playlists_path='/etc/storage/xupnpd/playlists/'
cfg.profiles='/etc/storage/xupnpd/profiles/'      -- device profiles feature
cfg.feeds_path='/tmp/xupnpd-feeds/'
cfg.drive=''                    -- reload playlists only if drive state=active/idle, example: cfg.drive='/dev/sda'

dofile('xupnpd_main.lua')
