#!/bin/bash

i=0

ip link set lowpan${i} down
ip link set wpan${i} down
ip link delete lowpan${i}

