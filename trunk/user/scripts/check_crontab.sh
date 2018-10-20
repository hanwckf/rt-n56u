#!/bin/sh
# $1-5: crontab expr, eg: a/1 a a a a
# $6: script name
[ -z "$6" ] && exit 0
cd /etc/storage/
exp=`echo "$1 $2 $3 $4 $5" |sed 's/a/\*/g'`
if [ ! -f "cron/crontabs/admin" ] || [ -z "$(cat cron/crontabs/admin |grep $6)" ]; then
	echo "$exp /usr/bin/$6 > /dev/null 2>&1" >> cron/crontabs/admin && exit 1
fi
exit 0
