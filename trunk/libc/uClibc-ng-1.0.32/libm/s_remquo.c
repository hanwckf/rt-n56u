/* Copyright (C) 2002 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include "math.h"
#include "math_private.h"

double remquo(double x, double y, int *quo)	/* wrapper remquo */
{
        int signx, signy, signres;
        int mswx;
        int mswy;
        double x_over_y;

        GET_HIGH_WORD(mswx, x);
        GET_HIGH_WORD(mswy, y);

        signx = (mswx & 0x80000000) >> 31;
        signy = (mswy & 0x80000000) >> 31;

        signres = (signx ^ signy) ? -1 : 1;

        x_over_y = fabs(x / y);

        *quo = signres * (lrint(x_over_y) & 0x7f);

        return remainder(x,y);
}
libm_hidden_def(remquo)
