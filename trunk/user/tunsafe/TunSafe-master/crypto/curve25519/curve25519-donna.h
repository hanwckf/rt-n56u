#ifndef TUNSAFE_CRYPTO_CURVE25519_DONNA_H_
#define TUNSAFE_CRYPTO_CURVE25519_DONNA_H_

#include "tunsafe_types.h"

void curve25519_donna_ref(uint8 *mypublic, const uint8 *secret, const uint8 *basepoint);
extern "C" void curve25519_donna_x64(uint8 *mypublic, const uint8 *secret, const uint8 *basepoint);

#if defined(ARCH_CPU_X86_64) && defined(COMPILER_MSVC)
#define curve25519_donna curve25519_donna_x64
#else
#define curve25519_donna curve25519_donna_ref
#endif

void curve25519_normalize(uint8 *e);

extern const uint8 kCurve25519Basepoint[32];

#endif  // TUNSAFE_CRYPTO_CURVE25519_DONNA_H_