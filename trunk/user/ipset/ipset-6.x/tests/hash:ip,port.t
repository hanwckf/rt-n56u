# Create a set with timeout
0 ipset create test hash:ip,port timeout 5
# Add partly zero valued element
0 ipset add test 2.0.0.1,0
# Test partly zero valued element
0 ipset test test 2.0.0.1,0
# Delete partly zero valued element
0 ipset del test 2.0.0.1,0
# Add first random value
0 ipset add test 2.0.0.1,5
# Add second random value
0 ipset add test 2.1.0.0,128
# Test first random value
0 ipset test test 2.0.0.1,5
# Test second random value
0 ipset test test 2.1.0.0,128
# Test value not added to the set
1 ipset test test 2.0.0.1,4
# Delete value not added to the set
1 ipset del test 2.0.0.1,6
# Test value before first random value
1 ipset test test 2.0.0.0,5
# Test value after second random value
1 ipset test test 2.1.0.1,128
# Try to add value before first random value
0 ipset add test 2.0.0.0,5
# Try to add value after second random value
0 ipset add test 2.1.0.1,128
# List set
0 ipset list test | grep -v Revision: | sed 's/timeout ./timeout x/' > .foo0 && ./sort.sh .foo0
# Check listing
0 diff -u -I 'Size in memory.*' .foo hash:ip,port.t.list0
# Sleep 5s so that elements can time out
0 sleep 5
# List set
0 ipset list test | grep -v Revision: > .foo0 && ./sort.sh .foo0
# Check listing
0 diff -u -I 'Size in memory.*' .foo hash:ip,port.t.list1
# Flush test set
0 ipset flush test
# Add multiple elements in one step
0 ipset add test 1.1.1.1-1.1.1.4,80-84
# Delete multiple elements in one step
0 ipset del test 1.1.1.2-1.1.1.3,tcp:81-82
# Check number of elements after multi-add/multi-del
0 n=`ipset save test|wc -l` && test $n -eq 17
# Delete test set
0 ipset destroy test
# Create a set
0 ipset create test hash:ip,port
# Add element without specifying protocol
0 ipset add test 2.0.0.1,80
# Add "same" element but with UDP protocol
0 ipset add test 2.0.0.1,udp:80
# Test element without specifying protocol
0 ipset test test 2.0.0.1,80
# Test element with TCP protocol
0 ipset test test 2.0.0.1,tcp:80
# Test element with UDP protocol
0 ipset test test 2.0.0.1,udp:80
# Add element with vrrp
0 ipset add test 2.0.0.1,vrrp:0
# Test element with vrrp
0 ipset test test 2.0.0.1,vrrp:0
# Add element with sctp
0 ipset add test 2.0.0.1,sctp:80
# Test element with sctp
0 ipset test test 2.0.0.1,sctp:80
# Delete element with sctp
0 ipset del test 2.0.0.1,sctp:80
# List set
0 ipset list test | grep -v Revision: > .foo0 && ./sort.sh .foo0
# Check listing
0 diff -u -I 'Size in memory.*' .foo hash:ip,port.t.list2
# Delete set
0 ipset destroy test
# Create set to add a range
0 ipset new test hash:ip,port hashsize 64
# Add a range which forces a resizing
0 ipset add test 10.0.0.0-10.0.3.255,tcp:80-82
# Check that correct number of elements are added
0 n=`ipset list test|grep '^10.0'|wc -l` && test $n -eq 3072
# Flush set
0 ipset flush test
# Add an single element
0 ipset add test 10.0.0.1,tcp:80
# Check number of elements
0 n=`ipset save test|wc -l` && test $n -eq 2
# Delete the single element
0 ipset del test 10.0.0.1,tcp:80
# Check number of elements
0 n=`ipset save test|wc -l` && test $n -eq 1
# Add an IP range
0 ipset add test 10.0.0.1-10.0.0.10,tcp:80
# Check number of elements
0 n=`ipset save test|wc -l` && test $n -eq 11
# Delete the IP range
0 ipset del test 10.0.0.1-10.0.0.10,tcp:80
# Check number of elements
0 n=`ipset save test|wc -l` && test $n -eq 1
# Add a port range
0 ipset add test 10.0.0.1,tcp:80-89
# Check number of elements
0 n=`ipset save test|wc -l` && test $n -eq 11
# Delete the port range
0 ipset del test 10.0.0.1,tcp:80-89
# Check number of elements
0 n=`ipset save test|wc -l` && test $n -eq 1
# Add an IP and port range
0 ipset add test 10.0.0.1-10.0.0.10,tcp:80-89
# Check number of elements
0 n=`ipset save test|wc -l` && test $n -eq 101
# Delete the IP and port range
0 ipset del test 10.0.0.1-10.0.0.10,tcp:80-89
# Check number of elements
0 n=`ipset save test|wc -l` && test $n -eq 1
# Destroy set
0 ipset -X test
# eof
