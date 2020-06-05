local cjson = require "cjson"
local server_section = arg[1]
--local proto = arg[2]
local local_port = arg[2]

local ssrindext = io.popen("dbus get ssconf_basic_json_" .. server_section)
local servertmp = ssrindext:read("*all")
local server = cjson.decode(servertmp)
io.popen('ipt2socks -s ' ..server.server.. ' -p ' ..server.server_port.. ' -l ' ..local_port.. ' -r -4 -R >/dev/null 2>&1 &')