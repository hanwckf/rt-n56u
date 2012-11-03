
Ethtool readme for selecting different PHY address.

Before doing any ethtool command you should make sure the current PHY
address is expected. The default PHY address is 1(port 1).

You can change current PHY address to X(0~4) by doing follow command:
# echo X > /proc/rt2880/gmac

Ethtool command also would show the current PHY address as following.

# ethtool  eth2
Settings for eth2:
        Supported ports: [ TP MII ]
        Supported link modes:   10baseT/Half 10baseT/Full
                                100baseT/Half 100baseT/Full
        Supports auto-negotiation: Yes
        Advertised link modes:  10baseT/Half 10baseT/Full
                                100baseT/Half 100baseT/Full
        Advertised auto-negotiation: No
        Speed: 10Mb/s
        Duplex: Full
        Port: MII
        PHYAD: 1
        Transceiver: internal
        Auto-negotiation: off
        Current message level: 0x00000000 (0)
        Link detected: no


The "PHYAD" field shows the current PHY address.



Usage example
1) show port1 info
# echo 1 > /proc/rt2880/gmac		# change phy address to 1
# ethtool eth2

2) show port0 info
# echo 0 > /proc/rt2880/gmac		# change phy address to 0
# ethtool eth2


