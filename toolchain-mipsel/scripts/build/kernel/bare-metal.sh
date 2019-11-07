# This file declares functions for bare metal kernel (IE. none)
# Copyright 2008 Yann E. MORIN
# Licensed under the GPL v2. See COPYING in the root of this package

CT_DoKernelTupleValues()
{
    # For bare-metal, there is no kernel part in the tuple
    CT_TARGET_KERNEL=
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
