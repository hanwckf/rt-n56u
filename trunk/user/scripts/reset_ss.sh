#!/bin/sh

script_start="/etc/storage/start_script.sh"

# create start script
cat > "$script_start" <<EOF
#!/bin/sh

### Custom user script for tuning router on start

########################################################
### tune linux kernel
########################################################
# igmp
echo 30       > /proc/sys/net/ipv4/igmp_max_memberships
echo 2        > /proc/sys/net/ipv4/conf/all/force_igmp_version
echo 2        > /proc/sys/net/ipv4/conf/default/force_igmp_version

# arp
echo 1        > /proc/sys/net/ipv4/conf/all/arp_filter
echo 1        > /proc/sys/net/ipv4/conf/all/arp_announce
echo 1        > /proc/sys/net/ipv4/conf/default/arp_announce

# reverse-path filter
echo 1        > /proc/sys/net/ipv4/conf/all/rp_filter
echo 1        > /proc/sys/net/ipv4/conf/default/rp_filter
echo 1        > /proc/sys/net/ipv4/conf/lo/rp_filter
echo 1        > /proc/sys/net/ipv4/conf/eth2/rp_filter

# route
echo 16384    > /proc/sys/net/ipv4/route/max_size
echo 2        > /proc/sys/net/ipv4/route/gc_elasticity

# conntrack
echo 300      > /proc/sys/net/netfilter/nf_conntrack_generic_timeout
echo 1200     > /proc/sys/net/netfilter/nf_conntrack_tcp_timeout_established
echo 30       > /proc/sys/net/netfilter/nf_conntrack_icmp_timeout

# panic
echo 1        > /proc/sys/kernel/panic
echo 1        > /proc/sys/kernel/panic_on_oops
echo 0        > /proc/sys/vm/panic_on_oom

# vm
echo 16384    > /proc/sys/vm/min_free_kbytes
echo 2        > /proc/sys/vm/overcommit_memory
echo 60       > /proc/sys/vm/overcommit_ratio
echo 5        > /proc/sys/vm/dirty_background_ratio
echo 20       > /proc/sys/vm/dirty_ratio
echo 60       > /proc/sys/vm/swappiness

EOF
chmod 755 "$script_start"

if [ -z "$1" ] ; then
	$script_start
	mtd_storage.sh save
fi
