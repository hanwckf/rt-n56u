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
feholdexcept (fenv_t *envp)
{
  unsigned long int temp;

  /* Store the environment.  */
  _FPU_GETCW (temp);
  envp->__fpscr = temp;

  /* Now set all exceptions to non-stop.  */
  temp &= ~FE_ALL_EXCEPT;
  _FPU_SETCW (temp);

  return 1;
}
