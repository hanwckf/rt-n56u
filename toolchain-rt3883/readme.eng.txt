* Ralink RT3883/3662 CROSS-TOOLCHAIN BUILD INSTRUCTION *

To build the cross-toolchain, you need Linux environment. Debian squeeze 6.0.7 
and Ubuntu 10.04 distros has been tested.

Just run build script "build_toolchain" and wait for the build process complete.

The "build_toolchain" script is intended to build cross-toolchain for Linux 
kernel 3.0.x. Target directory is "toolchain-3.0.x".

The "build_toolchain_3.4.x" script is intended to build cross-toolchain for Linux 
kernel 3.4.x. Target directory is "toolchain-3.4.x".


* CROSS-TOOLCHAIN PACKAGES *

binutils-2.24 + upstream patches
gcc-4.4.7 + upstream patches
uClibc-0.9.33.2 + upstream patches


* NOTE *

To build the cross-toolchain under Debian squeeze you need the packages:
- build-essential
- gawk
- sudo
- pkg-config
- gettext
- automake
- autoconf
- libtool
- bison
- flex
- texinfo
- libgmp3-dev
- libmpfr-dev
- libmpc-dev




-
03/12/2014
Padavan
