#!/bin/sh

set -e

cmd=../src/ipset

$cmd f
$cmd x
$cmd n test hash:net
for x in `seq 1 32`; do
    $cmd a test 10.0.0.0/$x
    n=`$cmd l test | wc -l`
    n=$((n - 7))
    test $n -eq $x || exit 1
done
for x in `seq 32 -1 1`; do
    $cmd d test 10.0.0.0/$x
    n=`$cmd l test | wc -l`
    # We deleted one element
    n=$((n - 7 + 1))
    test $n -eq $x || exit 1
done
$cmd x test
