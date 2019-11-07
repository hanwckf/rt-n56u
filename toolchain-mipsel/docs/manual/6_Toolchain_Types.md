
There are four kinds of toolchains you could encounter.

First off, you must understand the following: when it comes to compilers
there are up to four machines involved:

1.  the machine configuring the toolchain components: the **config**
    machine
2.  the machine building the toolchain components: the **build** machine
3.  the machine running the toolchain: the **host** machine
4.  the machine the toolchain is generating code for: the **target**
    machine

We can most of the time assume that the config machine and the build
machine are the same. Most of the time, this will be true. The only time
it isn’t is if you’re using distributed compilation (such as distcc).
Let’s forget this for the sake of simplicity.

So we’re left with three machines:

-   build
-   host
-   target

Any toolchain will involve those three machines. You can be as pretty
sure of this as “2 and 2 are 4”. Here is how they come into play:

1.  build == host == target (**“native”**)

    This is a plain native toolchain, targeting the exact same machine
    as the one it is built on, and running again on this exact same
    machine. You have to build such a toolchain when you want to use an
    updated component, such as a newer gcc for example.

2.  build == host != target (**“cross”**)

    This is a classic cross-toolchain, which is expected to be run on
    the same machine it is compiled on, and generate code to run on a
    second machine, the target.

3.  build != host == target (**“cross-native”**)

    Such a toolchain is also a native toolchain, as it targets the same
    machine as it runs on. But it is build on another machine. You want
    such a toolchain when porting to a new architecture, or if the build
    machine is much faster than the host machine.

4.  build != host != target (**“canadian”**)

    This one is called a “Canadian Cross”¹ toolchain, and is tricky.
    The three machines in play are different. You might want such a
    toolchain if you have a fast build machine, but the users will use
    it on another machine, and will produce code to run on a third
    machine.

    The term Canadian Cross was coined because at the time that these
    issues were all being hashed out, Canada had three national
    political parties
    [(per Wikipedia)](http://en.wikipedia.org/wiki/Cross_compiler#Canadian_Cross).

**crosstool-NG** can build all these kinds of toolchains, or is aiming at it,
anyway. There are a few caveats, though.

While building a "native" toolchain, crosstool-ng will currently still
compile new version of libc for the target. There is currently no way
to use the system libc and/or system kernel headers as a part of
the toolchain. This may work if you choose a compatible version (i.e.,
the applications compiled with the toolchain will load the system libc).

A "cross-native" toolchain can be built as a trivial case of the "canadian"
toolchain. It is suboptimal, as it makes crosstool-NG build the tools
targeting the host machine twice (first, as a separate toolchain which
is a prerequisite for all canadian builds; and second, as a part of temporary
toolchain created as a part of the canadian build itself). This will likely
be improved in the future.

To build a "canadian" toolchain, you must build a toolchain that runs on build
and targets the host as a prerequisite (i.e., a simple cross). Then, add the
`/bin` directory of this prerequisite to the `$PATH` environment variable and
configure the canadian, specifying the target of the prerequisite toolchain
as the host of the new toolchain.

There are a few samples of canadian toolchains shipped with crosstool-NG. The
names of the canadian samples consist of two comma-separated parts, i.e.
`HOST,TARGET`. They require `HOST` sample as a prerequisiste. For example:

    ct-ng x86_64-w64-mingw32
    ct-ng build
    PATH=~/x-tools/x86_64-w64-mingw32/bin:$PATH
    ct-ng x86_64-w64-mingw32,x86_64-pc-linux-gnu
    ct-ng build

Note that you will not be able to run the binaries from the canadian toolchain
on your build machine! You need to transfer them to a machine running the OS
configured as the host.
