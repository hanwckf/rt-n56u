
This files lists the known issues encountered while developing crosstool-NG,
but that could not be addressed before the release.

The file has one section for each known issue, each section containing four
sub-sections: Symptoms, Explanations, Fix, and Workaround.

Each section is separated from the others with a lines of at least 4 dashes.

The following dummy section explains it all.

--------------------------------
**Symptoms**:
  A one- or two-liner of what you would observe.
  Usually, the error message you would see in the build logs.

**Explanations**:
  An as much as possible in-depth explanations of the context, why it
  happens, what has been investigated so far, and possible orientations
  as how to try to solve this (eg. URLs, code snippets...).

**Status**:
  Tells about the status of the issue:
  - UNCONFIRMED : missing information, or unable, to reproduce, but there
                  is consensus that there is an issue somewhere...
  - CURRENT     : the issue is applicable.
  - DEPRECATED  : the issue used to apply in some cases, but has not been
                  confirmed or reported again lately.
  - CLOSED      : the issue is no longer valid, and a fix has been added
                  either as a patch to this component, and/or as a
                  workaround in the scripts and/or the configuration.

**Fix**:
  What you have to do to fix it, if at all possible.
  The fact that there is a fix, and yet this is a known issue means that
  time to incorporate the fix in crosstool-NG was missing, or planned for
  a future release.

**Workaround**:
  What you can do to fix it *temporarily*, if at all possible.
  A workaround is not a real fix, as it can break other parts of
  crosstool-NG, but at least makes you going in your particular case.

So now, on for the real issues...

--------------------------------
**Symptoms**:
  gcc is not found, although I *do* have gcc installed.

**Explanations**:
  This is an issue on at least RHEL systems, where gcc is a symlink to ccache.
  Because crosstool-NG create links to gcc for the build and host environment,
  those symlinks are in fact pointing to ccache, which then doesn't know how
  to run the compiler.

  A possible fix could probably set the environment variable CCACHE_CC to the
  actual compiler used.

**Status**:
  CURRENT

**Fix**:
  None known.

**Workaround**:
  Uninstall ccache.

--------------------------------
**Symptoms**:
  Build fails with: `unable to detect the exception model`

**Explanations**:
  On some architectures, proper stack unwinding (C++) requires that
  setjmp/longjmp (sjlj) be used, while on other architectures do not
  need sjlj. On some architectures, gcc is unable to determine whether
  sjlj are needed or not.

**Status**:
  CURRENT

**Fix**:
  None so far.

**Workaround**:
  Trying setting use of sjlj to either 'Y' or 'N' (instead of the
  default 'M') in the menuconfig, option `CT_CC_GCC_SJLJ_EXCEPTIONS`
  labeled "Use sjlj for exceptions".

--------------------------------
**Symptoms**:
  On x86_64 hosts with 32bit userspace the GMP build fails with:
````
    configure: error: Oops, mp_limb_t is 32 bits, but the assembler code
    in this configuration expects 64 bits.
    You appear to have set $CFLAGS, perhaps you also need to tell GMP the
    intended ABI, see "ABI and ISA" in the manual.
````

**Explanations**:
  `uname -m` detects x86_64 but the build host is really x86.

**Status**:
  CURRENT

**Fix**:
  None so far. See above issue.

**Workaround**:
  use "setarch i686 ct-ng build"

--------------------------------
