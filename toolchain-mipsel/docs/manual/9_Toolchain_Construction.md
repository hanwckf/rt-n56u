
This is the result of a [discussion with Francesco
Turco](http://sourceware.org/ml/crossgcc/2011-01/msg00060.html).

Francesco had a [nice tutorial for
beginners](http://fturco.org/wiki/doku.php?id=debian:cross-compiler)
[dead link, Wayback Machine has no archived version],
along with a sample, step-by-step procedure to build a toolchain for an
ARM target from an x86\_64 Debian host.

Thank you Francesco for initiating this!

I want a cross-compiler! What is this toolchain you’re speaking about?
----------------------------------------------------------------------

A cross-compiler is in fact a collection of different tools set up to
tightly work together. The tools are arranged in a way that they are
chained, in a kind of cascade, where the output from one becomes the
input to another one, to ultimately produce the actual binary code that
runs on a machine. So, we call this arrangement a “toolchain”. When a
toolchain is meant to generate code for a machine different from the
machine it runs on, this is called a cross-toolchain.

So, what are those components in a toolchain?
---------------------------------------------

The components that play a role in the toolchain are first and foremost
the compiler itself. The compiler turns source code (in C, C++,
whatever) into assembly code. The compiler of choice is the GNU compiler
collection, well known as `gcc`.

The assembly code is interpreted by the assembler to generate object
code. This is done by the binary utilities, such as the GNU *binutils*.

Once the different object code files have been generated, they got to
get aggregated together to form the final executable binary. This is
called linking, and is achieved with the use of a linker. The GNU
*binutils* also come with a linker.

So far, we get a complete toolchain that is capable of turning source
code into actual executable code. Depending on the Operating System, or
the lack thereof, running on the target, we also need the C library. The
C library provides a standard abstraction layer that performs basic
tasks (such as allocating memory, printing output on a terminal,
managing file access…). There are many C libraries, each targeted to
different systems. For the Linux *desktop*, there is `glibc` or `eglibc`
or even `uClibc`, for embedded Linux, you have a choice of `eglibc` or
`uClibc`, while for system without an operating system, you may use
`newlib`, `dietlibc`, or even none at all. There a few other C
libraries, but they are not as widely used, and/or are targeted to very
specific needs (e.g., `klibc` is a very small subset of the C library
aimed at building constrained initial ramdisks).

Under Linux, the C library needs to know the API to the kernel to decide
what features are present, and if needed, what emulation to include for
missing features. That API is provided by the kernel headers. Note: this
is Linux-specific (and potentially a very few others), the C library on
other OSes do not need the kernel headers.

And now, how do all these components chained together?
------------------------------------------------------

So far, all major components have been covered, but yet there is a
specific order they need to be built. Here we see what the dependencies
are, starting with the compiler we want to ultimately use. We call that
compiler the *final compiler*.

-   the final compiler needs the C library, to know how to use it, but:

-   building the C library requires a compiler

A needs B which needs A. This is the classic chicken’n'egg problem… This
is solved by building a stripped-down compiler that does not need the C
library, but is capable of building it. We call it a bootstrap, initial,
or core compiler. So here is the new dependency list:

-   the final compiler needs the C library, to know how to use it,

-   building the C library requires a core compiler but:

-   the core compiler needs the C library headers and start files, to
    know how to use the C library

B needs C which needs B. Chicken’n'egg, again. To solve this one, we
will need to build a C library that will only install its headers and
start files. The start files (also called "C runtime", or CRT) are
a very few files that gcc needs to be able to turn on thread local
storage (TLS) on an NPTL system. So now we have:

-   the final compiler needs the C library, to know how to use it,

-   building the C library requires a core compiler

-   the core compiler needs the C library headers and start files, to
    know how to use the C library but:

-   building the start files require a compiler

Geez… C needs D which needs C, yet again. So we need to build a yet
simpler compiler, that does not need the headers and does need the start
files. This compiler is also a bootstrap, initial or core compiler. In
order to differentiate the two core compilers, let’s call that one
`core pass 1`, and the former one `core pass 2`. The dependency list
becomes:

-   the final compiler needs the C library, to know how to use it,

-   building the C library requires a compiler

-   the core pass 2 compiler needs the C library headers and start
    files, to know how to use the C library

-   building the start files requires a compiler

-   we need a core pass 1 compiler

And as we said earlier, the C library also requires the kernel headers.
There is no requirement for the kernel headers, so end of story in this
case:

-   the final compiler needs the C library, to know how to use it,

-   building the C library requires a core compiler

-   the core pass 2 compiler needs the C library headers and start
    files, to know how to use the C library

-   building the start files requires a compiler and the kernel headers

-   we need a core pass 1 compiler

We need to add a few new requirements. The moment we compile code for
the target, we need the assembler and the linker. Such code is, of
course, built from the C library, so we need to build the binutils
before the C library start files, and the complete C library itself.
Also, some code in gcc will turn to run on the target as well. Luckily,
there is no requirement for the binutils. So, our dependency chain is as
follows:

-   the final compiler needs the C library, to know how to use it, and
    the binutils

-   building the C library requires a core pass 2 compiler and the
    binutils

-   the core pass 2 compiler needs the C library headers and start
    files, to know how to use the C library, and the binutils

-   building the start files requires a compiler, the kernel headers and
    the binutils

-   the core pass 1 compiler needs the binutils

Which turns in this order to build the components:

1.  binutils

2.  core pass 1 compiler

3.  kernel headers

4.  C library headers and start files

5.  core pass 2 compiler

6.  complete C library

7.  final compiler

Yes! :-) But are we done yet?

In fact, no, there are still missing dependencies. As far as the tools
themselves are involved, we do not need anything else.

But gcc has a few pre-requisites. It relies on a few external libraries
to perform some non-trivial tasks (such as handling complex numbers in
constants…). There are a few options to build those libraries. First,
one may think to rely on a Linux distribution to provide those
libraries. Alas, they were not widely available until very, very
recently. So, if the distro is not too recent, chances are that we will
have to build those libraries (which we do below). The affected
libraries are:

-   the GNU Multiple Precision Arithmetic Library, GMP;

-   the C library for multiple-precision floating-point computations
    with correct rounding, MPFR;

-   the C library for the arithmetic of complex numbers, MPC.

The dependencies for those libraries are:

-   MPC requires GMP and MPFR

-   MPFR requires GMP

-   GMP has no pre-requisite

So, the build order becomes:

1.  GMP

2.  MPFR

3.  MPC

4.  binutils

5.  core pass 1 compiler

6.  kernel headers

7.  C library headers and start files

8.  core pass 2 compiler

9.  complete C library

10. final compiler

Yes! Or yet some more?

This is now sufficient to build a functional toolchain. So if you’ve had
enough for now, you can stop here. Or if you are curious, you can
continue reading.

gcc can also make use of a few other external libraries. These
additional, optional libraries are used to enable advanced features in
gcc, such as loop optimisation (GRAPHITE) and Link Time Optimisation
(LTO). If you want to use these, you’ll need three additional libraries:

To enable GRAPHITE, depending on GCC version, it may need one or more of
the following:
- the Parma Polyhedra Library, PPL;
- the Integer Set Library, ISL;
- the Chunky Loop Generator, using the PPL backend, CLooG/PPL;
- the Chunky Loop Generator, using the ISL backend, CLooG.

To enable LTO: - the ELF object file access library, libelf

The dependencies for those libraries are:

-   PPL requires GMP;

-   CLooG/PPL requires GMP and one of PPL or ISL;

-   ISL has no prerequisites;

-   libelf has no pre-requisites.

The list now looks like:

1.  GMP

2.  MPFR

3.  MPC

4.  PPL (if needed)

5.  ISL (if needed)

5.  CLooG (if needed)

6.  libelf (if needed)

7.  binutils

8.  core pass 1 compiler

9.  kernel headers

10. C library headers and start files

11. core pass 2 compiler

12. complete C library

13. final compiler

This list is now complete! Wouhou! Or is it?


But why does crosstool-NG have more steps?
------------------------------------------

The already thirteen steps are the necessary steps, from a theoretical
point of view. In reality, though, there are small differences; there
are three different reasons for the additional steps in crosstool-NG.

First, the GNU binutils do not support some kinds of output. It is not
possible to generate *flat* binaries with binutils, so we have to use
another component that adds this support: `elf2flt`. `elf2flt` also
requires the `zlib` compression library - we may not be able to use
the host's zlib if we're building a canadian or cross-native toolchain.

Second, localizations of the toolchain require additional libraries
on some host OSes: `gettext` and `libiconv`.

Third, crosstool-NG can also build some additional debug utilities to
run on the target. This is where we build, for example, the `cross-gdb`,
the `gdbserver` and the native `gdb` (the last two run on the target,
the first runs on the same machine as the toolchain). The others
(`strace`, `ltrace`, DUMA and `dmalloc`) are absolutely not related to
the toolchain, but are nice-to-have stuff that can greatly help when
developing, so are included as goodies (and they are quite easy to
build, so it’s OK; more complex stuff is not worth the effort to include
in crosstool-NG).
