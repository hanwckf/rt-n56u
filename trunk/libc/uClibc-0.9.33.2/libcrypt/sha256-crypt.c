/* One way encryption based on SHA256 sum.
   Copyright (C) 2007, 2009 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2007.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "sha256.h"
#include "libcrypt.h"

/* Define our magic string to mark salt for SHA256 "encryption"
   replacement.  */
static const char sha256_salt_prefix[] = "$5$";

/* Prefix for optional rounds specification.  */
static const char sha256_rounds_prefix[] = "rounds=";

/* Maximum salt string length.  */
#define SALT_LEN_MAX 16
/* Default number of rounds if not explicitly specified.  */
#define ROUNDS_DEFAULT 5000
/* Minimum number of rounds.  */
#define ROUNDS_MIN 1000
/* Maximum number of rounds.  */
#define ROUNDS_MAX 999999999

/* Table with characters for base64 transformation.  */
static const char b64t[64] =
"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

#define B64_FROM_24BIT(b2, b1, b0, steps) \
	{ \
		int n = (steps); \
		unsigned int w = ((b2) << 16) | ((b1) << 8) | (b0); \
		while (n-- > 0 && buflen > 0) \
		{ \
			*cp++ = b64t[w & 0x3f]; \
			--buflen; \
			w >>= 6; \
		} \
	}

char *
__sha256_crypt_r (const char *key,
     const char *salt,
     char *buffer,
     int buflen)
{
  unsigned char alt_result[32]
    __attribute__ ((__aligned__ (__alignof__ (uint32_t))));
  unsigned char temp_result[32]
    __attribute__ ((__aligned__ (__alignof__ (uint32_t))));
  size_t salt_len;
  size_t key_len;
  size_t cnt;
  char *cp;
  char *copied_key = NULL;
  char *copied_salt = NULL;
  char *p_bytes;
  char *s_bytes;
  /* Default number of rounds.  */
  size_t rounds = ROUNDS_DEFAULT;
  bool rounds_custom = false;

  /* Find beginning of salt string.  The prefix should normally always
     be present.  Just in case it is not.  */
  if (strncmp (sha256_salt_prefix, salt, sizeof (sha256_salt_prefix) - 1) == 0)
    /* Skip salt prefix.  */
    salt += sizeof (sha256_salt_prefix) - 1;

  if (strncmp (salt, sha256_rounds_prefix, sizeof (sha256_rounds_prefix) - 1)
      == 0)
    {
      const char *num = salt + sizeof (sha256_rounds_prefix) - 1;
      char *endp;
      unsigned long int srounds = strtoul (num, &endp, 10);
      if (*endp == '$')
	{
	  salt = endp + 1;
	  rounds = MAX (ROUNDS_MIN, MIN (srounds, ROUNDS_MAX));
	  rounds_custom = true;
	}
    }

  salt_len = MIN (strcspn (salt, "$"), SALT_LEN_MAX);
  key_len = strlen (key);

  if ((key - (char *) 0) % __alignof__ (uint32_t) != 0)
    {
      char *tmp = (char *) alloca (key_len + __alignof__ (uint32_t));
      key = copied_key =
	memcpy (tmp + __alignof__ (uint32_t)
		- (tmp - (char *) 0) % __alignof__ (uint32_t),
		key, key_len);
      assert ((key - (char *) 0) % __alignof__ (uint32_t) == 0);
    }

  if ((salt - (char *) 0) % __alignof__ (uint32_t) != 0)
    {
      char *tmp = (char *) alloca (salt_len + __alignof__ (uint32_t));
      salt = copied_salt =
	memcpy (tmp + __alignof__ (uint32_t)
		- (tmp - (char *) 0) % __alignof__ (uint32_t),
		salt, salt_len);
      assert ((salt - (char *) 0) % __alignof__ (uint32_t) == 0);
    }

  struct sha256_ctx ctx;
  struct sha256_ctx alt_ctx;

  /* Prepare for the real work.  */
  __sha256_init_ctx (&ctx);

  /* Add the key string.  */
  __sha256_process_bytes (key, key_len, &ctx);

  /* The last part is the salt string.  This must be at most 16
     characters and it ends at the first `$' character.  */
  __sha256_process_bytes (salt, salt_len, &ctx);


  /* Compute alternate SHA256 sum with input KEY, SALT, and KEY.  The
     final result will be added to the first context.  */
  __sha256_init_ctx (&alt_ctx);

  /* Add key.  */
  __sha256_process_bytes (key, key_len, &alt_ctx);

  /* Add salt.  */
  __sha256_process_bytes (salt, salt_len, &alt_ctx);

  /* Add key again.  */
  __sha256_process_bytes (key, key_len, &alt_ctx);

  /* Now get result of this (32 bytes) and add it to the other
     context.  */
  __sha256_finish_ctx (&alt_ctx, alt_result);

  /* Add for any character in the key one byte of the alternate sum.  */
  for (cnt = key_len; cnt > 32; cnt -= 32)
    __sha256_process_bytes (alt_result, 32, &ctx);
  __sha256_process_bytes (alt_result, cnt, &ctx);

  /* Take the binary representation of the length of the key and for every
     1 add the alternate sum, for every 0 the key.  */
  for (cnt = key_len; cnt > 0; cnt >>= 1)
    if ((cnt & 1) != 0)
      __sha256_process_bytes (alt_result, 32, &ctx);
    else
      __sha256_process_bytes (key, key_len, &ctx);

  /* Create intermediate result.  */
  __sha256_finish_ctx (&ctx, alt_result);

  /* Start computation of P byte sequence.  */
  __sha256_init_ctx (&alt_ctx);

  /* For every character in the password add the entire password.  */
  for (cnt = 0; cnt < key_len; ++cnt)
    __sha256_process_bytes (key, key_len, &alt_ctx);

  /* Finish the digest.  */
  __sha256_finish_ctx (&alt_ctx, temp_result);

  /* Create byte sequence P.  */
  cp = p_bytes = alloca (key_len);
  for (cnt = key_len; cnt >= 32; cnt -= 32)
    cp = mempcpy (cp, temp_result, 32);
  memcpy (cp, temp_result, cnt);

  /* Start computation of S byte sequence.  */
  __sha256_init_ctx (&alt_ctx);

  /* For every character in the password add the entire password.  */
  for (cnt = 0; cnt < 16 + alt_result[0]; ++cnt)
    __sha256_process_bytes (salt, salt_len, &alt_ctx);

  /* Finish the digest.  */
  __sha256_finish_ctx (&alt_ctx, temp_result);

  /* Create byte sequence S.  */
  cp = s_bytes = alloca (salt_len);
  for (cnt = salt_len; cnt >= 32; cnt -= 32)
    cp = mempcpy (cp, temp_result, 32);
  memcpy (cp, temp_result, cnt);

  /* Repeatedly run the collected hash value through SHA256 to burn
     CPU cycles.  */
  for (cnt = 0; cnt < rounds; ++cnt)
    {
      /* New context.  */
      __sha256_init_ctx (&ctx);

      /* Add key or last result.  */
      if ((cnt & 1) != 0)
	__sha256_process_bytes (p_bytes, key_len, &ctx);
      else
	__sha256_process_bytes (alt_result, 32, &ctx);

      /* Add salt for numbers not divisible by 3.  */
      if (cnt % 3 != 0)
	__sha256_process_bytes (s_bytes, salt_len, &ctx);

      /* Add key for numbers not divisible by 7.  */
      if (cnt % 7 != 0)
	__sha256_process_bytes (p_bytes, key_len, &ctx);

      /* Add key or last result.  */
      if ((cnt & 1) != 0)
	__sha256_process_bytes (alt_result, 32, &ctx);
      else
	__sha256_process_bytes (p_bytes, key_len, &ctx);

      /* Create intermediate result.  */
      __sha256_finish_ctx (&ctx, alt_result);
    }

  /* Now we can construct the result string.  It consists of three
     parts.  */
  cp = stpncpy (buffer, sha256_salt_prefix, MAX (0, buflen));
  buflen -= sizeof (sha256_salt_prefix) - 1;

  if (rounds_custom)
    {
      int n = snprintf (cp, MAX (0, buflen), "%s%zu$",
			sha256_rounds_prefix, rounds);
      cp += n;
      buflen -= n;
    }

  cp = stpncpy (cp, salt, MIN ((size_t) MAX (0, buflen), salt_len));
  buflen -= MIN ((size_t) MAX (0, buflen), salt_len);

  if (buflen > 0)
    {
      *cp++ = '$';
      --buflen;
    }

  B64_FROM_24BIT (alt_result[0], alt_result[10], alt_result[20], 4);
  B64_FROM_24BIT (alt_result[21], alt_result[1], alt_result[11], 4);
  B64_FROM_24BIT (alt_result[12], alt_result[22], alt_result[2], 4);
  B64_FROM_24BIT (alt_result[3], alt_result[13], alt_result[23], 4);
  B64_FROM_24BIT (alt_result[24], alt_result[4], alt_result[14], 4);
  B64_FROM_24BIT (alt_result[15], alt_result[25], alt_result[5], 4);
  B64_FROM_24BIT (alt_result[6], alt_result[16], alt_result[26], 4);
  B64_FROM_24BIT (alt_result[27], alt_result[7], alt_result[17], 4);
  B64_FROM_24BIT (alt_result[18], alt_result[28], alt_result[8], 4);
  B64_FROM_24BIT (alt_result[9], alt_result[19], alt_result[29], 4);
  B64_FROM_24BIT (0, alt_result[31], alt_result[30], 3);
  if (buflen <= 0)
    {
      __set_errno (ERANGE);
      buffer = NULL;
    }
  else
    *cp = '\0';		/* Terminate the string.  */

  /* Clear the buffer for the intermediate result so that people
     attaching to processes or reading core dumps cannot get any
     information.  We do it in this way to clear correct_words[]
     inside the SHA256 implementation as well.  */
  __sha256_init_ctx (&ctx);
  __sha256_finish_ctx (&ctx, alt_result);
  memset (&ctx, '\0', sizeof (ctx));
  memset (&alt_ctx, '\0', sizeof (alt_ctx));

  memset (temp_result, '\0', sizeof (temp_result));
  memset (p_bytes, '\0', key_len);
  memset (s_bytes, '\0', salt_len);
  if (copied_key != NULL)
    memset (copied_key, '\0', key_len);
  if (copied_salt != NULL)
    memset (copied_salt, '\0', salt_len);

  return buffer;
}

static char *buffer;

/* This entry point is equivalent to the `crypt' function in Unix
   libcs.  */
char *
__sha256_crypt (const unsigned char *key, const unsigned char *salt)
{
  /* We don't want to have an arbitrary limit in the size of the
     password.  We can compute an upper bound for the size of the
     result in advance and so we can prepare the buffer we pass to
     `sha256_crypt_r'.  */
  static int buflen;
  int needed = (sizeof (sha256_salt_prefix) - 1
		+ sizeof (sha256_rounds_prefix) + 9 + 1
		+ strlen (salt) + 1 + 43 + 1);

  if (buflen < needed)
    {
      char *new_buffer = (char *) realloc (buffer, needed);
      if (new_buffer == NULL)
	return NULL;

      buffer = new_buffer;
      buflen = needed;
    }

  return __sha256_crypt_r ((const char *) key, (const char *) salt, buffer, buflen);
}
