local cjson = require "cjson"
local server_section = arg[1]
--local proto = arg[2]
local local_port = arg[2]

local ssrindext = io.popen("dbus get ssconf_basic_json_" .. server_section)
local servertmp = ssrindext:read("*all")
local server = cjson.decode(servertmp)
local ssr = {
	server = server.server,
	server_port = server.server_port,
	local_address = "0.0.0.0",
	local_port = local_port,
	password = server.password,
	timeout = 60,
	method = server.encrypt_method,
	protocol = server.protocol,
	protocol_param = server.protocol_param,
	obfs = server.obfs,
	obfs_param = server.obfs_param,
	reuse_port = true,
	fast_open = false
}

print(cjson.encode(ssr))