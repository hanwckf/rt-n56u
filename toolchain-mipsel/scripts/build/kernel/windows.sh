# This file declares functions to install the kernel headers for mingw64
# Copyright 2012 Yann Diorcet
# Licensed under the GPL v2. See COPYING in the root of this package

CT_DoKernelTupleValues()
{
    # Even we compile for x86_64 target architecture, the target OS have to
    # bet mingw32 (require by gcc and mingw-w64)
    CT_TARGET_KERNEL="mingw32"
}

do_kernel_get()
{
    :
}

do_kernel_extract()
{
    :
}

do_kernel_headers()
{
   :
}
