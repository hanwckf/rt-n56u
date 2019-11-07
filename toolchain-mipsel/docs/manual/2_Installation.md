
Before installing crosstool-NG, you may need to install additional packages on
the host OS.  Specific instructions for several supported operating systems and
distributions are provided [here](/docs/os-setup/).  Note that not all the
dependencies are currently detected by the `configure` script; missing some of
them may later result in failing `ct-ng build`.

There two ways how you can obtain the crosstool-NG sources:

-   by [downloading a released tarball](#download-tarball);

-   or by [cloning the current development repository](#clone).

There also are two ways you can use crosstool-NG:

-   [build and install it](#install-method), then get rid of the sources like
    you’d do for most programs;

-   or [only build it and run from the source directory](#hackers-way).

The typical workflow assumes using a released tarball and installing crosstool-NG.
If you intend to do some development on crosstool-NG and/or submit patches,
you'd likely want a clone of the repository and running from the source directory.


Downloading a released tarball <a name="download-tarball"></a>
------------------------------

First, download the tarball:

    wget http://crosstool-ng.org/download/crosstool-ng/crosstool-ng-VERSION.tar.bz2

or

    wget http://crosstool-ng.org/download/crosstool-ng/crosstool-ng-VERSION.tar.xz

Starting with 1.21.0, releases are signed with Bryan Hundven's PGP key. The
fingerprint is:

    561E D9B6 2095 88ED 23C6 8329 CAD7 C8FC 35B8 71D1

Starting with 1.23.0, releases are signed with Alexey Neyman's PGP key. The
fingerprint is:

    64AA FBF2 1475 8C63 4093 45F9 7848 649B 11D6 18A4

The public keys are found on [http://pgp.surfnet.nl/](http://pgp.surfnet.nl/).
To validate the release tarball run you need to import the keys from the keyserver
and download the signature of the tarball, then verify the tarball with both
the tarball and the signature in the same directory:

    gpg --keyserver http://pgp.surfnet.nl --recv-keys 35B871D1 11D618A4
    wget http://crosstool-ng.org/download/crosstool-ng/crosstool-ng-VERSION.tar.bz2.sig
    gpg --verify crosstool-ng-VERSION.tar.bz2.sig

<!-- TBD: who's key was used for 1.20.0? Yann's? what is the fingerprint/keyserver? -->

Crosstool-NG releases 1.19.0 and earlier provide MD5/SHA1/SHA512 digests for the tarballs.
Use `md5sum`/`sha1sum`/`sha512sum` commands to verify the tarballs:

    md5sum -c crosstool-ng-VERSION.tar.bz2.md5
    sha1sum -c crosstool-ng-VERSION.tar.bz2.sha1
    sha512sum -c crosstool-ng-VERSION.tar.bz2.sha512


Cloning a repository <a name="clone"></a>
--------------------

If the released version is not recent enough for your purposes, you
can try to build using the currently developed version. To do that,
clone the Git repository:

    git clone https://github.com/crosstool-ng/crosstool-ng

You'll need to run the `bootstrap` script before running configure:

    ./bootstrap

Install method <a name="install-method"></a>
--------------

First unpack the tarball and `cd` into the `crosstool-ng-VERSION` directory.

> **Note**
>
> Due to a bug in release scripts, version 1.22.0 of the crosstool-ng was
> packaged without the VERSION suffix.

Then follow the classical `configure` recipe:
`./configure` way:

    ./configure --prefix=/some/place
    make
    make install
    export PATH="${PATH}:/some/place/bin"

You can then get rid of crosstool-NG source. Next create a directory to serve
as a working place, `cd` in there and run:

    mkdir work-dir
    cd work-dir
    ct-ng help

> **Note**
>
> If you call `ct-ng --help` you will get help for `make(1)`. This is
> because ct-ng is in fact a `make(1)` script. There is no clean workaround for this.

A man page for the `ct-ng` utility is also installed. You can get some brief
help by typing `man ct-ng`.


The Hacker’s way <a name="hackers-way"></a>
----------------

Then, you run `./configure` for local execution of crosstool-NG:

    ./configure --enable-local
    make

Now, **do not** remove crosstool-NG sources. They are needed to run
crosstool-NG! Stay in the directory holding the sources, and run:

    ./ct-ng help


Preparing for packaging <a name="package-prep"></a>
-----------------------

If you plan on packaging crosstool-NG, you surely don’t want to install it
in your root file system. The install procedure of crosstool-NG honors the
`DESTDIR` variable:

    ./configure --prefix=/usr
    make
    make DESTDIR=/packaging/place install

Shell completion <a name="shell-completion"></a>
----------------

crosstool-NG comes with a shell script fragment that defines bash-compatible
completion. That shell fragment is currently not installed automatically.

To install the shell script fragment, you have two options:

-   install system-wide, most probably by copying `ct-ng.comp` into
    `/etc/bash_completion.d/`, or

-   install for a single user, by copying `ct-ng.comp` into `${HOME}/`
    and sourcing this file from your `${HOME}/.bashrc`.

Contributed code <a name="contributed-code"></a>
----------------

Some people contributed code that couldn’t get merged for various reasons.
This code is available as lzma-compressed patches, in the `contrib/`
sub-directory. These patches are to be applied to the source of crosstool-NG,
prior to installing, using something like the following:

    lzcat contrib/foobar.patch.lzma | patch -p1

There is no guarantee that a particular contribution applies to the current
version of crosstool-ng, or that it will work at all. Use contributions at
your own risk.
