
Sending a bug report
--------------------

Please file the bug reports on [Github](https://github.com/crosstool-ng/crosstool-ng/issues).

Crosstool-NG also has a mailing list at [crossgcc@sourceware.org](mailto:crossgcc@sourceware.org).
Archive and subscription info can be found here:
[https://sourceware.org/ml/crossgcc/](https://sourceware.org/ml/crossgcc/)

Sometimes you may be able to contact a maintainer on IRC at the #crosstool-ng
channel on the FreeNode net (irc.freenode.net).

Using a mailing list or IRC to triage an issue is okay. If it looks like a bug,
you'll be asked to file an issue at Github. If it looks like a pilot error,
you might get an advice over the mailing list or IRC faster.


Sending patches
---------------

The preferred method of patch submission is via the Github's pull
requests. The status of currently pending pull requests can be
checked [here](https://github.com/crosstool-ng/crosstool-ng/pulls).

Patches are also welcome at the mailing list.

Patches should come with the appropriate *Signed-off-by line* (SOB line). An SOB line is
typically something like:

    Signed-off-by: John DOE <john.doe@somewhere.net>

Why this line is needed is well described in the
[Linux kernel's documentation on patch submission](https://www.kernel.org/doc/html/latest/process/submitting-patches.html#sign-your-work-the-developer-s-certificate-of-origin).

You can also add any of the following lines if applicable:

    Acked-by:
    Tested-by:
    Reviewed-by:

If submitting patches over the mailing list, please also follow other
guidelines described in the [Linux kernel's guide](https://www.kernel.org/doc/html/latest/process/submitting-patches.html).

We previously used patchwork for development, but it is no longer used. I'd like
to see patches that are still applicable turned into Pull Requests on GitHub.
[Here](http://patchwork.ozlabs.org/project/crosstool-ng/) is the list of pending patches.
