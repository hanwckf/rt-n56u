/*
 * Wrapper for XZ decompressor to make it usable for kernel and initramfs
 * decompression
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#ifndef DECOMPRESS_UNXZ_H
#define DECOMPRESS_UNXZ_H

int unxz(unsigned char *in, int in_size, unsigned char *out, int *in_used);

#endif

