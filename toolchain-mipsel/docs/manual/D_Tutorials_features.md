
This section describes setup quirks, caveats and limitations specific to
particular features in crosstool-NG.

GNU libc locales
----------------

GNU libc does not offer a means to cross-compile locale data for the target system.
As a workaround, crosstool-NG configures the glibc for the build machine and
generates the locales on the build, even though they will be used on the target.

This obviously has caveats:
- The build and the target machines must have the same endianness and the same
  sizes of integer types.
- This approach does not work on the build machines not supported by GNU libc.
  Currently, GNU libc locales are disabled on macOS and Cygwin.

uClibc/uClibc-NG configuration
------------------------------

Before release 1.21.0, it was necessary to supply a configuration file for
uClibc/uClibc-ng library. Current crosstool-NG releases can generate this
configuration file based on the menu selection. You can still supply your
own uClibc configuration file.

Note however that the choices made in crosstool-NG configuration will be
applied on top of the uClibc configuration file, to ensure the compatibility
of the uClibc with the target components built afterwards. This also allows
the user to share the same base configuration file for uClibc and tweak it
for different targets using crosstool-NG configuration.

uClibc with newer GCC
---------------------

uClibc (not uClibc-ng) produces invalid binaries when compiled with GCC5
or newer (noticed at least on the i686 architecture); applications
segfault while starting up when run against the resulting libraries.
Use GCC 4.9 or 4.8 when compiling a uClibc based toolchain.

Given that uClibc is essentially unmaintained, this is unlikely to get
ever fixed.

Multilib caveats
----------------

Crosstool-NG has experimental support for building multilib toolchain.
There are a few caveats though:
- Buildroot does not accept the toolchains if the produced libraries are
  scattered across separate subdirectories. For example, on AArch64 the
  GNU libc installs its dynamic linker into `/lib`, while the rest of the
  dynamic libraries are installed into `/lib64`. Crosstool-NG offers a
  configuration option to combine the libraries into a single library
  directory; it is turned on by default for non-multilib builds. For
  multilib builds, it is not recommended to turn it on unless your multilib
  configuration uses separate sysroots for all variants. At this time, only
  the SuperH architecture is known to do that.
- Starting with version 2016.02, Buildroot is rejecting the toolchains
  with `/etc/ld.so.conf` present in sysroot. Generation of this file is
  optional in crosstool-NG; however, without this file the cross-ldd
  helper script will not be able to find the library dependencies residing
  outside of the default `/lib` and `/usr/lib` directories. A fix for this
  is planned.
- On x86, current GNU libc versions share the headers between different
  multilib variants. Older libc versions had conflicting headers, in
  particular `<ucontext.h>`. If you see compilation errors referring
  to wrong registers for selected multilib variant (e.g., `%rip` with
  `-m32` flag), the selected version of GNU libc is too old and does not
  support multilib.
- Certain architectures have a fixed set of multilibs in GCC. As a result,
  if not all of them are supported by the selected C library (glibc, uclibc),
  the build will fail. This is not a problem with crosstool-NG.

Using crosstool-NG to build Alpha toolchains
--------------------------------------------

The `alphaev67-unknown-linux-gnu` sample produces errors related to the
`.eh_frame_hdr` section in the C runtime files (`crt*.o`). The toolchain
compiles fine, but may have issues with the generated binaries. If you use
this toolchain and encounter any issues, please let us know.

Python scripting in cross GDB
-----------------------------

Crosstool-NG offers an option to enable Python scripting in the cross-GDB
for the host. This requires the Python headers and libraries for the host
to be available. Usually, these come from a `python-dev` or a similarly
named package.

For canadian (and hence, for cross-native) toolchains, this configuration
option will result in build failure, unless special steps are taken to
place the cross-compiled Python libraries and headers where the compiler
for the host will be able to find them. Otherwise, the build will fail
as well.

Using crosstool-NG to build Xtensa toolchains
---------------------------------------------

*Contributed by: Max Filippov*

Xtensa cores are highly configurable: endianness, instruction set, register set
of a core is chosen at processor configuration time. New registers and
instructions may be added by designers, making each core configuration unique.
Toolchain components cannot know about features of each individual core and
need to be configured in order to be compatible with particular architecture
variant. This configuration includes:
- definitions of instruction formats, names and properties for assembler,
  disassembler and debugger;
- definitions of register names and properties for assembler, disassembler and
  debugger;
- selection of predefined features, such as endianness, presence of certain
  processor options or instructions for compiler, debugger C library and OS
  kernels;
- macros with instruction sequences for saving and restoring special, user or
  coprocessor registers for OS kernels.

This configuration is provided in form of source files, that must replace
corresponding files in binutils, gcc, gdb or newlib source trees or be added
to OS kernel source tree. This set of files is usually distributed as archive
known as Xtensa configuration overlay.

Tensilica provides such an overlay as part of the processor download, however,
it needs to be reformatted to match the specific format required by the
crosstool-NG. For a script to convert the overlay file, and additional
information, please see [this link](http://wiki.linux-xtensa.org/index.php/Toolchain_Overlay_File)

The current version of crosstool-NG requires that the overlay file name has the
format xtensa_<CORE_NAME>.tar, where CORE_NAME can be any user selected name.
To make crosstool-NG use overlay file located at `<PATH>/xtensa_<CORE_NAME>.tar`
select XTENSA_CUSTOM, set config parameter `CT_ARCH_XTENSA_CUSTOM_NAME` to
`CORE_NAME` and `CT_ARCH_XTENSA_CUSTOM_OVERLAY_LOCATION` to `PATH`.

The fsf target architecture variant is the configuration provided by toolchain
components by default. It is present only for build-testing toolchain
components and is in no way special or universal.
