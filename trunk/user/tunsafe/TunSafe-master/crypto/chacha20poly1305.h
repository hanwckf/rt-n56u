#pragma once
#include "tunsafe_types.h"


enum {
  XCHACHA20POLY1305_NONCELEN = 24,
  CHACHA20POLY1305_KEYLEN = 32,
  CHACHA20POLY1305_AUTHTAGLEN = 16
};


void chacha20poly1305_decrypt_get_mac(uint8 *dst, const uint8 *src, const size_t src_len,
                                      const uint8 *ad, const size_t ad_len,
                                      const uint64 nonce, const uint8 key[CHACHA20POLY1305_KEYLEN],
                                      uint8 mac[CHACHA20POLY1305_AUTHTAGLEN]);

bool chacha20poly1305_decrypt(uint8 *dst, const uint8 *src, const size_t src_len,
                              const uint8 *ad, const size_t ad_len,
                              const uint64 nonce, const uint8 key[CHACHA20POLY1305_KEYLEN]);

void chacha20poly1305_encrypt(uint8 *dst, const uint8 *src, const size_t src_len,
                              const uint8 *ad, const size_t ad_len,
                              const uint64 nonce, const uint8 key[CHACHA20POLY1305_KEYLEN]);


void xchacha20poly1305_encrypt(uint8 *dst, const uint8 *src, const size_t src_len,
                               const uint8 *ad, const size_t ad_len,
                               const uint8 nonce[XCHACHA20POLY1305_NONCELEN],
                               const uint8 key[CHACHA20POLY1305_KEYLEN]);

bool xchacha20poly1305_decrypt(uint8 *dst, const uint8 *src, const size_t src_len,
                               const uint8 *ad, const size_t ad_len,
                               const uint8 nonce[XCHACHA20POLY1305_NONCELEN],
                               const uint8 key[CHACHA20POLY1305_KEYLEN]);

void poly1305_get_mac(const uint8 *src, size_t src_len,
                     const uint8 *ad, const size_t ad_len,
                     const uint64 nonce, const uint8 key[CHACHA20POLY1305_KEYLEN],
                     uint8 mac[CHACHA20POLY1305_AUTHTAGLEN]);


struct chacha20_streaming {
  uint32 left;
  uint8 buf[64];
  uint32 state[16];
};

void chacha20_streaming_init(chacha20_streaming *state, uint8 key[CHACHA20POLY1305_KEYLEN]);
void chacha20_streaming_crypt(chacha20_streaming *state, uint8 *dst, size_t size);
