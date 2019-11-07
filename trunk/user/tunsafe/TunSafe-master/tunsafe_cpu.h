// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#ifndef TUNSAFE_CPU_H_
#define TUNSAFE_CPU_H_

#include "tunsafe_types.h"


#if defined(ARCH_CPU_X86_FAMILY)

extern uint32 x86_pcap[3];

// cpuid 1, edx
#define X86_PCAP_SSE (x86_pcap[0] & (1 << 25))
#define X86_PCAP_SSE2 (x86_pcap[0] & (1 << 26))
// cpuid 1, ecx
#define X86_PCAP_SSE3 (x86_pcap[1] & (1 << 0))
#define X86_PCAP_PCLMULQDQ (x86_pcap[1] & (1 << 0))
#define X86_PCAP_SSSE3 (x86_pcap[1] & (1 << 9))
#define X86_PCAP_MOVBE (x86_pcap[1] & (1 << 22))
#define X86_PCAP_AES (x86_pcap[1] & (1 << 25))
#define X86_PCAP_AVX (x86_pcap[1] & (1 << 28))
// cpuid 7, ebx
#define X86_PCAP_AVX2 (x86_pcap[2] & (1 << 5))
#define X86_PCAP_AVX512F (x86_pcap[2] & (1 << 16))
#define X86_PCAP_AVX512VL (x86_pcap[2] & (1 << 31))

#endif  // defined(ARCH_CPU_X86_FAMILY)


#if defined(ARCH_CPU_ARM_FAMILY)

extern uint32 arm_pcap[1];

#define ARM_PCAP_NEON (arm_pcap[0] & (1 << 0))

#endif  // defined(ARCH_CPU_ARM_FAMILY)

void InitCpuFeatures();
void PrintCpuFeatures();


#endif  // TUNSAFE_CPU_H_
