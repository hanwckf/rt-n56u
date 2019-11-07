
Using the toolchain is as simple as adding the toolchain’s bin directory in
your `PATH`, such as:

    export PATH="${PATH}:/your/toolchain/path/bin"

Depending on the project being compiled, there may be different ways
to specify the toolchain.

If the software uses GNU autotools or a similar configure script, you shoul
use the `--host` tuple to tell the build system to use your toolchain
(if the software package uses the autotools system you should also
pass `--build`, for completeness):

    ./configure --host=your-host-tuple --build=your-build-tuple

Other systems may need, for example:

    make CC=your-host-tuple-gcc
    make CROSS_COMPILE=your-host-tuple-
    make CHOST=your-host-tuple

and so on. Please read the documentation of the package being compiled
as to how it can be cross-compiled.

> **Note**
>
> in the above example, `host` refers to the host of your program (i.e.
> the target of the toolchain), not the host of the toolchain; and
> `build` refers to the machine where you build your program, that
> is the host of the toolchain.

Assembling a root filesystem
----------------------------

Assembling a root filesystem for a target device requires the successive
building of a set of software packages for the target architecture.
Building a package potentially requires artifacts which were generated as
part of an earlier build. Note that not all artifacts which are installed as
part of a package are desirable on a target’s root filesystem (e.g.,
man/info files, include files, etc.).
Therefore we must distinguish between a `staging` directory and a `rootfs`
directory.

A `staging` directory is a location into which we install all the build
artifacts. We can then point future builds to this location so they can
find the appropriate header and library files. A `rootfs` directory is a
location into which we place only the files we want to have on our target.

There are four schools of thought here:

 -  **Option 1**: Install directly into the sysroot of the toolchain.

    By default (i.e., if you don’t pass any arguments to the tools which
    would change this behaviour) the toolchain that is built by
    crosstool-NG will only look in its toolchain directories for system
    header and library files:

        #include "..." search starts here:
        #include <...> search starts here:
        <ct-ng install path>/lib/gcc/<host tuple>/4.5.2/include
        <ct-ng install path>/lib/gcc/<host tuple>/4.5.2/include-fixed
        <ct-ng install path>/lib/gcc/<host tuple>/4.5.2/../../../../<host tuple>/include
        <ct-ng install path>/<host tuple>/sysroot/usr/include

    In other words, the compiler will automagically find headers and
    libraries without extra flags if they are installed under the
    toolchain’s sysroot directory.

    However, this is bad because the toolchain gets poluted, and can not
    be re-used.

        $ ./configure --build=<build tuple> --host=<host tuple> \
              --prefix=/usr --enable-foo-bar...
        $ make
        $ make DESTDIR=/<ct-ng install path>/<host tuple>/sysroot install

 -  **Option 2**: Copy the toolchain’s sysroot to the `staging` area.

    If you start off by copying the toolchain’s sysroot directory to
    your staging area, you can simply proceed to install all your
    packages’ artifacts to the same staging area. You then only need to
    specify a `--sysroot=<staging area>` option to the compiler of any
    subsequent builds and all your required header and library files
    will be found/used.

    This is a viable option, but requires the user to always specify
    `CFLAGS` in order to include `--sysroot=<staging area>`, or requires
    the use of a wrapper to a few select tools (gcc, ld…) to pass this
    flag.

    Instead of polluting the toolchain’s sysroot you are copying its
    contents to a new location and polluting the contents in that new
    location. By specifying the `--sysroot` option you’re effectively
    abandoning the default sysroot in favour of your own.

    Incidentally this is what buildroot does using a wrapper, when using
    an external toolchain.

        $ cp -a $(<host tuple>-gcc --your-cflags-except-sysroot -print-sysroot) \
              /path/to/staging
        $ ./configure --build=<build tuple> --host=<host tuple>          \
                      --prefix=/usr --enable-foo-bar...                  \
                      CC="<host tuple>-gcc --syroot=/path/to/staging"    \
                      CXX="<host tuple>-g++ --sysroot=/path/to/staging"  \
                      LD="<host tuple>-ld --sysroot=/path/to/staging"    \
                      AND_SO_ON="tuple-andsoon --sysroot=/path/to/staging"
        $ make
        $ make DESTDIR=/path/to/staging install

 -  **Option 3**: Use separate staging and sysroot directories.

    In this scenario you use a staging area to install programs, but you do
    not pre-fill that staging area with the toolchain’s sysroot. In this case
    the compiler will find the system includes and libraries in its sysroot
    area but you have to pass appropriate `CPPFLAGS` and `LDFLAGS` to tell it
    where to find your headers and libraries from your staging area (or use
    a wrapper).

        $ ./configure --build=<build tuple> --host=<host tuple>          \
                      --prefix=/usr --enable-foo-bar...                  \
                      CPPFLAGS="-I/path/to/staging/usr/include"          \
                      LDFLAGS="-L/path/to/staging/lib -L/path/to/staging/usr/lib"
        $ make
        $ make DESTDIR=/path/to/staging install

 -  **Option 4**: A mix of options 2 and 3, using carefully crafted union mounts.

    The staging area is a union mount of:

    -   the sysroot as a read-only branch

    -   the real staging area as a read-write branch

        This also requires passing `--sysroot` to point to the union
        mount, but has other advantages, such as allowing per-package
        staging, and a few more obscure pros. It also has its
        disadvantages, as it potentially requires non-root users to
        create union mounts. Additionally, union mounts are not yet
        mainstream in the Linux kernel, so it requires patching. There
        is a FUSE-based unionfs implementation, but development is
        almost stalled, and there are a few gotchas.

            $ (good luck!)

It is strongly advised not to use the toolchain sysroot directory as an
install directory (i.e., option 1) for your programs/packages. If you do
so, you will not be able to use your toolchain for another project. It
is even strongly advised that your toolchain is chmod-ed to read-only
once successfully install, so that you don’t go polluting your toolchain
with your programs’/packages’ files. This can be achieved by selecting
the "Render the toolchain read-only" from crosstool-NG’s "Paths and misc
options" configuration page.

Thus, when you build a program/package, install it in a separate,
staging, directory and let the cross-toolchain continue to use its own,
pristine, sysroot directory.

When you are done building and want to assemble your rootfs you could
simply take the full contents of your staging directory and use the
`populate` script to add in the necessary files from the sysroot.
However, the staging area you have created will include lots of build
artifacts that you won’t necessarily want/need on your target. For
example: static libraries, header files, linking helper files, man/info
pages. You’ll also need to add various configuration files, scripts, and
directories to the rootfs so it will boot.

Therefore you’ll probably end up creating a separate rootfs directory
which you will populate from the staging area, necessary extras, and
then use crosstool-NG’s populate script to add the necessary sysroot
libraries.


The `populate` script
---------------------

When your root directory is ready, it is still missing some important
bits: the toolchain’s libraries. To populate your root directory with
those libs, just run:

    your-target-tuple-populate -s /your/root -d /your/root-populated

This will copy `/your/root` into `/your/root-populated`, and put the
needed and only the needed libraries there. Thus you don’t pollute
/your/root with any cruft that would no longer be needed should you have
to remove stuff. `/your/root` always contains only those things you
install in it.

You can then use `/your/root-populated` to build up your file system
image, a tarball, or to NFS-mount it from your target, or whatever you
need.

The populate script accepts the following options:

-   `-s src_dir`: Use `src_dir` as the un-populated root directory.

-   `-d dst_dir`: Put the populated root directory in `dst_dir`.

-   `-l lib1 [...]`: Always add specified libraries.

-   `-L file`: Always add libraries listed in `file`.

-   `-f`: Remove `dst_dir` if it previously existed; continue even if
    any library specified with `-l` or `-L` is missing.

-   `-v`: Be verbose, and tell what’s going on (you can see exactly
    where libs are coming from).

-   `-h`: Print the help.

See `your-target-tuple-populate -h` for more information on the options.

Here is how populate works:

1.  Perform some sanity checks:
    -   `src_dir` and `dst_dir` are specified,
    -   `src_dir` exists,
    -   unless forced, `dst_dir` does not exist,
    -   `src_dir` != `dst_dir`.

2.  Copy `src_dir` to `dst_dir`.

3.  Add forced libraries to `dst_dir`:
    -   build the list from `-l` and `-L` options,
    -   get forced libraries from the sysroot (see below for heuristics)
        -   abort on the first missing library, unless `-f` is specified.

4.  Add all missing libraries to `dst_dir`:
    -   scan `dst_dir` for every ELF files that are *executable* or
        *shared object*
    -   list the "NEEDED Shared library" fields
        -   check if the library is already in `dst_dir/lib` or
            `dst_dir/usr/lib`
        -   if not, get the library from the sysroot
            -   if it’s in `sysroot/lib`, copy it to `dst_dir/lib`
            -   if it’s in `sysroot/usr/lib`, copy it to
                `dst_dir/usr/lib`
            -   in both cases, use the `SONAME` of the library to create
                the file in `dst_dir`
            -   if it was not found in the sysroot, this is an error.


The cross-ldd script
--------------------

There is another script provided by crosstool-NG to work with the sysroot
on the host. A dynamically linked application will load certain shared
libraries at runtime. These libraries in turn may require some more shared
libraries as dependencies. The search paths for each of these dynamic
objects differ, and finding the shared libraries required for a given
application is not always trivial. Crosstool-NG attempts to solve this
task by providing a `${CT_TARGET}-ldd` script in the generated toolchain
(optionally, if the "Install a cross ldd-like helper" option is selected
in the configuration).

This script recursively resolves all the dynamic library dependencies
and outputs the list of libraries in a format compatible with that of
GNU libc's `ldd` script. It needs to have the system root specified,
using either the `--root` option or the `CT_XLDD_ROOT` environment variable.

For example:

    PATH=~/x-tools/powerpc64-multilib-linux-gnu/bin:$PATH
    powerpc64-multilib-linux-gnu-gcc -o example example.c
    powerpc64-multilib-linux-gnu-ldd --root=`powerpc64-multilib-linux-gnu-gcc -print-sysroot` example

produces an output like this:

````
        libc.so.6 => /lib64/libc.so.6 (0x00000000deadbeef)
        ld64.so.1 => /lib64/ld64.so.1 (0x00000000deadbeef)
````

The load addresses are obviously bogus, as this script does not actually
load the libraries.
