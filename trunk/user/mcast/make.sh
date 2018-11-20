#/opt/rt-n56u/toolchain-mipsel/toolchain-3.4.x/bin/mipsel-linux-uclibc-gcc -mips32r2 -march=mips32r2 -Os -fomit-frame-pointer -pipe  -Dlinux -D__linux__ -Dunix -DEMBED -I/home/mao/rt-n56u.tsl.k2/trunk/stage/include -Wall -Wno-trigraphs -Wno-strict-aliasing -Wno-format-security   -Wno-pointer-sign '-DRP_VERSION="3.12"' -DUSE_SINGLE_MAC -c -o mcast.o -fPIC mcast.c
#/opt/rt-n56u/toolchain-mipsel/toolchain-3.4.x/bin/mipsel-linux-uclibc-gcc -mips32r2 -march=mips32r2 -Os -fomit-frame-pointer -pipe  -Dlinux -D__linux__ -Dunix -DEMBED -I/home/mao/rt-n56u.tsl.k2/trunk/stage/include -Wall -Wno-trigraphs -Wno-strict-aliasing -Wno-format-security   -Wno-pointer-sign '-DRP_VERSION="3.12"' -DUSE_SINGLE_MAC -L/home/mao/rt-n56u.tsl.k2/trunk/stage/lib -o mcast mcast.o 

make CC=/opt/rt-n56u/toolchain-mipsel/toolchain-3.4.x/bin/mipsel-linux-uclibc-gcc

ls -la mcast speed

/opt/rt-n56u/toolchain-mipsel/toolchain-3.4.x/bin/mipsel-linux-uclibc-strip mcast
/opt/rt-n56u/toolchain-mipsel/toolchain-3.4.x/bin/mipsel-linux-uclibc-strip speed

ls -la mcast speed
