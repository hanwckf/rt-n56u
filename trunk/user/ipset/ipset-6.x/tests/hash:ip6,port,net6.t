# Range: Create a set
0 ipset -N test ipportnethash -6
# Range: Add zero valued element
1 ipset -A test 2:0:0::1,0,0:0:0::0/0
# Range: Test zero valued element
1 ipset -T test 2:0:0::1,0,0:0:0::0/0
# Range: Delete zero valued element
1 ipset -D test 2:0:0::1,0,0:0:0::0/0
# Range: Add almost zero valued element
0 ipset -A test 2:0:0::1,0,0:0:0::0/24
# Range: Test almost zero valued element
0 ipset -T test 2:0:0::1,0,0:0:0::0/24
# Range: Delete almost zero valued element
0 ipset -D test 2:0:0::1,0,0:0:0::0/24
# Range: Add first random value
0 ipset -A test 2:0:0::1,5,1:1:1::1/24
# Range: Add second random value
0 ipset -A test 2:1:0::0,128,2:2:2::2/12
# Range: Test first random value
0 ipset -T test 2:0:0::1,5,1:1:1::2
# Range: Test second random value
0 ipset -T test 2:1:0::0,128,2:2:2::0
# Range: Test value not added to the set
1 ipset -T test 2:0:0::1,5,2:1:1::255
# Range: Test value not added to the set
1 ipset -T test 2:0:0::1,6,1:1:1::1
# Range: Test value not added to the set
1 ipset -T test 2:0:0::2,6,1:1:1::1
# Range: Test value before first random value
1 ipset -T test 2:0:0::0,5,1:1:1::1
# Range: Test value after second random value
1 ipset -T test 2:1:0::1,128,2:2:2::2
# Range: Try to add value before first random value
0 ipset -A test 2:0:0::0,5,1:1:1::1/24
# Range: Try to add value after second random value
0 ipset -A test 2:1:0::1,128,2:2:2::2/12
# Range: List set
0 ipset -L test | grep -v Revision: > .foo0 && ./sort.sh .foo0
# Range: Check listing
0 diff -u -I 'Size in memory.*' .foo hash:ip6,port,net6.t.list0
# Range: Flush test set
0 ipset -F test
# Range: Delete test set
0 ipset -X test
# Create set to add a range
0 ipset new test hash:ip,port,net -6 hashsize 64
# Add a range which forces a resizing
0 ipset add test 1::1,tcp:80-1105,2::2/12
# Check that correct number of elements are added
0 n=`ipset list test|grep 1::1|wc -l` && test $n -eq 1026
# Destroy set
0 ipset -X test
# Create test set with timeout support
0 ipset create test hash:ip,port,net family inet6 timeout 30
# Add a non-matching IP address entry
0 ipset -A test 2:2:2::2,80,1:1:1::1 nomatch
# Add an overlapping matching small net
0 ipset -A test 2:2:2::2,80,1:1:1::/124 
# Add an overlapping non-matching larger net
0 ipset -A test 2:2:2::2,80,1:1:1::/120 nomatch
# Add an even larger matching net
0 ipset -A test 2:2:2::2,80,1:1:1::/116
# Check non-matching IP
1 ipset -T test 2:2:2::2,80,1:1:1::1
# Check matching IP from non-matchin small net
0 ipset -T test 2:2:2::2,80,1:1:1::F
# Check non-matching IP from larger net
1 ipset -T test 2:2:2::2,80,1:1:1::10
# Check matching IP from even larger net
0 ipset -T test 2:2:2::2,80,1:1:1::100
# Update non-matching IP to matching one
0 ipset -! -A test 2:2:2::2,80,1:1:1::1
# Delete overlapping small net
0 ipset -D test 2:2:2::2,80,1:1:1::/124
# Check matching IP
0 ipset -T test 2:2:2::2,80,1:1:1::1
# Add overlapping small net
0 ipset -A test 2:2:2::2,80,1:1:1::/124
# Update matching IP as a non-matching one, with shorter timeout
0 ipset -! -A test 2:2:2::2,80,1:1:1::1 nomatch timeout 2
# Check non-matching IP
1 ipset -T test 2:2:2::2,80,1:1:1::1
# Sleep 3s so that element can time out
0 sleep 3
# Check non-matching IP
0 ipset -T test 2:2:2::2,80,1:1:1::1
# Check matching IP
0 ipset -T test 2:2:2::2,80,1:1:1::F
# Delete test set
0 ipset destroy test
# eof
