* Ralink RT3883/3662 CROSS-TOOLCHAIN BUILD INSTRUCTION *

To build the cross-toolchain, you need Linux environment. Debian squeeze 6.0.3 
and Ubuntu 10.04 distros has been tested.

Just run build script "build_toolchain" or "build_toolchain_new" and wait for 
the build process complete.

The "build_toolchain" script is intended to build cross-toolchain for Linux 
kernel 2.6.21.x. Target directory is "toolchain-2.6.21.x".

The "build_toolchain_new" script is intended to build cross-toolchain for Linux 
kernel 3.0.x. Target directory is "toolchain-3.0.x".


* CROSS-TOOLCHAIN PACKAGES *

binutils-2.21.1 + upstream patches
gcc-4.5.4 + upstream patches
uClibc-0.9.28.3 + upstream patches (for Linux kernel 2.6.21.x)
uClibc-0.9.33.2 + upstream patches (for Linux kernel 3.0.x)


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
- libgmp3-dev
- libmpfr-dev
- libmpc-dev




-
09/18/2012
Padavan
