#!/bin/sh

mount -t tmpfs tmpfs /dev   -o size=8K
mount -t tmpfs tmpfs /etc   -o size=1M
mount -t tmpfs tmpfs /home  -o size=1M
mount -t tmpfs tmpfs /media -o size=8K
mount -t tmpfs tmpfs /mnt   -o size=8K
mount -t tmpfs tmpfs /tmp   -o size=16M
mount -t tmpfs tmpfs /var   -o size=2M

mkdir /dev/pts
mount -t devpts devpts /dev/pts

mdev -s

# create nodes for loadable modules
mknod   /dev/rtl8367m	c	206	0
mknod   /dev/video0	c	81	0
mknod   /dev/spiS0	c	217	0
mknod   /dev/i2cM0	c	218	0
mknod   /dev/rdm0	c	254	0
mknod   /dev/flash0	c	200	0
mknod   /dev/swnat0	c	210	0
mknod   /dev/hwnat0	c	215	0
mknod   /dev/acl0	c	230	0
mknod   /dev/ac0	c	240	0
mknod   /dev/mtr0	c	250	0
mknod   /dev/gpio	c	252	0
mknod   /dev/nvram	c	228	0
#mknod   /dev/PCM	c	233	0
#mknod   /dev/I2S	c	234	0
#mknod   /dev/tun	c	10	200

cat > /etc/mdev.conf <<EOF
# <device regex> <uid>:<gid> <octal permissions> [<@|$|*> <command>]
# The special characters have the meaning:
# @ Run after creating the device.
# $ Run before removing the device.
# * Run both after creating and before removing the device.
lp[0-9] 0:0 0660 */sbin/asus_lp \$MDEV \$ACTION
sd[a-z] 0:0 0660 */sbin/asus_sd \$MDEV \$ACTION
sd[a-z][0-9] 0:0 0660 */sbin/asus_sd \$MDEV \$ACTION
sg[0-9] 0:0 0660 @/sbin/asus_sg \$MDEV \$ACTION
sr[0-9] 0:0 0660 @/sbin/asus_sr \$MDEV \$ACTION
usb[0-9] 0:0 0660 */sbin/asus_net \$MDEV \$ACTION
eth[0-9] 0:0 0660 */sbin/asus_net \$MDEV \$ACTION
wimax[0-9] 0:0 0660 */sbin/asus_net \$MDEV \$ACTION
ttyUSB[0-9] 0:0 0660 */sbin/asus_tty \$MDEV \$ACTION
ttyACM[0-9] 0:0 0660 */sbin/asus_tty \$MDEV \$ACTION
[1-2]-[1-2]:[1-9].[0-9] 0:0 0660 */sbin/asus_usb_interface \$MDEV \$ACTION
EOF

# enable usb hot-plug feature
echo "/sbin/mdev" > /proc/sys/kernel/hotplug

# create dirs
mkdir -p -m 777 "/var/lock"
mkdir -p -m 777 "/var/locks"
mkdir -p -m 777 "/var/private"
mkdir -p -m 777 "/var/lib"
mkdir -p -m 777 "/var/log"
mkdir -p -m 777 "/var/run"
mkdir -p -m 777 "/var/tmp"
mkdir -p -m 777 "/var/spool"
mkdir -p -m 777 "/var/lib/misc"
mkdir -p -m 777 "/var/state"
mkdir -p -m 777 "/var/state/parport"
mkdir -p -m 777 "/var/state/parport/svr_statue"
mkdir -p -m 777 "/var/spool/cron/crontabs"
mkdir -p -m 777 "/tmp/var"
mkdir -p -m 777 "/tmp/hashes"
mkdir -p -m 777 "/tmp/modem"
mkdir -p -m 777 "/tmp/rc_notification"
mkdir -p -m 777 "/tmp/rc_action_incomplete"
mkdir -p -m 700 "/home/root"
mkdir -p -m 755 "/etc/storage"
mkdir -p -m 755 "/etc/cron"

# extract storage files
mtd_storage.sh load

touch "/etc/resolv.conf"
cp -f "/etc_ro/ld.so.cache" "/etc"

# create symlinks
ln -sf "/home/root" "/home/admin"
ln -sf "/proc/mounts" "/etc/mtab"
ln -sf "/etc_ro/protocols" "/etc/protocols"
ln -sf "/etc_ro/services" "/etc/services"
ln -sf "/etc_ro/shells" "/etc/shells"
ln -sf "/etc_ro/profile" "/etc/profile"
ln -sf "/etc_ro/mke2fs.conf" "/etc/mke2fs.conf"
ln -sf "/etc_ro/e2fsck.conf" "/etc/e2fsck.conf"
ln -sf "/etc_ro/ipkg.conf" "/etc/ipkg.conf"
ln -sf "/etc_ro/ld.so.conf" "/etc/ld.so.conf"

# tune linux kernel
echo "1024 65535" > /proc/sys/net/ipv4/ip_local_port_range
echo 1            > /proc/sys/kernel/panic
echo 65536        > /proc/sys/fs/file-max

script_start="/etc/storage/start_script.sh"
script_postf="/etc/storage/post_iptables_script.sh"
script_postw="/etc/storage/post_wan_script.sh"
chap_secrets="/etc/storage/chap-secrets"
user_hosts="/etc/storage/hosts"

# create start script
if [ ! -f "$script_start" ] ; then
	reset_ss.sh -a
fi

# create post-iptables script
if [ ! -f "$script_postf" ] ; then
	cat > "$script_postf" <<EOF
#!/bin/sh

### Custom user script
### Called after internal iptables reconfig (firewall update)

EOF
	chmod 755 "$script_postf"
fi

# create post-wan script
if [ ! -f "$script_postw" ] ; then
	cat > "$script_postw" <<EOF
#!/bin/sh

### Custom user script
### Called after internal WAN up/down action
### \$1 - WAN action (up/down)
### \$2 - WAN interface name (e.g. eth3 or ppp0)

EOF
	chmod 755 "$script_postw"
fi

# create pptpd chap-secrets
if [ ! -f "$chap_secrets" ] ; then
	cat > "$chap_secrets" <<EOF
# Secrets for authentication pptpd using MS-CHAP V1/V2
# user		server		password		IP addresses
#myusername	*		mystrongpass		*
EOF
	chmod 600 "$chap_secrets"
fi

# create user hosts
if [ ! -f "$user_hosts" ] ; then
	cat > "$user_hosts" <<EOF
# Custom user hosts file
# Example:
# 192.168.1.100		Obi-Wan
EOF
	chmod 644 "$user_hosts"
fi

# prepare ssh authorized_keys
if [ -f /etc/storage/authorized_keys ] ; then
	mkdir -p -m 700 /home/root/.ssh
	cp -f /etc/storage/authorized_keys /home/root/.ssh
	chmod 600 /home/root/.ssh/authorized_keys
fi

# perform start script
if [ -x /etc/storage/start_script.sh ] ; then
	/etc/storage/start_script.sh &
fi
