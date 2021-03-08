/*
 * Copyright (C) 2018 Denys Vlasenko
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */
#define CURVE25519_KEYSIZE 32
void curve25519(uint8_t *result, const uint8_t *e, const uint8_t *q) FAST_FUNC;
