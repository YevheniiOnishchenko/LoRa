#!/bin/bash

#   Since ipv6 addresses are evaluated from wpan short address, 
# we can automarically start client and server with correct one

DEFAULT_MAC=$(cat /sys/class/net/wlan0/address)
DEFAULT_PORT=12345

declare -A short_address_assoc=(
    ['e4:5f:01:55:95:66']="fe80::bcef:ff:fe00:1111%lowpan0"
    ['dc:a6:32:f1:3b:57']="fe80::bcef:ff:fe00:2222%lowpan0"
)

server_addr=${short_address_assoc[${DEFAULT_MAC}]}

echo "Running the server on ${server_addr}:${DEFAULT_PORT}"
./server ${server_addr} ${DEFAULT_PORT}