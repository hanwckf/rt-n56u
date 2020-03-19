local cjson = require "cjson"
local server_section = arg[1]
--local proto = arg[2]
local local_port = arg[2]

local ssrindext = io.popen("dbus get ssconf_basic_json_" .. server_section)
local servertmp = ssrindext:read("*all")
local server = cjson.decode(servertmp)
print('listen-addr="0.0.0.0:' ..local_port.. '"')
print('proxy-addr="socks5://' ..server.server .. ':' ..server.server_port.. '"')