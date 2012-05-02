/*
 * misc.h	All that didn't fit elsewhere.
 *
 * Copyright (C) 1995 Olaf Kirch <okir@monad.swb.de>
 */

#ifndef MISC_H
#define MISC_H

/*
 * Generate random key, returning the length of the result. Currently,
 * weakrandomkey generates a maximum of 20 bytes are generated, but this
 * may change with future implementations.
 */
int	randomkey(unsigned char *keyout, int len);
int	weakrandomkey(unsigned char *keyout, int len);

extern int is_mountpoint(char *path);

#endif /* MISC_H */
