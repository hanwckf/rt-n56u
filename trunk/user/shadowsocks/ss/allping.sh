#!/bin/sh
num=1
dbus list ssconf_basic_json | cut -d '_' -f 4 | cut -d '=' -f 1 > /tmp/dlinkping.txt
logger -t "SS" "开始ping全部节点,cpu使用率可能会短暂升高..."
while read line
do
if [ "$num" != "20" ]; then
/etc_ro/ss/ping.sh $line >/dev/null 2>&1 &
else
sleep 10
num=1
fi
num=`expr $num + 1`
done < /tmp/dlinkping.txt
logger -t "SS" "已ping完所有节点,延迟和丢包率仅供参考,实际以连接速度为准！"