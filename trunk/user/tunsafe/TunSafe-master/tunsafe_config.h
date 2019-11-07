// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#pragma once

#define TUNSAFE_VERSION_STRING "TunSafe 1.5-rc2"
#define TUNSAFE_VERSION_STRING_LONG "TunSafe 1.5-rc2"

// Enable support for handshake extensions
#define WITH_HANDSHAKE_EXT 1

// Whether to enable the boolean features functionality
#define WITH_BOOLEAN_FEATURES 1 

// Enable support for header obfuscation
#define WITH_HEADER_OBFUSCATION 1

// Enable support for two-factor authentication (requires WITH_HANDSHAKE_EXT)
#define WITH_TWO_FACTOR_AUTHENTICATION 1

// Whether to enable the short MAC feature, that uses an 8-byte MAC instead of 16-byte (Saves overhead)
#define WITH_SHORT_MAC 0

// Enable support for the keypair->compress_handler_ feature
#define WITH_PACKET_COMPRESSION 0
 
// Enable support for short (down to 2 byte headers) instead of 16 bytes
#define WITH_SHORT_HEADERS 0

// Enable support for alternative cipher suites
#define WITH_CIPHER_SUITES 0

#define WITH_AVX512_OPTIMIZATIONS 0
#define WITH_BENCHMARK 0

// Use bytell hashmap instead. Only works in 64-bit builds
#define WITH_BYTELL_HASHMAP 0
