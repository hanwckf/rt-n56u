#!/bin/sh

rm -rf ../linux-2.6.21.x/drivers/net/wireless/ralink

cp -rf rt3883-asus/linux-2.6.21.x ..
cp -rf rt3883-wifi/linux-2.6.21.x ..

for i in ???-*.patch ; do
	[ -f "$i" ] && echo "*** patch $i ***" && patch -d .. -p0 < "$i"
done

for i in ????-*.patch ; do
	[ -f "$i" ] && echo "*** patch $i ***" && patch -d .. -p1 < "$i"
done

