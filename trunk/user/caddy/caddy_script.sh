#!/bin/sh
caddy_storage=`nvram get caddy_storage`
caddy_dir=`nvram get caddy_dir`
caddy_file=`nvram get caddy_file`
caddyf_wan_port=`nvram get caddyf_wan_port`
caddyw_wan_port=`nvram get caddyw_wan_port`
caddy_wname=`nvram get caddy_wname`
caddy_wpassword=`nvram get caddy_wpassword`
caddyfile="$caddy_dir/caddy/caddyfile"
rm -f $caddyfile
if [ "$caddy_file" = "0" ] || [ "$caddy_file" = "2" ]; then
cat <<-EOF >/tmp/cf
:$caddyf_wan_port {
 root $caddy_storage
 timeouts none
 gzip
 filebrowser / $caddy_storage {
  database /etc/storage/caddy_filebrowser.db
 }
}
EOF
fi
if [ "$caddy_file" = "1" ] || [ "$caddy_file" = "2" ]; then
cat <<-EOF >/tmp/cw
:$caddyw_wan_port {
root $caddy_storage
timeouts none
browse
gzip
filebrowser /document $caddy_storage {
  database /etc/storage/caddy_filebrowser.db
}
basicauth / $caddy_wname $caddy_wpassword
webdav /disk {
    scope $caddy_storage
    allow $caddy_storage 
}
}
EOF
fi
cat /tmp/cw /tmp/cf > $caddyfile
rm -f /tmp/cw
rm -f /tmp/cf
caddybin="/usr/bin/caddy_filebrowser"
if [ ! -f "$caddybin" ]; then
caddybin="$caddy_dir/caddy/caddy_filebrowser"
fi
$caddybin -conf $caddyfile &
