
**crosstool-NG** aims at building toolchains. Toolchains are an essential
component in a software development project. It will compile, assemble and
link the code that is being developed. Some pieces of the toolchain will
eventually end up in the resulting binaries: static libraries are but an
example.

So, a toolchain is a very sensitive piece of software, as any bug in one of the
components, or a poorly configured component, can lead to execution problems,
ranging from poor performance, to applications ending unexpectedly, to
misbehaving software (which more than often is hard to detect), to hardware
damage, or even to human risks (which is more than regrettable).

Toolchains are made of different piece of software, each being quite complex
and requiring specially crafted options to build and work seamlessly. This is
usually not that easy, even in the not-so-trivial case of native toolchains.
The work reaches a higher degree of complexity when it comes to
cross-compilation, where it can become quite a nightmare …

Some cross-toolchains exist on the internet, and can be used for general
development, but they have a number of limitations:

-   They can be general purpose, in that they are configured for
    the majority: no optimisation for your specific target.

-   They can be prepared for a specific target and thus are not easy
    to use, nor optimised for, or even supporting your target.

-   They often are using aging components (compiler, C library, etc.)
    not supporting special features of your shiny new processor.

On the other side, these toolchain offer some advantages:

-   They are ready to use and quite easy to install and setup.

-   They are proven if used by a wide community.

But once you want to get all the juice out of your specific hardware, you will
want to build your own toolchain. This is where **crosstool-NG** comes into play.

There are also a number of tools that build toolchains for specific needs,
which are not really scalable. Examples are:

-   [buildroot](http://buildroot.uclibc.org/), whose main purpose is to build
    root file systems, hence the name. But once you have your toolchain with
    buildroot, part of it is installed in the root-to-be, so if you want to
    build a whole new root, you either have to save the existing one as a
    template and restore it later, or restart again from scratch. This is not
    convenient.

-   [ptxdist](http://www.ptxdist.org/), whose
    purpose is very similar to buildroot.

-   Other projects (e.g., [openembedded.org](http://www.openembedded.org/)),
    which are again used to build root file systems.

**crosstool-NG** is really targeted at building toolchains, and only toolchains.
It is then up to you to use it the way you want.

History <a name="history"></a>
-------

crosstool was first *conceived* by Dan Kegel, who offered it to the community
as a set of scripts, a repository of patches, and some pre-configured, general
purpose setup files to be used to configure crosstool. This is available at
[kegel.com/crosstool](http://www.kegel.com/crosstool), and the subversion
repository is hosted on [Google Code](http://code.google.com/p/crosstool/).

Yann E. Morin once managed to add support for uClibc-based toolchains, but it
did not make it into mainline, mostly because Yann didn’t have time to port
the patch forward to the new versions, due in part to the big effort it was
taking.

So Yann decided to clean up crosstool in the state it was, re-order the things
in place, add appropriate support for what he needed, that is uClibc support
and a menu-driven configuration, named the new implementation **crosstool-NG**,
(standing for crosstool Next Generation, as many other community projects do,
and as a wink at the TV series "Star Trek: The Next Generation" ;-) ) and made
it available to the community, in case it was of interest to any one.

In late 2014, Yann became very busy with buildroot and other projects, and so
Bryan Hundven opted to become the new maintainer for **crosstool-NG**.

Referring to **crosstool-NG** <a name="name"></a>
-----------------------------

The long name of the project is **crosstool-NG**:

-   no leading uppercase (except as first word in a sentence),

-   crosstool and NG separated with a hyphen (dash),

-   NG in uppercase.

Crosstool-NG can also be referred to by its short name CT-NG:

-   all in uppercase,

-   CT and NG separated with a hyphen (dash).

The long name is preferred over the short name, except in mail subjects,
where the short name is a better fit.

When referring to a specific version of **crosstool-NG**, append the
version number either as:

-   `crosstool-NG X.Y.Z` (long name, a space, and the version string)

-   `crosstool-ng-X.Y.Z` (long name in lowercase, a hyphen (dash),
    and the version string) -- this is used to name the release tarballs

-   `crosstool-ng-X.Y.Z+git_id` (long name in lowercase, a hyphen,
    the version string, and the Git ID as returned by `ct-ng version`)
    -- this is used to differentiate between releases and snapshots

The frontend to **crosstool-NG** is the command `ct-ng`:

-   all in lowercase,

-   ct and ng separated by a hyphen (dash).


