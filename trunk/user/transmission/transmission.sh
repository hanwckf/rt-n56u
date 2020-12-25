#!/bin/sh

#######################################################################
# (1) run process from superuser root (less security)
# (0) run process from unprivileged user "nobody" (more security)
SVC_ROOT=0

# process priority (0-normal, 19-lowest)
SVC_PRIORITY=5
#######################################################################

SVC_NAME="Transmission"
SVC_PATH="/usr/bin/transmission-daemon"
DIR_LINK="/mnt/transmission"

func_start()
{
	# Make sure already running
	if [ -n "`pidof transmission-daemon`" ] ; then
		return 0
	fi
	
	echo -n "Starting $SVC_NAME:."
	
	if [ ! -d "${DIR_LINK}" ] ; then
		echo "[FAILED], unable to find target dir!"
		return 1
	fi

	export TR_CURL_SSL_NO_VERIFY=1
	
	DIR_CFG="${DIR_LINK}/config"
	DIR_DL1="`cd \"$DIR_LINK\"; dirname \"$(pwd -P)\"`/Downloads"
	[ ! -d "$DIR_DL1" ] && DIR_DL1="${DIR_LINK}/downloads"
	DIR_DL2="${DIR_LINK}/incomplete"
	DIR_DL3="${DIR_LINK}/watch"
	
	[ ! -d "$DIR_CFG" ] && mkdir -p "$DIR_CFG"
	[ ! -d "$DIR_DL1" ] && mkdir -p "$DIR_DL1"
	[ ! -d "$DIR_DL2" ] && mkdir -p "$DIR_DL2"
	[ ! -d "$DIR_DL3" ] && mkdir -p "$DIR_DL3"
	
	# full acces for watch dir
	chmod 777 "$DIR_DL3"
	
	tr_pport=`nvram get trmd_pport`
	tr_rport=`nvram get trmd_rport`
	
	# create default settings.json
	if [ ! -f "$DIR_CFG/settings.json" ] ; then
		tr_user=`nvram get http_username`
		tr_pass=`nvram get http_passwd`
		$SVC_PATH -w "$DIR_DL1" --incomplete-dir "$DIR_DL2" -c "$DIR_DL3" --no-incomplete-dir -y -L 90 -l 30 --no-utp -M -t -u "$tr_user" -v "$tr_pass" -P "$tr_pport" -p "$tr_rport" -x /var/run/transmission.pid -d 2>/tmp/settings.json
		sed -i 's|"umask": 18,|"umask": 0,|g' /tmp/settings.json
		mv /tmp/settings.json "$DIR_CFG/settings.json"
	fi
	
	svc_user=""
	
	# check start-stop-daemon stuff
	if [ $SVC_ROOT -eq 0 ] ; then
		chmod 777 "${DIR_LINK}"
		chown -R nobody "$DIR_CFG"
		chown -R nobody "$DIR_DL2"
		chmod -R 777 "$DIR_DL1"
		svc_user=" -c nobody"
	fi
	
	start-stop-daemon -S -N $SVC_PRIORITY$svc_user -x $SVC_PATH -- -g "$DIR_CFG" -P "$tr_pport" -p "$tr_rport" -e "${DIR_LINK}/transmission.log"
	
	if [ $? -eq 0 ] ; then
		echo "[  OK  ]"
		logger -t "$SVC_NAME" "daemon is started"
	else
		echo "[FAILED]"
	fi
}

func_stop()
{
	# Make sure not running
	if [ -z "`pidof transmission-daemon`" ] ; then
		return 0
	fi
	
	echo -n "Stopping $SVC_NAME:."
	
	# stop daemon
	killall -q transmission-daemon
	
	# gracefully wait max 25 seconds while transmission stopped
	i=0
	while [ -n "`pidof transmission-daemon`" ] && [ $i -le 25 ] ; do
		echo -n "."
		i=$(( $i + 1 ))
		sleep 1
	done
	
	tr_pid=`pidof transmission-daemon`
	if [ -n "$tr_pid" ] ; then
		# force kill (hungup?)
		kill -9 "$tr_pid"
		sleep 1
		echo "[KILLED]"
		logger -t "$SVC_NAME" "Cannot stop: Timeout reached! Force killed."
	else
		echo "[  OK  ]"
	fi
}

func_reload()
{
	if [ -n "`pidof transmission-daemon`" ] ; then
		echo -n "Reload $SVC_NAME config:."
		killall -SIGHUP transmission-daemon
		echo "[  OK  ]"
	fi
}

case "$1" in
start)
	func_start
	;;
stop)
	func_stop
	;;
reload)
	func_reload
	;;
restart)
	func_stop
	func_start
	;;
*)
	echo "Usage: $0 {start|stop|reload|restart}"
	exit 1
	;;
esac

