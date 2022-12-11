#!/bin/bash

# restart networking
systemctl restart networking

# config tux3 ip
# CONNECTO TO ETH1
ifconfig eth2 up
ifconfig eth2 172.16.60.1/24
ifconfig eth2

read -n 1 -p "Set up the ip address in tuxY4 and tuxY2 then press any key to continue."

# add route to tux2 through tux4
route add -net 172.16.61.0/24 gw 172.16.60.254

read -n 1 -p "Set up the router ip and then click any key to continue."

# configure tuxY4 to be the default router
route add default gw 172.16.60.254

# configure DNS
echo 'nameserver 172.16.2.1' > /etc/resolv.conf
