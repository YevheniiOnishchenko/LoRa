#!/bin/bash

DEFAULT_PORT=12345
DEFAULT_MSG="hello"

DEFAULT_MAC=$(cat /sys/class/net/wlan0/address)

declare -A self_addr_assoc=(
    ['e4:5f:01:55:95:66']="fe80::bcef:ff:fe00:1111%lowpan0"
    ['dc:a6:32:f1:3b:57']="fe80::bcef:ff:fe00:2222%lowpan0"
)

declare -A other_addr_assoc=(
    ['e4:5f:01:55:95:66']="fe80::bcef:ff:fe00:2222%lowpan0"
    ['dc:a6:32:f1:3b:57']="fe80::bcef:ff:fe00:1111%lowpan0"
)

self_addr=${self_addr_assoc[${DEFAULT_MAC}]} 
other_addr=${other_addr_assoc[${DEFAULT_MAC}]} 

echo "Connecting client (${self_addr}) to server (${other_addr}) on port ${DEFAULT_PORT}"
./client ${self_addr} ${other_addr} ${DEFAULT_PORT} ${DEFAULT_MSG}