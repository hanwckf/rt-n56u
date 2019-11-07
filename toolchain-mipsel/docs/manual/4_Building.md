
To build the toolchain, simply type:

    ct-ng build

This will use the above configuration to retrieve, extract and patch the
components, build, install and eventually test your newly built toolchain.
If all goes well, you should see something like this:

````
[INFO ]  Performing some trivial sanity checks
[INFO ]  Build started 20170319.002217
[INFO ]  Building environment variables
[EXTRA]  Preparing working directories
[EXTRA]  Installing user-supplied crosstool-NG configuration
[EXTRA]  =================================================================
[EXTRA]  Dumping internal crosstool-NG configuration
[EXTRA]    Building a toolchain for:
[EXTRA]      build  = x86_64-pc-linux-gnu
[EXTRA]      host   = x86_64-pc-linux-gnu
[EXTRA]      target = mipsel-sde-elf
[EXTRA]  Dumping internal crosstool-NG configuration: done in 0.05s (at 00:02)
[INFO ]  =================================================================
[INFO ]  Retrieving needed toolchain components' tarballs
[EXTRA]    Retrieving 'gmp-6.1.2'
[EXTRA]    Saving 'gmp-6.1.2.tar.xz' to local storage
[EXTRA]    Retrieving 'mpfr-3.1.5'
[EXTRA]    Saving 'mpfr-3.1.5.tar.xz' to local storage
...
[INFO ]  Installing cross-gdb
[EXTRA]    Configuring cross-gdb
[EXTRA]    Building cross-gdb
[EXTRA]    Installing cross-gdb
[EXTRA]    Installing '.gdbinit' template
[INFO ]  Installing cross-gdb: done in 98.55s (at 10:51)
[INFO ]  =================================================================
[INFO ]  Cleaning-up the toolchain's directory
[INFO ]    Stripping all toolchain executables
[EXTRA]    Creating toolchain aliases
[EXTRA]    Removing access to the build system tools
[EXTRA]    Removing installed documentation
[INFO ]  Cleaning-up the toolchain's directory: done in 0.42s (at 10:52)
[INFO ]  Build completed at 20170319.003309
[INFO ]  (elapsed: 10:51.42)
[INFO ]  Finishing installation (may take a few seconds)...
````

You are then free to add the toolchain's `/bin` directory in your PATH to use it
at will.


Stopping and restarting a build
-------------------------------

If you want to stop the build after a step you are debugging, you can pass
the variable `STOP` to make:

    ct-ng build STOP=some_step

Conversely, if you want to restart a build at a specific step you are
debugging, you can pass the `RESTART` variable to make:

    ct-ng build RESTART=some_step

Alternatively, you can call make with the name of a step to just do that
step:

    ct-ng libc_headers

which is equivalent to:

    ct-ng build RESTART=libc_headers STOP=libc_headers

The shortcuts `+step_name` and `step_name+` allow to respectively stop
or restart at that step. Thus

    ct-ng +libc_headers

is equivalent to

    ct-ng build STOP=libc_headers

and

    ct-ng libc_headers+

is equivalent to:

    ct-ng build RESTART=libc_headers

To obtain the list of acceptable steps, please call:

    ct-ng list-steps

Note that in order to restart a build, you’ll have to say `Y` to the config
option `CT_DEBUG_CT_SAVE_STEPS`, and that the previous build effectively went
that far.


Overriding the number of jobs
-----------------------------

If you want to override the number of jobs to run in (the `-j` option to
make), you can either re-enter the menuconfig, or simply add it on the
command line, as such:

    ct-ng build.4

which tells crosstool-NG to override the number of jobs to 4.

You can see the actions that support overriding the number of jobs in the
help menu. Those are the ones with `[.#]` after them (e.g., `build[.#]` or
`build-all[.#]`, and so on).

> **Note**
> 
> The crosstool-NG script `ct-ng` is a Makefile-script. It does **not**
> execute in parallel (there is not much to gain). When speaking of
> jobs, we are refering to the number of jobs when making the
> **components**. That is, we speak of the number of jobs used to build
> gcc, glibc, and so on.


Building all toolchains at once
-------------------------------

You can build all samples; simply call:

    ct-ng build-all

Note that it is *very* time consuming (depending on your machine configuration
and host OS, it takes from 24 hours to a full week). By default, this removes
each build tree after a *successful* build, but leaves the unpacked/patched
sources so that they can be re-used by the samples that follow). However, even
that consumes considerable amount of disk space given the variety of component
versions represented in samples.
