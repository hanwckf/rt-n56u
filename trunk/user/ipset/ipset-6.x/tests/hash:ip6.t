# IP: Create a set with timeout
0 ipset -N test iphash -6 --hashsize 128 timeout 5
# IP: Add zero valued element
1 ipset -A test ::
# IP: Test zero valued element
1 ipset -T test ::
# IP: Delete zero valued element
1 ipset -D test ::
# IP: Add first random value
0 ipset -A test 2:0:0::1 timeout 5
# IP: Add second random value
0 ipset -A test 192:168:68::69 timeout 0
# IP: Test first random value
0 ipset -T test 2:0:0::1
# IP: Test second random value
0 ipset -T test 192:168:68::69
# IP: Test value not added to the set
1 ipset -T test 2:0:0::2
# IP: Add third random value
0 ipset -A test 200:100:0::12
# IP: Delete the same value
0 ipset -D test 200:100:0::12
# IP: List set
0 ipset -L test | grep -v Revision: | sed 's/timeout ./timeout x/' > .foo0 && ./sort.sh .foo0
# IP: Check listing
0 diff -u -I 'Size in memory.*' .foo hash:ip6.t.list2
# IP: Save set
0 ipset save test > hash:ip6.t.restore
# Sleep 5s so that element can time out
0 sleep 5
# IP: List set
0 ipset -L test 2>/dev/null | grep -v Revision: > .foo0 && ./sort.sh .foo0
# IP: Check listing
0 diff -u -I 'Size in memory.*' .foo hash:ip6.t.list0
# IP: Destroy set
0 ipset x test
# IP: Restore saved set
0 ipset restore < hash:ip6.t.restore && rm hash:ip6.t.restore
# IP: List set
0 ipset -L test | grep -v Revision: | sed 's/timeout ./timeout x/' > .foo0 && ./sort.sh .foo0
# IP: Check listing
0 diff -u -I 'Size in memory.*' .foo hash:ip6.t.list2
# IP: Flush test set
0 ipset -F test
# IP: Try to add multiple elements in one step
1 ipset -A test 1::1-1::10
# IP: Delete test set
0 ipset -X test
# Network: Create a set with timeout
0 ipset -N test iphash -6 --hashsize 128 --netmask 64 timeout 5
# Network: Add zero valued element
1 ipset -A test ::
# Network: Test zero valued element
1 ipset -T test ::
# Network: Delete zero valued element
1 ipset -D test ::
# Network: Add first random network
0 ipset -A test 2:0:0::1
# Network: Add second random network
0 ipset -A test 192:168:68::69
# Network: Test first random value
0 ipset -T test 2:0:0::255
# Network: Test second random value
0 ipset -T test 192:168:68::95
# Network: Test value not added to the set
1 ipset -T test 4:0:1::0
# Network: Add third element
0 ipset -A test 200:100:10::1 timeout 0
# Network: Add third random network
0 ipset -A test 200:101:0::12
# Network: Delete the same network
0 ipset -D test 200:101:0::12
# Network: Test the deleted network
1 ipset -T test 200:101:0::12
# Network: List set
0 ipset -L test | grep -v Revision: | sed 's/timeout ./timeout x/' > .foo0 && ./sort.sh .foo0
# Network: Check listing
0 diff -u -I 'Size in memory.*' .foo hash:ip6.t.list3
# Sleep 5s so that elements can time out
0 sleep 5
# Network: List set
0 ipset -L test | grep -v Revision: > .foo
# Network: Check listing
0 diff -u -I 'Size in memory.*' .foo hash:ip6.t.list1
# Network: Flush test set
0 ipset -F test
# Network: Delete test set
0 ipset -X test
# Check more complex restore commands
0 ipset restore < restore.t.restore
# List restored set a
0 ipset l a | grep -v Revision: > .foo0 && ./sort.sh .foo0
# Check listing of set a
0 diff -u -I 'Size in memory.*' .foo restore.t.list0
# List restored set b
0 ipset l b | grep -v Revision: > .foo0 && ./sort.sh .foo0
# Check listing of set b
0 diff -u -I 'Size in memory.*' .foo restore.t.list1
# Destroy by restore
0 ipset restore < restore.t.destroy
# eof
