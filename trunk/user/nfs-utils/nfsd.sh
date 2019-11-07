#!/bin/sh

func_start()
{
	# check rpcbind (needed for NFS)
	if [ -z "`pidof rpcbind`" ] ; then
		/sbin/rpcbind
		sleep 1
	fi

	# check NFS server already running
	if [ -n "`pidof nfsd`" ] && [ -n "`pidof rpc.mountd`" ] ; then
		# reload exports only
		[ -f /etc/exports ] && /sbin/exportfs -r
		return 0
	fi

	touch /etc/exports

	mkdir -p /var/lib/nfs/sm
	mkdir -p /var/lib/nfs/sm.bak
	mkdir -p /var/lib/nfs/rpc_pipefs

	echo -n > /var/lib/nfs/etab
	echo -n > /var/lib/nfs/rmtab
	echo -n > /var/lib/nfs/xtab

	if [ -z "`cat /proc/filesystems | grep nfsd`" ] ; then
		modprobe -q nfsd
		mount -t nfsd nfsd /proc/fs/nfsd
	fi

	if ! grep -q /var/lib/nfs/rpc_pipefs /proc/mounts ; then
		mount -t rpc_pipefs rpc_pipefs /var/lib/nfs/rpc_pipefs
	fi

	echo 65536 > /proc/fs/nfsd/max_block_size

	/sbin/exportfs -r

	/sbin/rpc.nfsd -N4 -N4.1 1
	/sbin/rpc.mountd

	if [ $? -eq 0 ] ; then
		logger -t "NFS server" "daemon is started"
	fi
}

func_reload()
{
	if [ -n "`pidof rpcbind`" ] && [ -f /etc/exports ] ; then
		/sbin/exportfs -r
	fi
}

func_stop()
{
	killall -q rpc.mountd
	killall -q -2 nfsd

	for i in 1 2 3 ; do
		[ -z "`pidof nfsd`" ] && break
		sleep 1
	done

	[ -n "`pidof nfsd`" ] && killall -q -9 nfsd

	if [ -n "`pidof rpcbind`" ] ; then
		if [ -f /var/lib/nfs/etab ] ; then
			/sbin/exportfs -ua
		fi
	fi

	umount /var/lib/nfs/rpc_pipefs 2>/dev/null
	umount /proc/fs/nfsd 2>/dev/null

	rm -f /var/lib/nfs/etab
}

case "$1" in
start)
	func_start
	;;
stop)
	func_stop
	;;
restart)
	func_stop
	sleep 1
	func_start
	;;
reload)
	func_reload
	;;
*)
	echo "Usage: $0 {start|stop|restart|reload}"
	exit 1
esac

exit $?

