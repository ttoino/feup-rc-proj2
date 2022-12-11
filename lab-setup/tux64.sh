#!/bin/bash

# restart networking
systemctl restart networking

# config tux4 ip
ifconfig eth0 up
ifconfig eth0 172.16.60.254/24

ifconfig eth1 up
ifconfig eth1 172.16.61.253/24

# enable IP forwarding
echo 1 > /proc/sys/net/ipv4/ip_forward

# disable ICMP echo-ignore-broadcast
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts

read -n 1 -p "Set up the router ip and then click any key to continue."

# add Rc as the default router
route add default gw 172.16.61.254

# configure DNS
echo 'nameserver 172.16.2.1' > /etc/resolv.conf
