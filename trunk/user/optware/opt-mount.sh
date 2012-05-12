#!/bin/sh

self_name="opt-mount.sh"

logger -t "${self_name}" "started [$@]"

# check params
[ -z "$1" ] || [ -z "$2" ] && exit 1

# check /opt already mounted
grep -q /opt /proc/mounts && exit 0

# check dir "opt" exist on the drive root
[ ! -d "$2/opt" ] && exit 0

# mount /opt (bind only)
mount -o bind "$2/opt" /opt
if [ $? -ne 0 ] ; then
	logger -t "${self_name}" "Mount $2/opt to /opt FAILED! WTF?"
	exit 1
fi

# check dirs exist
for i in "bin" "etc/init.d" "home/admin" "lib" "sbin" "var/log" ; do
	[ ! -d /opt/${i} ] && mkdir -p /opt/${i}
done

# check opt profile exist
if [ ! -f /opt/etc/profile ] ; then
	cat > /opt/etc/profile <<EOF

# If running interactively, then
if [ "\$PS1" ] ; then

    export LANG=en_US.UTF-8

    alias mc='mc -c'

fi;

EOF
fi

# expand home to opt
if [ -d /opt/home/admin ] ; then
	rm -f /home/admin
	ln -sf /opt/home/admin /home/admin
	chmod 700 /opt/home/admin
fi

# prepare ssh authorized_keys
if [ -f /etc/storage/authorized_keys ] && [ ! -f /opt/home/admin/.ssh/authorized_keys ] ; then
	mkdir -p /opt/home/admin/.ssh
	cp -f /etc/storage/authorized_keys /opt/home/admin/.ssh
	chmod 700 /opt/home/admin/.ssh
	chmod 600 /opt/home/admin/.ssh/authorized_keys
fi

# check swap file exist
if [ -f /opt/.swap ] ; then
	swap_part=`cat /proc/swaps | grep 'partition' 2>/dev/null`
	swap_file=`cat /proc/swaps | grep 'file' 2>/dev/null`
	if [ -z "$swap_part" ] && [ -z "$swap_file" ] ; then
		swapon /opt/.swap
		[ $? -eq 0 ] && logger -t "${self_name}" "Activate swap file /opt/.swap SUCCESS!"
	fi
fi

# copy base scripts for transmission and aria2
for i in "01system" "95aria2" "95transmission" ; do
	if [ ! -f "/opt/etc/init.d/K${i}" ] && [ ! -f "/opt/etc/init.d/S${i}" ] ; then
		cp -f "/etc_ro/opt/K${i}" /opt/etc/init.d && chmod 755 "/opt/etc/init.d/K${i}"
	fi
done

# create iptables update script
iptables_script="/opt/bin/update_iptables.sh"
iptables_init_d="/opt/etc/init.d/S10iptables"
if [ ! -f "$iptables_script" ]  ; then
	cat > "$iptables_script" <<EOF
#!/bin/sh

### Custom user script for post-update iptables
### This script auto called after internal firewall restart
### First param is:
###  "start" (call at start optware),
###  "stop" (call before stop optware),
###  "update" (call after internal firewall restart).
### Include you custom rules for iptables below:

case "\$1" in
start|update)
	# add iptables custom rules
	echo "firewall started"
	;;
stop)
	# delete iptables custom rules
	echo "firewall stopped"
	;;
*)
	echo "Usage: \$0 {start|stop|update}"
	exit 1
	;;
esac

EOF
	chmod 755 "$iptables_script"
fi

if [ ! -L "$iptables_init_d" ]  ; then
	ln -sf "$iptables_script" "$iptables_init_d"
fi

# create script for WPS button events handling
wps_script="/opt/bin/on_wps.sh"
if [ ! -f "$wps_script" ]  ; then
	cat > "$wps_script" <<EOF
#!/bin/sh

### Custom user script for WPS button events handling

case "\$1" in
1)
	# WPS short pressed
	echo "WPS button short pressed!"
	;;
2)
	# WPS long pressed
	echo "WPS button long pressed!"
	;;
esac

EOF
	chmod 755 "$wps_script"
fi

# create script for printer hotplug event handling
lph_script="/opt/bin/on_hotplug_printer.sh"
if [ ! -f "$lph_script" ]  ; then
	cat > "$lph_script" <<EOF
#!/bin/sh

[ -z "\$1" ] && exit 1

### Custom user script for printer hotplug event handling
### First param is /dev/lp[0-9]

### Example: load firmware to printer HP LJ1020
lpfw="/opt/share/firmware/sihp1020.dl"
if [ -r "\$lpfw" ] ; then
	cat "\$lpfw" > "\$1"
fi

EOF
	chmod 755 "$lph_script"
fi

# extend path to optware
export PATH=/opt/sbin:/usr/sbin:/sbin:/opt/bin:/usr/bin:/bin

# start all services S* in /opt/etc/init.d
for i in `ls /opt/etc/init.d/S??* 2>/dev/null` ; do
	[ ! -x "${i}" ] && continue
	${i} start
done

# check ipkg installed
if [ ! -f /opt/bin/ipkg ] ; then
	# install and update ipkg in background
	/usr/bin/opt-ipkg-upd.sh &
fi
