#!/bin/bash

set -e

DEFAULT_MAC=$(cat /sys/class/net/wlan0/address)

declare -A short_address_assoc=(
    ['e4:5f:01:55:95:66']="0x1111"
    ['dc:a6:32:f1:3b:57']="0x2222"
)

panid="0xbeef"
short_addr=${short_address_assoc[${DEFAULT_MAC}]}
i=0

echo "Associating with $short_addr short address"

iwpan phy phy1 set channel 0 0
iwpan dev wpan${i} set pan_id $panid
iwpan dev wpan${i} set short_addr ${short_addr}

ip link add link wpan${i} name lowpan${i} type lowpan
sysctl -w net.ipv6.conf.lowpan0.accept_dad=0
sysctl -w net.ipv6.conf.lowpan0.dad_transmits=0
sysctl -w net.ipv6.conf.lowpan0.accept_ra=0
sysctl -w net.ipv6.conf.lowpan0.autoconf=0
sysctl -w net.ipv6.conf.lowpan0.accept_ra_pinfo=0
sysctl -w net.ipv6.conf.lowpan0.dad_transmits=0
sysctl -w net.ipv6.conf.lowpan0.enhanced_dad=0

sysctl -p

ip link set wpan${i} up
ip link set lowpan${i} up