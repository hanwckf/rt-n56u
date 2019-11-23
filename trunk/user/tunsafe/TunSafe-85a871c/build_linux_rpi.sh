#!/bin/sh

set -e
#cpp -D__ARM_ARCH__=7 crypto/chacha20/chacha20-arm.s > crypto/chacha20/chacha20-arm.preprocessed.s
#cpp -D__ARM_ARCH__=7 crypto/poly1305/poly1305-arm.s > crypto/poly1305/poly1305-arm.preprocessed.s

g++-6 -mfpu=neon  -I . -g -O2 -DNDEBUG -DWITH_NETWORK_BSD=1 -fno-omit-frame-pointer -march=armv7-a -mthumb -std=c++11 -pthread -lrt -o tunsafe \
tunsafe_amalgam.cpp \
crypto/chacha20/chacha20-arm-linux.S \
crypto/poly1305/poly1305-arm-linux.S \
