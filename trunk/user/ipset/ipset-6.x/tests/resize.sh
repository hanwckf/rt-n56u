#!/bin/bash

../src/ipset x resize-test 2>/dev/null

set -e

../src/ipset n resize-test hash:ip hashsize 64
for x in `seq 1 32`; do
   for y in `seq 1 255`; do
      ../src/ipset a resize-test 192.168.$x.$y
   done
done
