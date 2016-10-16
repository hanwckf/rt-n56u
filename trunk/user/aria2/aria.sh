#!/bin/sh

#######################################################################
# (1) run process from superuser root (less security)
# (0) run process from unprivileged user "nobody" (more security)
SVC_ROOT=0

# process priority (0-normal, 19-lowest)
SVC_PRIORITY=3
#######################################################################

SVC_NAME="Aria2"
SVC_PATH="/usr/bin/aria2c"
DIR_LINK="/mnt/aria"

func_start()
{
	# Make sure already running
	if [ -n "`pidof aria2c`" ] ; then
		return 0
	fi

	echo -n "Starting $SVC_NAME:."

	if [ ! -d "${DIR_LINK}" ] ; then
		echo "[FAILED]"
		logger -t "$SVC_NAME" "Cannot start: unable to find target dir!"
		return 1
	fi

	DIR_CFG="${DIR_LINK}/config"
	DIR_DL1="${DIR_LINK}/downloads"

	[ ! -d "$DIR_CFG" ] && mkdir -p "$DIR_CFG"

	FILE_CONF="$DIR_CFG/aria2.conf"
	FILE_LIST="$DIR_CFG/incomplete.lst"
	FILE_WEB_CONF="$DIR_CFG/configuration.js"

	touch "$FILE_LIST"

	aria_pport=`nvram get aria_pport`
	aria_rport=`nvram get aria_rport`
	aria_user=`nvram get http_username`
	aria_pass=`nvram get http_passwd`
	lan_ipaddr=`nvram get lan_ipaddr_t`

	[ -z "$aria_rport" ] && aria_rport="6800"
	[ -z "$aria_pport" ] && aria_pport="16888"

	if [ ! -f "$FILE_CONF" ] ; then
		[ ! -d "$DIR_DL1" ] && mkdir -p "$DIR_DL1"
		chmod -R 777 "$DIR_DL1"
		cat > "$FILE_CONF" <<EOF

### XML-RPC
rpc-listen-all=true
#rpc-secret=
rpc-user=$aria_user
rpc-passwd=$aria_pass

### Common
dir=$DIR_DL1
max-download-limit=0
max-overall-download-limit=6M
disable-ipv6=true

### File
file-allocation=trunc
#file-allocation=falloc
#file-allocation=none
no-file-allocation-limit=10M
allow-overwrite=false
auto-file-renaming=true

### Bittorent
bt-enable-lpd=false
#bt-lpd-interface=eth2.2
bt-max-peers=50
bt-max-open-files=100
bt-request-peer-speed-limit=100K
bt-stop-timeout=0
enable-dht=true
#enable-dht6=false
enable-peer-exchange=true
seed-ratio=1.5
#seed-time=60
max-upload-limit=0
max-overall-upload-limit=5M

### FTP/HTTP
ftp-pasv=true
ftp-type=binary
timeout=120
connect-timeout=60
split=5
max-concurrent-downloads=5
max-connection-per-server=1
min-split-size=20M

### Log
log=$DIR_CFG/aria2.log
log-level=notice

EOF
	fi

	if [ ! -f "$FILE_WEB_CONF" ] ; then
		cat > "$FILE_WEB_CONF" <<EOF
angular
.module('webui.services.configuration',  [])
.constant('\$name', 'Aria2 WebUI')
.constant('\$titlePattern', 'DL: {download_speed} - UL: {upload_speed}')
.constant('\$pageSize', 11)
.constant('\$authconf', {
  host: '$lan_ipaddr',
  path: '/jsonrpc',
  port: '$aria_rport',
  encrypt: false,
  auth: {
  //token: 'admin',
  user: '$aria_user',
  pass: '$aria_pass',
  },
  directURL: ''
})
.constant('\$enable', {
  torrent: true,
  metalink: true,
  sidebar: {
    show: true,
    stats: true,
    filters: true,
    starredProps: true,
  }
})
.constant('\$starredProps', [
  'dir', 'auto-file-renaming', 'max-connection-per-server'
])
.constant('\$downloadProps', [
  'pause', 'dir', 'max-connection-per-server'
])
.constant('\$globalTimeout', 1000)
;

EOF
	else
		old_host=`grep 'host:' $FILE_WEB_CONF | awk -F \' '{print $2}'`
		old_port=`grep 'port:' $FILE_WEB_CONF | awk -F \' '{print $2}'`
		[ "$old_host" != "$lan_ipaddr" ] && sed -i "s/\(host:\).*/\1\ \'$lan_ipaddr\'\,/" $FILE_WEB_CONF
		[ "$old_port" != "$aria_rport" ] && sed -i "s/\(port:\).*/\1\ \'$aria_rport\'\,/" $FILE_WEB_CONF
	fi

	# aria2 needed home dir
	export HOME="$DIR_CFG"

	svc_user=""

	if [ $SVC_ROOT -eq 0 ] ; then
		chmod 777 "${DIR_LINK}"
		chown -R nobody "$DIR_CFG"
		svc_user=" -c nobody"
	fi

	start-stop-daemon -S -N $SVC_PRIORITY$svc_user -x $SVC_PATH -- \
		-D --enable-rpc=true --conf-path="$FILE_CONF" --input-file="$FILE_LIST" --save-session="$FILE_LIST" \
		--rpc-listen-port="$aria_rport" --listen-port="$aria_pport" --dht-listen-port="$aria_pport"

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
	if [ -z "`pidof aria2c`" ] ; then
		return 0
	fi

	echo -n "Stopping $SVC_NAME:."

	# stop daemon
	killall -q aria2c

	# gracefully wait max 15 seconds while aria2c stopped
	i=0
	while [ -n "`pidof aria2c`" ] && [ $i -le 15 ] ; do
		echo -n "."
		i=$(( $i + 1 ))
		sleep 1
	done

	aria_pid=`pidof aria2c`
	if [ -n "$aria_pid" ] ; then
		# force kill (hungup?)
		kill -9 "$aria_pid"
		sleep 1
		echo "[KILLED]"
		logger -t "$SVC_NAME" "Cannot stop: Timeout reached! Force killed."
	else
		echo "[  OK  ]"
	fi
}

func_reload()
{
	aria_pid=`pidof aria2c`
	if [ -n "$aria_pid" ] ; then
		echo -n "Reload $SVC_NAME config:."
		kill -1 "$aria_pid"
		echo "[  OK  ]"
	else
		echo "Error: $SVC_NAME is not started!"
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
