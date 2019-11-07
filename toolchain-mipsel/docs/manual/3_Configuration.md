
crosstool-NG is configured with a configurator presenting a menu-structured
set of options. These options let you specify the way you want your toolchain
built, where you want it installed, what architecture and specific processor
it will support, the version of the components you want to use, etc.
The value for those options are then stored in a configuration file.

The configurator works the same way you configure your Linux kernel. It is
assumed you know how to handle this.

To enter the menu, type:

    ct-ng menuconfig

Almost every config item has a help entry. Read them carefully.

String and number options can refer to environment variables. In such a case,
you must use the shell syntax: `${VAR}`. You shall neither single- nor double-
quote the string/number options.

There are three environment variables that are computed by crosstool-NG, and
that you can use:

-   `CT_TARGET`: Represents the target tuple you are building for.
    You can use it for example in the installation/prefix directory,
    such as: `/opt/x-tools/${CT_TARGET}`. If needed, parts of `CT_TARGET`
    are also available as `CT_TARGET_ARCH`, `CT_TARGET_VENDOR`,
    `CT_TARGET_KERNEL` and `CT_TARGET_SYS`.

-   `CT_TOP_DIR`: The top directory where crosstool-NG is running.
    You shouldn't need it in most cases. One case where you
    may need it is if you have local patches or config files and
    you store them in your current working directory, you can refer
    to them by using `CT_TOP_DIR`, such as:
    `${CT_TOP_DIR}/patches.myproject`.

-   `CT_VERSION`: The version of crosstool-NG you are using. Not much
    use for you, but it's there if you need it.

-   You can also refer to the config variables recursively, but take
    care to avoid circular dependencies or nesting the references too
    deep (crosstool-NG currently only follows them to a depth of 10).


Sample configurations
---------------------

Crosstool-NG ships with several sample configurations (pre-configured toolchains
that are known to build and work). Sample names are 1- to 4-part tuples.
To get the list of these samples and see more detailed information on any sample,
do (replace "arm-unknown-linux-gnueabi" with the sample name you want to view):

    ct-ng list-samples
    ct-ng show-arm-unknown-linux-gnueabi

Once you chose one sample as a starting point, load it as a base and fine-tune
using `ct-ng menuconfig` as described above:

    ct-ng arm-unknown-linux-gnueabi
    ct-ng menuconfig

Interesting config options
--------------------------

-   `CT_LOCAL_TARBALLS_DIR`: If you already have some tarballs in a
    directory, enter it here. That will speed up the retrieving phase,
    where crosstool-NG would otherwise download those tarballs.

-   `CT_PREFIX_DIR`: This is where the toolchain will be installed in
    (and for now, where it will run from). Common use is to add the
    target tuple in the directory path, such as (see above):
    `/opt/x-tools/${CT_TARGET}`.

-   `CT_TARGET_VENDOR`: An identifier for your toolchain, will take
    place in the vendor part of the target tuple. It shall *not*
    contain spaces or dashes. Usually, keep it to a one-word string,
    or use underscores to separate words if you need. Avoid dots,
    commas, and special characters. It can be set to empty, to
    remove the vendor string from the target tuple.

-   `CT_TARGET_ALIAS`: An alias for the toolchain. It will be used as
    a prefix to the toolchain tools. For example, you will have
    `${CT_TARGET_ALIAS}-gcc`.

Also, if you think you don't see enough versions, you can try to enable one of
those:

-   `CT_OBSOLETE`: Show obsolete versions of tools. Most of the time,
    you don't want to base your toolchain on too old a version (of
    gcc, for example). But at times, it can come handy to use such an
    old version for regression tests or to support some outdated
    system configuration. Those old versions are hidden
    behind `CT_OBSOLETE`. Those versions (or features) are so marked
    because maintaining support for those in crosstool-NG would be too
    costly, time-wise, and time is dear. Note that these versions are
    likely going to disappear in the next crosstool-NG release.

-   `CT_EXPERIMENTAL`: Show experimental versions of tools and crosstool-NG
    features.  This may enable using unreleased versions of the tools,
    or configure the toolchain in a way that is not thoroughly tested.
    Use with care.


Re-building an existing toolchain
---------------------------------

If you have an existing toolchain, you can re-use the options used to build it
to create a new toolchain. That needs a very little bit of effort on your side
but is quite easy. The options to build a toolchain are saved with the
toolchain, and you can retrieve this configuration by running:

    ${CT_TARGET}-ct-ng.config

An alternate method is to extract the configuration from a `build.log` file.
This will be necessary if your toolchain was build with crosstool-NG prior to
`1.4.0`, but can be used with build.log files from any version:

    ct-ng extractconfig <build.log >.config

Or, if your `build.log` file is compressed (most probably!):

    bzcat build.log.bz2 | ct-ng extractconfig >.config

The above commands will dump the configuration to `stdout`, so to rebuild a
toolchain with this configuration, just redirect the output to the `.config`
file:

    ${CT_TARGET}-ct-ng.config >.config
    ct-ng oldconfig

Then, you can review and change the configuration by running:

    ct-ng menuconfig

> **Note**
>
> This procedure applies to rebuilding the toolchain using same version of
> crosstool-NG.

Upgrading from a previous crosstool-NG configuration
----------------------------------------------------

Before the 1.24.0 release of crosstool-NG, there was no specific upgrade
procedure. The procedure described in the previous section worked in some
cases; it didn't handle the upgrades when some options were removed or
renamed. This particularly affects the selected versions of various
toolchain components; so, after running the `ct-ng oldconfig` (or
`ct-ng olddefconfig`) you must verify all the settings.

Starting with the 1.24.0 version of crosstool-NG, a new command has
been introduced, `ct-ng upgradeconfig`. It can upgrade the configurations
from the 1.23.0 version; upgrading from versions in between 1.23.0 and 1.24.0
is not supported. It is expected that after the 1.24.0 release even the
interim unreleased versions will be upgradable using the same command.
