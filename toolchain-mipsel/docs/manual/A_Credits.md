
This section comes from crosstool-NG author, Yann Morin. Now that crosstool-NG
uses Git (and previously used Mercurial), each contribution is attributed to
its author, so this file will probably not be updated anymore.

I would like to thank these fine people for making crosstool-NG possible:

[Dan Kegel](http://www.kegel.com/), the original author of crosstool:
  - Dan was very helpful and willing to help when I build my first toolchains.
  I owe him one. Thank you Dan!  Some crosstool-NG scripts have code snippets
  coming almost as-is from the original work by Dan.

And in order of appearance on the mailing list:

Allan Clark:
  - Allan made extensive tests of the first alpha of crosstool-NG on his
    MacOS-X, and unveiled some bash-2.05 weirdness.

Enrico Weigelt:
  - some improvements to the build procedure
  - `cxa_atexit` disabling for C libraries not supporting it (old uClibc)
  - misc suggestions (restartable build, ...)
  - get rid of some bashisms in ./configure
  - contributed OpenRISC or32 support

Robert P. J. Day:
  - some small improvements to the configurator, misc prompting glitches
  - 'sanitised' patches for binutils-2.17
  - patches for glibc-2.5
  - misc patches, typos and eye candy
  - too many to list any more!

Al Stone:
  - initial ia64 support
  - some cosmetics

Szilveszter Ordog:
  - a uClibc floating point fix
  - initial support for ARM EABI

Mark Jonas:
  - initiated Super-H port

Michael Abbott:
  - make it build with ancient findutils

Willy Tarreau:
  - a patch to glibc to build on 'ancient' shells
  - reported mis-use of `$CT_CC_NATIVE`

Matthias Kaehlcke:
  - fix building glibc-2.7 (and 2.6.1) with newer kernels

Daniel Dittmann:
  - PowerPC support

Ioannis E. Venetis:
  - preliminary Alpha support
  - intense gcc-4.3 brainstorming

Thomas Jourdan:
  - intense gcc-4.3 brainstorming
  - eglibc support

Konrad Eisele:
  - initial [multilib support](http://sourceware.org/ml/crossgcc/2011-11/msg00040.html)

Many others have contributed, either in form of patches, suggestions,
comments, or testing... Thank you to all of you!

Special dedication to the [buildroot](http://buildroot.org) people for maintaining a set of patches I
happily and shamelessly vampirize from time to time.
