* BUILD INSTRUCTION *

1) You need Linux environment to build the firmware. Debian 'wheezy' 7.8.0 and
   Debian 'jessie' 8.4 distros has been tested and recommended.
2) Build cross-toolchain for MIPS32_R2 CPU (binutils-2.24, gcc-447, uclibc-0.9.33.2)
   from external package toolchain-mipsel.
3) Configure firmware content via .config file. Use comment mark # for
   disable features. Change "CONFIG_TOOLCHAIN_DIR=" param to target cross-toolchain
   directory.
4) Build firmware via "build_firmware" script. After the build is finished,
   the firmware file (*.trx) will be placed to directory "images".


* NOTE *

You need following packages to build the firmware under Debian 'wheezy'/'jessie':
- autoconf
- automake
- bison
- build-essential
- flex
- gawk
- gettext
- gperf
- libtool
- pkg-config
- sudo
- zlib1g-dev

You need additional packages to build cross-toolchain:
- libgmp3-dev
- libmpc-dev
- libmpfr-dev
- texinfo




-
05/10/2015
Padavan
