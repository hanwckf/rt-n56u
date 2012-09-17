/*
 *
 * Copyright (c) 2007  STMicroelectronics Ltd
 * Filippo Arcidiacono (filippo.arcidiacono@st.com)
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 * Taken from glibc 2.6
 *
 */

#include <fenv.h>
#include <fpu_control.h>

int
fesetenv (const fenv_t *envp)
{
  if (envp == FE_DFL_ENV)
      _FPU_SETCW (_FPU_DEFAULT);
  else
    {
      unsigned long int temp = envp->__fpscr;
      _FPU_SETCW (temp);
    }
  return 0;
}
