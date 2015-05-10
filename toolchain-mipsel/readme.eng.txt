* CROSS-TOOLCHAIN BUILD INSTRUCTION *

The cross-toolchain is builded to CPU with arch MIPS32_R2 LE:
- Ralink RT3883/RT3662 (MIPS 74Kc)
- MediaTek MT7620 (MIPS 24KEc)
- MediaTek MT7621 (MIPS 1004Kc)

To build the cross-toolchain, you need Linux environment. Debian 'wheezy' 7.8.0 and
Debian 'jessie' 8.0.0 distros has been tested and recommended.

Just run build script "build_toolchain" and wait for the build process complete.

The "build_toolchain" script is intended to build cross-toolchain for Linux
kernel 3.4.x. Target directory is "toolchain-3.4.x".

The "build_toolchain_3.0.x" script is intended to build cross-toolchain for Linux
kernel 3.0.x. Target directory is "toolchain-3.0.x". Linux kernel 3.0.x used only
for ASUS RT-N65U board (iNIC_mii.ko is pre-compiled as blob w/o source code).


* CROSS-TOOLCHAIN PACKAGES *

binutils-2.24 + upstream patches
gcc-4.4.7 + upstream patches
uClibc-0.9.33.2 + upstream patches


* NOTE *

To build the cross-toolchain under Debian 'wheezy'/'jessie' you need the packages:
- sudo
- build-essential
- gawk
- pkg-config
- gettext
- autoconf
- automake
- libtool
- bison
- flex
- texinfo
- libgmp3-dev
- libmpfr-dev
- libmpc-dev




-
05/10/2015
Padavan
