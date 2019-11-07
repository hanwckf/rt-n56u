
Internally, crosstool-NG is script-based. To ease usage, the frontend is
Makefile-based.


Makefile front-end
------------------

The entry point to crosstool-NG is the Makefile script `ct-ng`. Calling
this script with an action will act exactly as if the Makefile was in
the current working directory and make was called with the action as
rule. Thus:

    ct-ng menuconfig

is equivalent to having the Makefile in `CWD`, and calling:

    make menuconfig

Having ct-ng as it is avoids copying the Makefile everywhere, and acts
as a traditional command.

ct-ng loads sub- Makefiles from the library directory `$(CT_LIB_DIR)`,
as set up at configuration time with `./configure`.

ct-ng also searches for config files, sub-tools, samples, scripts and
patches in that library directory.

Because of a stupid make behavior/bug I was unable to track down,
implicit make rules are disabled: installing with `--local` would
trigger those rules, and mconf was unbuildable.


Kconfig parser
--------------

The kconfig language is a hacked version, vampirised from the [Linux
kernel](http://www.kernel.org/), and (heavily) adapted to my needs.

The list of the most notable changes (at least the ones I remember)
follows:

-   the `CONFIG_` prefix has been replaced with `CT_`

-   a leading `|` in prompts is skipped, and subsequent leading spaces
    are not trimmed; otherwise leading spaces are silently trimmed

-   removed the warning about undefined environment variable

The kconfig parsers (conf and mconf) are not installed pre-built, but as
source files. Thus you can have the directory where crosstool-NG is
installed, exported (via NFS or whatever) and have clients with
different architectures use the same crosstool-NG installation, and most
notably, the same set of patches.


Architecture-specific
---------------------

> **Note**
>
> this chapter is not really well written, and might thus be a little
> bit complex to understand. To get a better grasp of what an
> architecture is, the reader is kindly encouraged to look at the
> `arch/` sub-directory, and to the existing architectures to see how
> things are laid out.

### An architecture is defined by:

-   a human-readable name, in lower case letters, with numbers as
    appropriate; underscore is allowed; space and special characters are
    not, e.g.:

        arm
        x86_64

-   a file in `config/arch/`, named after the architecture’s name, and
    suffixed with `.in`, e.g.:

        config/arch/arm.in

-   a file in `scripts/build/arch/`, named after the architecture’s
    name, and suffixed with `.sh`, e.g.:

        scripts/build/arch/arm.sh

### The architecture’s `.in` file API

> **Note**
>
> Here, and in the following, `%arch%` is to be replaced with the actual
> architecture name.

#### The `ARCH_%arch%` option

This config option must have **neither** a type, **nor** a prompt! Also,
it can **not** depend on any other config option.

`EXPERIMENTAL` is managed as in [kernel options](#kernel-in) ?.

A (terse) help entry **must** be defined for this architecture, e.g.,

    config ARCH_arm
      help
        The ARM architecture.

Adequate associated config options may be selected, e.g.,

    config ARCH_arm
      select ARCH_SUPPORTS_BOTH_ENDIAN
      select ARCH_DEFAULT_LE
      help
        The ARM architecture.

> **Note**
>
> 64-bit architectures **shall** select `ARCH_64`.

    config ARCH_x86_64
       select ARCH_64
       help
         The x86_64 architecture.

#### Other target-specific options

At your discretion. Note however that to avoid name-clashing, such
options shall be prefixed with `ARCH_%arch%`.

> **Note**
>
> Due to historical reasons, and lack of time to clean up the code, I
> may have left some config options that do not completely conform to
> this, as the architecture name was written all upper case. However,
> the prefix is unique among architectures, and does not cause harm).

### The architecture’s `.sh` file API

-   the function `CT_DoArchTupleValues`

    -   parameters: none

    -   environment:

        -   all variables from the `.config` file,

        -   the two variables `target_endian_eb` and `target_endian_el`
            which are the endianness suffixes

    -   return value: `0` upon success, `!0` upon failure

    -   provides:

        -   environment variable `CT_TARGET_ARCH` (**mandatory**)

            -   contains: the architecture part of the target tuple,
                e.g. `armeb` for big endian ARM, or `i386` for an i386

        -   environment variable `CT_TARGET_SYS` (optional)

            -   contains: the system part of the target tuple, e.g.,
                `gnu` for glibc on most architectures, or `gnueabi` for
                glibc on an ARM EABI

            -   defaults to:

                -   `gnu` for glibc-based toolchain

                -   `uclibc` for uClibc-based toolchain

        -   environment variables to configure the cross-gcc (defaults)
            (optional)

            | gcc ./configure switch   | selects                  | default                        |
            |--------------------------|--------------------------|--------------------------------|
            | `CT_ARCH_WITH_ARCH`      | architecture level       | `--with-arch=${CT_ARCH_ARCH}`  |
            | `CT_ARCH_WITH_ABI`       | ABI level                | `--with-abi=${CT_ARCH_ABI}`    |
            | `CT_ARCH_WITH_CPU`       | CPU instruction set      | `--with-cpu=${CT_ARCH_CPU}`    |
            | `CT_ARCH_WITH_TUNE`      | scheduling               | `--with-tune=${CT_ARCH_TUNE}`  |
            | `CT_ARCH_WITH_FPU`       | FPU type                 | `--with-fpu=${CT_ARCH_FPU}`    |
            | `CT_ARCH_WITH_FLOAT`     | floating point arithm.   | `--with-float=soft` or [empty] |

        -   environment variables to pass to the cross-gcc to build
            target binaries (defaults) (optional)

            | gcc ./configure switch   | selects                  | default                             |
            |--------------------------|--------------------------|-------------------------------------|
            | `CT_ARCH_ARCH_CFLAG`     | architecture level       | `-march=${CT_ARCH_ARCH}`            |
            | `CT_ARCH_ABI_CFLAG`      | ABI level                | `-mabi=${CT_ARCH_ABI}`              |
            | `CT_ARCH_CPU_CFLAG`      | CPU instruction set      | `-mcpu=${CT_ARCH_CPU}`              |
            | `CT_ARCH_TUNE_CFLAG`     | scheduling               | `-mtune=${CT_ARCH_TUNE}`            |
            | `CT_ARCH_FPU_CFLAG`      | FPU type                 | `-mfpu=${CT_ARCH_FPU}`              |
            | `CT_ARCH_FLOAT_CFLAG`    | floating point arithm.   | `-msoft-float` or [empty]           |
            | `CT_ARCH_ENDIAN_CFLAG`   | big or little endian     | `-mbig-endian` or `-mlittle-endian` |

        -   the environment variables to configure the core and final
            compiler, specific to this architecture (optional):

            -   `CT_ARCH_CC_CORE_EXTRA_CONFIG`: additional, architecture
                specific core `gcc ./configure` flags

            -   `CT_ARCH_CC_EXTRA_CONFIG`: additional, architecture
                specific final `gcc ./configure` flags

            -   default to: all empty

        -   the architecture-specific `CFLAGS` and `LDFLAGS` (optional):

            -   `CT_ARCH_TARGET_CLFAGS`

            -   `CT_ARCH_TARGET_LDFLAGS`

            -   default to: all empty

You can have a look at `config/arch/arm.in` and `scripts/build/arch/arm.sh`
for a quite complete example of what an actual architecture description
looks like.


Kernel specific <a name="kernel-in"></a>
---------------

### A kernel is defined by:

-   a human-readable name, in lower case letters, with numbers as
    appropriate; underscore is allowed, space and special characters are
    not (although they are internally replaced with underscores); e.g.:

        linux
        bare-metal

-   a file in `config/kernel/`, named after the kernel name, and
    suffixed with `.in`, e.g.:

        config/kernel/linux.in
        config/kernel/bare-metal.in

-   a file in `scripts/build/kernel/`, named after the kernel name, and
    suffixed with `.sh`, e.g.:

        scripts/build/kernel/linux.sh
        scripts/build/kernel/bare-metal.sh

### The kernel’s `.in` file must contain:

-   an optional line containing exactly `# EXPERIMENTAL`, starting on
    the first column, and without any following space or other
    character.

    If this line is present, then this kernel is considered
    `EXPERIMENTAL`, and correct dependency on `EXPERIMENTAL` will be
    set.

-   the config option `KERNEL_%kernel_name%` (where `%kernel_name%` is
    to be replaced with the actual kernel name, with all special
    characters and spaces replaced by underscores), e.g.:

        KERNEL_linux
        KERNEL_bare_metal

    This config option must have **neither** a type, **nor** a prompt!
    Also, it can **not** depends on `EXPERIMENTAL`.

    A (terse) help entry for this kernel **must** be defined, e.g.:

        config KERNEL_bare_metal
          help
            Build a compiler for use without any kernel.

    Adequate associated config options may be selected, e.g.:

        config KERNEL_bare_metal
          select BARE_METAL
          help
            Build a compiler for use without any kernel.

-   other kernel specific options, at your discretion. Note however
    that, to avoid name-clashing, such options should be prefixed with
    `KERNEL_%kernel_name%`, where `%kernel_name%` is again to be
    replaced with the actual kernel name.

> **Note**
>
> Due to historical reasons, and lack of time to clean up the code, Yann 
> may have left some config options that do not completely conform to
> this, as the kernel name was written all upper case. However, the
> prefix is unique among kernels, and does not cause harm).

### The kernel’s `.sh` file API:

-   is a bash script fragment

-   defines function `CT_DoKernelTupleValues`

    -   see the architecture’s `CT_DoArchTupleValues`, except for:
        [FIXME?]

    -   set the environment variable `CT_TARGET_KERNEL`, the kernel part
        of the target tuple

    -   return value: ignored

-   defines function `do_kernel_get`:

    -   parameters: none

    -   environment:

        -   all variables from the `.config` file.

    -   return value: `0` for success, `!0` for failure.

    -   behavior: download the kernel’s sources, and store the tarball
        into `${CT_TARBALLS_DIR}`. To this end, a function is available
        that abstracts downloading tarballs:

        -   `CT_DoGet <tarball_base_name> <URL1 [URL...]>`, e.g.:

                CT_DoGet linux-2.6.26.5 ftp://ftp.kernel.org/pub/linux/kernel/v2.6

            > **Note**
            >
            > Retrieving sources from SVN, CVS, git and the likes is not
            > supported by `CT_DoGet`. You’ll have to do this by hand,
            > as it is done for eglibc in
            > `scripts/build/libc/eglibc.sh`.

-   defines function `do_kernel_extract`:

    -   parameters: none

    -   environment:

        -   all variables from the `.config` file,

    -   return value: `0` for success, `!0` for failure.

    -   behavior: extract the kernel’s tarball into `${CT_SRC_DIR}`, and
        apply required patches. To this end, a function is available
        that abstracts extracting tarballs:

        -   `CT_ExtractAndPatch <tarball_base_name>`, e.g.:

                CT_ExtractAndPatch linux-2.6.26.5

-   defines function `do_kernel_headers`:

    -   parameters: none

    -   environment:

        -   all variables from the `.config` file,

    -   return value: `0` for success, `!0` for failure.

    -   behavior: install the kernel headers (if any) in
        `${CT_SYSROOT_DIR}/usr/include`

-   defines any kernel-specific helper functions

    These functions, if any, must be prefixed with
    `do_kernel_%CT_KERNEL%_`, where `%CT_KERNEL%` is to be replaced with
    the actual kernel name, to avoid any name-clashing.

You can have a look at `config/kernel/linux.in` and
`scripts/build/kernel/linux.sh` as an example of what a complex kernel
description looks like.


Adding a new version of a component
-----------------------------------

When a new component, such as the Linux kernel, gcc or any other is
released, adding the new version to crosstool-NG is quite easy. There is
a script that will do all that for you:

    scripts/addToolVersion.sh

Run it with no option to get some help.


Build scripts
-------------

[To Be Written later…]
