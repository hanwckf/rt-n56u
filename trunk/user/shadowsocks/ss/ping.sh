#!/bin/sh
server_address=`lua /etc_ro/ss/getconfig.lua $1`
ping_text=`ping -4 $server_address -c 5 -w 5 -q`
ping_time=`echo $ping_text | awk -F '/' '{print $4}'`
ping_loss=`echo $ping_text | awk -F ', ' '{print $3}' | awk '{print $1}'`
if [ -z "$ping_time" ]; then
	ping_time="failed"
else
ping_time=$(echo $ping_time | sed "s/\..*//g")
fi
lua /etc_ro/ss/ssping.lua $ping_time $ping_loss $1
