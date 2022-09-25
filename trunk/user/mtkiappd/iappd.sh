#!/bin/sh

func_start() {
	    if grep -q 'rai0' /proc/interrupts; then
	    mtkiappd -wi rai0 -wi ra0  &
	    sysctl -wq net.ipv4.neigh.rai0.base_reachable_time_ms=10000
	    sysctl -wq net.ipv4.neigh.rai0.delay_first_probe_time=1
	    else
	    mtkiappd -wi rax0 -wi ra0  &
	    sysctl -wq net.ipv4.neigh.rax0.base_reachable_time_ms=10000
	    sysctl -wq net.ipv4.neigh.rax0.delay_first_probe_time=1	    
	    fi
	    sysctl -wq net.ipv4.neigh.br0.base_reachable_time_ms=10000
	    sysctl -wq net.ipv4.neigh.br0.delay_first_probe_time=1
	    sysctl -wq net.ipv4.neigh.ra0.base_reachable_time_ms=10000
	    sysctl -wq net.ipv4.neigh.ra0.delay_first_probe_time=1
	    iptables -A INPUT -i br0 -p tcp --dport 3517 -j ACCEPT
	    iptables -A INPUT -i br0 -p udp --dport 3517 -j ACCEPT 
}



case "$1" in
start)
          func_start
            ;;

*)
          echo $"Usage: $0 {start}"
	  exit 1
esac

