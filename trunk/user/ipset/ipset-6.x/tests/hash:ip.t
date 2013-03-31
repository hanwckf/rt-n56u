# IP: Create a set with timeout
0 ipset -N test iphash --hashsize 128 timeout 5
# Range: Add zero valued element
1 ipset -A test 0.0.0.0
# Range: Test zero valued element
1 ipset -T test 0.0.0.0
# IP: Add first random value
0 ipset -A test 2.0.0.1 timeout 5
# IP: Add second random value
0 ipset -A test 192.168.68.69 timeout 0
# IP: Test first random value
0 ipset -T test 2.0.0.1
# IP: Test second random value
0 ipset -T test 192.168.68.69
# IP: Test value not added to the set
1 ipset -T test 2.0.0.2
# IP: Add third random value
0 ipset -A test 200.100.0.12
# IP: Delete the same value
0 ipset -D test 200.100.0.12
# IP: List set
0 ipset -L test | grep -v Revision: | sed 's/timeout ./timeout x/' > .foo0 && ./sort.sh .foo0
# IP: Check listing
0 diff -u -I 'Size in memory.*' .foo hash:ip.t.list2
# Sleep 5s so that element can time out
0 sleep 5
# IP: List set
0 ipset -L test 2>/dev/null | grep -v Revision: > .foo0 && ./sort.sh .foo0
# IP: Check listing
0 diff -u -I 'Size in memory.*' .foo hash:ip.t.list0
# IP: Flush test set
0 ipset -F test
# IP: Add multiple elements in one step
0 ipset -A test 1.1.1.1-1.1.1.5
# IP: Delete multiple elements in one step
0 ipset -D test 1.1.1.2-1.1.1.5
# IP: Test element after deletion
0 ipset -T test 1.1.1.1
# IP: Test deleted element
1 ipset -T test 1.1.1.2
# IP: Delete test set
0 ipset -X test
# IP: Restore values so that rehashing is triggered
0 sed 's/hashsize 128/hashsize 128 timeout 5/' iphash.t.restore | ipset -R
# IP: Check that the values are restored
0 test `ipset -S test| grep add| wc -l` -eq 129
# Sleep 5s so that elements can time out
0 sleep 5
# IP: check that elements timed out
0 test `ipset -S test| grep add| wc -l` -eq 0
# IP: Flush test set
0 ipset -F test
# IP: Stress test resizing
0 ./resize.sh
# IP: Check listing, which requires multiple messages
0 n=`ipset -S resize-test | wc -l` && test $n -eq 8161
# IP: Swap test and resize-test sets
0 ipset -W test resize-test
# IP: Check listing, which requires multiple messages
0 n=`ipset -S test | wc -l` && test $n -eq 8161
# IP: Destroy sets
0 ipset -X
# IP: Create set to add a range
0 ipset new test hash:ip hashsize 64
# IP: Add a range which forces a resizing
0 ipset add test 10.0.0.0-10.0.3.255
# IP: Check that correct number of elements are added
0 n=`ipset list test|grep '^10.0'|wc -l` && test $n -eq 1024
# IP: Destroy sets
0 ipset -X
# Network: Create a set with timeout
0 ipset -N test iphash --hashsize 128 --netmask 24 timeout 5
# Network: Add zero valued element
1 ipset -A test 0.0.0.0
# Network: Test zero valued element
1 ipset -T test 0.0.0.0
# Network: Delete zero valued element
1 ipset -D test 0.0.0.0
# Network: Add first random network
0 ipset -A test 2.0.0.1
# Network: Add second random network
0 ipset -A test 192.168.68.69
# Network: Test first random value
0 ipset -T test 2.0.0.255
# Network: Test second random value
0 ipset -T test 192.168.68.95
# Network: Test value not added to the set
1 ipset -T test 2.0.1.0
# Network: Add third element
0 ipset -A test 200.100.10.1 timeout 0
# Network: Add third random network
0 ipset -A test 200.100.0.12
# Network: Delete the same network
0 ipset -D test 200.100.0.12
# Network: List set
0 ipset -L test | grep -v Revision: | sed 's/timeout ./timeout x/' > .foo0 && ./sort.sh .foo0
# Network: Check listing
0 diff -u -I 'Size in memory.*' -I 'Size in memory.*' .foo hash:ip.t.list3
# Sleep 5s so that elements can time out
0 sleep 5
# Network: List set
0 ipset -L test | grep -v Revision: > .foo
# Network: Check listing
0 diff -u -I 'Size in memory.*' .foo hash:ip.t.list1
# Network: Flush test set
0 ipset -F test
# Network: add element with 1s timeout
0 ipset add test 200.100.0.12 timeout 1
# Network: readd element with 3s timeout
0 ipset add test 200.100.0.12 timeout 3 -exist
# Network: sleep 2s
0 sleep 2s
# Network: check readded element
0 ipset test test 200.100.0.12
# Network: Delete test set
0 ipset -X test
# Range: Create set
0 ipset create test hash:ip
# Range: Add a single element
0 ipset add test 10.0.0.1
# Range: Check number of elements
0 n=`ipset save test|wc -l` && test $n -eq 2
# Range: Delete the element
0 ipset del test 10.0.0.1
# Range: Check number of elements
0 n=`ipset save test|wc -l` && test $n -eq 1
# Range: Add a range
0 ipset add test 10.0.0.1-10.0.0.10
# Range: Check number of elements
0 n=`ipset save test|wc -l` && test $n -eq 11
# Range: Delete a range
0 ipset del test 10.0.0.1-10.0.0.10
# Range: Check number of elements
0 n=`ipset save test|wc -l` && test $n -eq 1
# Range: Delete test set
0 ipset destroy test
# eof
