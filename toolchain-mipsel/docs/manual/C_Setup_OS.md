
This section describes the setup needed by various operating systems
in order to run crosstool-NG, as well as some OS-specific caveats and limitations.
The package lists given in the following subsections cover all the features tested
by the sample configurations. You particular configuration may not need all those
packages. For example, `git` is needed if your configuration is for an uClinux-based
target which requires `elf2flt` utilities (which does "rolling releases" and must
be checked out from a Git repository).

Linux
-----

Sample configurations for supported Linux distributions are available as Docker
configuration files in the `testing` directory. You can use the contents of
these files as a list of the packages that need to be installed on a particular
distribution.

If on the other hand you encounter a dependency not listed there, please let us
know over the mailing list or via a pull request!

Windows: Cygwin
---------------

*Originally contributed by: Ray Donnelly*

Prerequisites and instructions for using crosstool-NG for building a cross
toolchain on Windows (Cygwin) as build and, optionally Windows (hereafter)
MinGW-w64 as host.

1. Use Cygwin64 if you can. DLL base-address problems are lessened that
   way and if you bought a 64-bit CPU, you may as well use it.

2. You must enable Case Sensitivity in the Windows Kernel (this is only really
   necessary for Linux targets, but at present, crosstool-ng refuses to operate
   on case insensitive filesystems). The registry key for this is:
   `HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\kernel\obcaseinsensitive`
   Read more [here](https://cygwin.com/cygwin-ug-net/using-specialnames.html).

   Since release 1.7, Cygwin no longer supports the 'managed' mount option.
   You must use case sensitive FS.

3. Using `setup.exe` or `setup-x86_64.exe`, install the default packages and also
   the following ones:
<!-- TBD in a clean win install, check which ones are selected by default -->
   - autoconf
   - automake
   - bison
   - diffutils
   - flex
   - gawk
   - gcc-g++
   - git
   - gperf
   - help2man
   - libncurses-devel
   - make
   - patch
   - python-devel
   - texinfo
   - wget
   - xz

   Leave "Select required packages (RECOMMENDED)" ticked.

   Notes:
   - The packages marked with * are only needed if your host is MinGW-w64.

4. Although nativestrict symlinks seem like the best idea, extracting glibc fails
   when they are enabled, so just don't set anything here. If your host is MinGW-w64
   then these 'Cygwin-special' symlinks won't work, but you can dereference them by
   using tar options --dereference and --hard-dereference when making a final tarball.
   I plan to investigate and fix or at least work around the extraction problem.
   Read more [here](https://cygwin.com/cygwin-ug-net/using-cygwinenv.html).

5. If both BFD and GOLD linkers are enabled in binutils, `collect2.exe` will attempt
   to run `ld` which is a shell script so you need to make sure that a working shell
   is in your path.  Eventually this will be replaced with a native program for
   MinGW-w64 host.

**Notes**

1. Cygwin is slow. No, really, really slow. Expect about approximately 5x to 10x slowdown
   compared to a Linux system.

FreeBSD (and other BSD)
-----------------------

FreeBSD support is currently experimental in crosstool-NG.

FreeBSD does not provide a `gcc` command by default. Crosstool-NG and many of the packages
used expect this by default. A comprehensive fix for various ways of setting up the OS
is planned after the 1.23 release. Until then, setting up the following packages is
recommended as a prerequisite for crosstool-NG:

- `archivers/zip`
- `devel/automake`
- `devel/bison`
- `devel/gettext-tools`
- `devel/git`
- `devel/gmake`
- `devel/gperf`
- `devel/libatomic_ops`
- `devel/libtool`
- `devel/patch`
- `lang/gcc6`
- `lang/gawk`
- `misc/help2man`
- `print/texinfo`
- `textproc/asciidoc`
- `textproc/gsed`
- `textproc/xmlto`

Use any supported method of installation, e.g.:
````
cd /usr/ports/lang/gcc6
make install clean
````

Even with these packages installed, some of the samples are failing to build. YMMV.

> **Previous version of the installation guidelines**
>
> *Contributed by: Titus von Boxberg*
>
> Prerequisites and instructions for using ct-ng for building a cross toolchain on FreeBSD as host.
>
> 0. Tested on FreeBSD 8.0
>
> 1. Install (at least) the following ports
>    archivers/lzma
>    textproc/gsed
>    devel/gmake
>    devel/patch
>    shells/bash
>    devel/bison
>    lang/gawk
>    devel/automake110
>    ftp/wget
>
>    Of course, you should have /usr/local/bin in your PATH.
>
> 2. run ct-ng's configure with the following tool configuration:
> ````
> ./configure --with-sed=/usr/local/bin/gsed \
>       --with-make=/usr/local/bin/gmake \
>       --with-patch=/usr/local/bin/gpatch
>    [...other configure parameters as you like...]
> ````
>
> 3. proceed as described in general documentation
>   but use gmake instead of make


macOS (a.k.a Mac OS X, OS X)
----------------------------

**Note** macOS is no longer officially supported by crosstool-NG. If the instructions
below work for you, congratulations. If they kill your cat, ye be warned.

*Originally contributed by: Titus von Boxberg*

The instructions below have been verified on macOS Sierra (10.12). They have been previously
reported to work with versions since Mac OS X Snow Leopard (10.6) with Developer Tools 3.2,
and with Mac OS X Leopard (10.5) with Developer Tools + GCC 4.3 or newer installed via MacPorts.

1. You have to use a case sensitive file system for crosstool-NG's build and target
   directories. Use a disk or disk image with a case sensitive FS that you
   mount somewhere.

2. Install required tools via HomeBrew. The following set is sufficient for
   HomeBrew: `autoconf binutils gawk gmp gnu-sed help2man mpfr openssl pcre readline wget xz`.
   Install them using `brew install PACKAGE` command.

   Also, installing `homebrew/dupes/grep` is recommended. It has been noticed that GNU libc
   was misconfigured due to a subtle difference between BSD grep (which is used by macOS) and
   GNU grep. This has since been fixed, but other scripts in various packages may still contain
   GNUisms.

   If you prefer to use MacPorts, refer to the previous version of the instruction below
   and let us know if it works with current crosstool-NG and macOS releases.

3. Mac OS X defaults to a fairly low limit on the number of the files that can be opened by
   a process (256) that is exceeded by the build framework of the GNU C library. Increase this
   limit to 1024:
   ````
ulimit -n 1024
   ````

**Notes:**

1. When building on macOS, the following message may be displayed:

   ````
clang: error: unsupported option '-print-multi-os-directory'
clang: error: no input files
   ````

   It is reported when the host version of `libiberty` (from GCC) is compiled by macOS
   default compiler, `clang`. In absense of any reported multilib information, `libiberty`
   is then configured with the default compilation flags. This does not seem to affect
   the resulting toolchain.

2. `ct-ng menuconfig` will not work on Snow Leopard 10.6.3 since libncurses
   is broken with this release. MacOS <= 10.6.2 and >= 10.6.4 are ok.

3. APFS filesystem is known to have some random issues with parallel build of GCC.
   See [this bug report](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=81797) for
   details. Don't use APFS, or turn off the parallel build in crosstool-NG (setting
   the number of parallel jobs to 1 in the confiuguration) as a workaround.

> **Previous version of the installation guidelines**
>
> Crosstool-NG has been reported to work with MacPorts as well, using the following set
> of ports: `lzmautils libtool binutils gsed gawk`. On Mac OS X Leopard, it is also required
> to install `gcc43` and `gcc_select`.
>
> On Leopard, make sure that the MacPort's `gcc` is called with the default commands
> (`gcc`, `g++`,...), via MacPort's `gcc_select`.
>
> On OSX 10.7 Lion / when using Xcode >= 4 make sure that the default commands
> `gcc`, `g++`, etc.) point to `gcc-4.2`, NOT `llvm-gcc-4.2`
> by using MacPort's `gcc_select` feature. With MacPorts >= 1.9.2
> the command is: "sudo port select --set gcc gcc42"
>
> This also requires (like written above) that macport's `bin` directory
> comes before the standard directories in your `PATH` environment variable
> because the `gcc` symlink is installed in `/opt/local/bin` and the default `/usr/bin/gcc`
> is not removed by the `gcc_select` command!
>
> Explanation: `llvm-gcc-4.2` (with Xcode 4.1 it is on my machine
> "gcc version 4.2.1 (Based on Apple Inc. build 5658) (LLVM build 2335.15.00)")
> cannot boostrap gcc. See [this bug](http://llvm.org/bugs/show_bug.cgi?id=9571)
>
> Apparently, GNU make's builtin variable `.LIBPATTERNS` is misconfigured
> under MacOS: It does not include `lib%.dylib`.
> This affects build of (at least) GDB 7.1
> Put `lib%.a lib%.so lib%.dylib` as `.LIBPATTERNS` into your environment
> before executing `ct-ng build`.
> See [here](http://www.gnu.org/software/make/manual/html_node/Libraries_002fSearch.html)
> for details.
>
> Note however, that GDB 7.1 (and anything earlier than 7.10) are known
> to fail to build on macOS.


