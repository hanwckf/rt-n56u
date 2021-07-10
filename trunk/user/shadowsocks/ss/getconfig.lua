local cjson = require "cjson"
local server_section = arg[1]

local ssrindext = io.popen("dbus get ssconf_basic_json_" .. server_section)
local servertmp = ssrindext:read("*all")
local server = cjson.decode(servertmp)
print(server.server)