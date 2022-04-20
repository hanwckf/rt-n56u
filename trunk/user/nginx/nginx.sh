#!/bin/sh

cat << EOF > /tmp/var/nginx.conf
events { }
http {
 server {
  listen 88;
  location / {
   proxy_pass http://$(nvram get lan_ipaddr):80;
   proxy_set_header Host host;
   proxy_set_header X-Real-IP emote_addr;
   proxy_set_header X-Forwarded_For proxy_add_x_forwarded_for;
  }
 }
} 
EOF
nginx
