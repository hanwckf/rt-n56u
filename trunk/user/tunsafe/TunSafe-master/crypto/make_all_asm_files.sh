#!/bin/sh

set -e

# macos
perl make_chacha20_x64.pl macosx > chacha20_x64_gas_macosx.s
perl make_poly1305_x64.pl macosx > poly1305_x64_gas_macosx.s

cd aesgcm

perl aesni-gcm-x86_64.pl macosx > aesni_gcm_x64_gas_macosx.s
perl aesni-x86_64.pl macosx > aesni_x64_gas_macosx.s
perl ghash-x86_64.pl macosx > ghash_x64_gas_macosx.s

cd ..


# linux,freebsd
perl make_chacha20_x64.pl gas > chacha20_x64_gas.s
perl make_poly1305_x64.pl gas > poly1305_x64_gas.s

cd aesgcm

perl aesni-gcm-x86_64.pl gas > aesni_gcm_x64_gas.s
perl aesni-x86_64.pl gas > aesni_x64_gas.s
perl ghash-x86_64.pl gas > ghash_x64_gas.s

cd ..
