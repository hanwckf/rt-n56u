#!/bin/sh

dir_storage="/etc/storage/openssh"
sshd_config="$dir_storage/sshd_config"

rsa_key="$dir_storage/ssh_host_rsa_key"
dsa_key="$dir_storage/ssh_host_dsa_key"
ed25519_key="$dir_storage/ssh_host_ed25519_key"

func_create_config()
{
	cat > "$sshd_config" <<EOF
# The strategy used for options in this file is to specify options with
# their default value where possible, but leave them commented.
# Uncommented options override the default value.

#Port 22
#AddressFamily any
#ListenAddress 0.0.0.0
#ListenAddress ::

# The default requires explicit activation of protocol 1
#Protocol 2

# HostKey for protocol version 1
#HostKey ${dir_storage}/ssh_host_key

# HostKeys for protocol version 2
HostKey ${rsa_key}
HostKey ${dsa_key}
#HostKey ${dir_storage}/ssh_host_ecdsa_key
HostKey ${ed25519_key}

# Lifetime and size of ephemeral version 1 server key
#KeyRegenerationInterval 1h
#ServerKeyBits 1024

# Ciphers and keying
#RekeyLimit default none

# Logging
# obsoletes QuietMode and FascistLogging
#SyslogFacility AUTH
#LogLevel INFO

# Authentication:

LoginGraceTime 1m
PermitRootLogin yes
#StrictModes yes
MaxAuthTries 3
#MaxSessions 10

#RSAAuthentication yes
#PubkeyAuthentication yes

# The default is to check both .ssh/authorized_keys and .ssh/authorized_keys2
# but this is overridden so installations will only check .ssh/authorized_keys
AuthorizedKeysFile	.ssh/authorized_keys

#AuthorizedPrincipalsFile none

#AuthorizedKeysCommand none
#AuthorizedKeysCommandUser nobody

# For this to work you will also need host keys in ${dir_storage}/ssh_known_hosts
#RhostsRSAAuthentication no
# similar for protocol version 2
#HostbasedAuthentication no
# Change to yes if you don't trust ~/.ssh/known_hosts for
# RhostsRSAAuthentication and HostbasedAuthentication
IgnoreUserKnownHosts yes
# Don't read the user's ~/.rhosts and ~/.shosts files
#IgnoreRhosts yes

# To disable tunneled clear text passwords, change to no here!
#PasswordAuthentication yes
#PermitEmptyPasswords no

# Change to no to disable s/key passwords
ChallengeResponseAuthentication no

AllowAgentForwarding no
#AllowTcpForwarding yes
#GatewayPorts no
#X11Forwarding no
#X11DisplayOffset 10
#X11UseLocalhost yes
#PermitTTY yes
#PrintMotd yes
#PrintLastLog yes
#TCPKeepAlive yes
#UseLogin no
#PermitUserEnvironment no
#Compression delayed
#ClientAliveInterval 0
#ClientAliveCountMax 3
#UseDNS no
PidFile /var/run/sshd.pid
#MaxStartups 10:30:100
#PermitTunnel no
#ChrootDirectory none
#VersionAddendum none

# no default banner path
#Banner none

# override default of no subsystems
Subsystem	sftp	/usr/libexec/sftp-server

EOF
	chmod 644 "$sshd_config"
}

func_start()
{
	[ ! -d "$dir_storage" ] && mkdir -p -m 755 $dir_storage

	old_path="/etc/storage"
	rm -f "${old_path}/sshd_config"
	for i in ssh_host_rsa_key ssh_host_dsa_key ; do
		[ -f "${old_path}/${i}" ] && mv -n "${old_path}/${i}" "$dir_storage"
		[ -f "${old_path}/${i}.pub" ] && mv -n "${old_path}/${i}.pub" "$dir_storage"
	done

	if [ ! -f "$rsa_key" ] || [ ! -f "$dsa_key" ] || [ ! -f "$ed25519_key" ] ; then
		/usr/bin/ssh-keygen -A
	fi

	if [ ! -f "$sshd_config" ] ; then
		func_create_config
	fi

	touch /var/run/utmp

	if [ -n "$1" ] ; then
		/usr/sbin/sshd -o PasswordAuthentication=no
	else
		/usr/sbin/sshd
	fi
}

func_stop()
{
	killall -q sshd
}

func_reload()
{
	kill -SIGHUP `cat /var/run/sshd.pid`
}

case "$1" in
start)
	func_start $2
	;;
stop)
	func_stop
	;;
reload)
	func_reload
	;;
*)
	echo "Usage: $0 {start|stop|reload}"
	exit 1
	;;
esac

exit 0
