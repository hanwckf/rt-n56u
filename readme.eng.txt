* BUILD INSTRUCTION *

1) To build the firmware, you need Linux environment. Debian 'wheezy' 7.8.0 and
   Debian 'jessie' 8.0.0 distros has been tested and recommended.
2) Build cross-toolchain for MIPS32_R2 CPU (binutils-2.24, gcc-447, uclibc-0.9.33.2)
   from external package toolchain-mipsel.
3) Manual configure firmware content via .config file. Use comment mark # for
   disable features. Change param "CONFIG_TOOLCHAIN_DIR=" to target cross-toolchain
   directory.
4) Build firmware via "build_firmware" script. After the build is finished,
   the firmware file (*.trx) will be placed to directory "images".


* NOTE *

To build the firmware under Debian 'wheezy'/'jessie' you need the packages:
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
- zlib1g-dev

To build cross-toolchain, you need additional packages:
- texinfo
- libgmp3-dev
- libmpfr-dev
- libmpc-dev




-
05/10/2015
Padavan
