#!/bin/sh

func_start() {
	    ralinkiappd -wi rai0 -wi ra0 -d 10 &
	    sleep 2
	    ralinkiappd -wi rax0 -wi ra0 -d 10 &
	    sysctl -wq net.ipv4.neigh.br0.base_reachable_time_ms=10000
	    sysctl -wq net.ipv4.neigh.br0.delay_first_probe_time=1
	    sysctl -wq net.ipv4.neigh.rai0.base_reachable_time_ms=10000
	    sysctl -wq net.ipv4.neigh.rai0.delay_first_probe_time=1
	    sysctl -wq net.ipv4.neigh.ra0.base_reachable_time_ms=10000
	    sysctl -wq net.ipv4.neigh.ra0.delay_first_probe_time=1
	    sysctl -wq net.ipv4.neigh.rax0.base_reachable_time_ms=10000
	    sysctl -wq net.ipv4.neigh.rax0.delay_first_probe_time=1
	    iptables -A INPUT -i br0 -p tcp --dport 3517 -j ACCEPT
	    iptables -A INPUT -i br0 -p udp --dport 3517 -j ACCEPT
}

func_stop() {
            killall -q  ralinkiappd
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
	  sleep3
          func_start
            ;;

*)
          echo $"Usage: $0 {start|stop|restart}"
	  exit 1
esac
