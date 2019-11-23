// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#include "stdafx.h"
#include "tunsafe_types.h"
#include "crypto/chacha20poly1305.h"
#include "crypto/aesgcm/aes.h"
#include "tunsafe_cpu.h"

#include <functional>
#include <string.h>

#if defined(OS_FREEBSD) || defined(OS_LINUX)
#include <time.h>
#include <stdlib.h>
typedef uint64 LARGE_INTEGER;
void QueryPerformanceCounter(LARGE_INTEGER *x) {
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
    fprintf(stderr, "clock_gettime failed\n");
    exit(1);
  }
  *x = (uint64)ts.tv_sec * 1000000000 + ts.tv_nsec;
}

void QueryPerformanceFrequency(LARGE_INTEGER *x) {
  *x = 1000000000;
}
#elif defined(OS_MACOSX)
#include <mach/mach.h>
#include <mach/mach_time.h>
typedef uint64 LARGE_INTEGER;

void QueryPerformanceCounter(LARGE_INTEGER *x) {
  *x = mach_absolute_time();
}

void QueryPerformanceFrequency(LARGE_INTEGER *x) {
  mach_timebase_info_data_t timebase = { 0, 0 };
  if (mach_timebase_info(&timebase) != 0)
    abort();
  printf("numer/denom: %d %d\n", timebase.numer, timebase.denom);
  *x = timebase.denom * 1000000000;  
}

#endif

int gcm_self_test();



void *fake_glb;
void Benchmark() {
  int64 a, b, f, t1 = 0, t2 = 0;

#if WITH_AESGCM
  gcm_self_test();
#endif  // WITH_AESGCM

  PrintCpuFeatures();

  QueryPerformanceFrequency((LARGE_INTEGER*)&f);

  uint8 dst[1500 + 16];
  uint8 key[32] = {0, 1, 2, 3, 4, 5, 6};
  uint8 mac[16];

  fake_glb = dst;

size_t max_bytes = 1000000000;
#if defined(ARCH_CPU_ARM_FAMILY)
  max_bytes = 100000000;
#endif
  auto RunOneBenchmark = [&](const char *name, const std::function<uint64(size_t)> &ff) {
    uint64 bytes = 0;
    QueryPerformanceCounter((LARGE_INTEGER*)&b);
    size_t i;
    for (i = 0; bytes < max_bytes; i++)
      bytes += ff(i);
    QueryPerformanceCounter((LARGE_INTEGER*)&a);
    RINFO("%s: %f MB/s", name, (double)bytes * 0.000001 / (a - b) * f);
  };

  memset(dst, 0, 1500);
  RunOneBenchmark("chacha20-encrypt", [&](size_t i) -> uint64 { chacha20poly1305_encrypt(dst, dst, 1460, NULL, 0, i, key); return 1460; });
  RunOneBenchmark("chacha20-decrypt", [&](size_t i) -> uint64 { chacha20poly1305_decrypt_get_mac(dst, dst, 1460, NULL, 0, i, key, mac); return 1460; });

  RunOneBenchmark("poly1305-only", [&](size_t i) -> uint64 { poly1305_get_mac(dst, 1460, NULL, 0, i, key, mac); return 1460; });

#if WITH_AESGCM
  if (X86_PCAP_AES) {
    AesGcm128StaticContext sctx;
    CRYPTO_gcm128_init(&sctx, key, 128);

    RunOneBenchmark("aes128-gcm-encrypt", [&](size_t i) -> uint64 { aesgcm_encrypt(dst, dst, 1460, NULL, 0, i, &sctx); return 1460; });
    RunOneBenchmark("aes128-gcm-decrypt", [&](size_t i) -> uint64 { aesgcm_decrypt_get_mac(dst, dst, 1460, NULL, 0, i, &sctx, mac); return 1460; });
  }
#endif   //  WITH_AESGCM
}
