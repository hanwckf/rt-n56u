set -e

clang++ -c -mavx512f -mavx512vl crypto/poly1305/poly1305-x64-osx.s crypto/chacha20/chacha20-x64-osx.s 

clang++ -g -O3 -I . -std=c++11 -DWITH_NETWORK_BSD=1 -DNDEBUG=1 -Wno-deprecated-declarations -fno-exceptions -fno-rtti -ffunction-sections -o tunsafe \
tunsafe_amalgam.cpp \
crypto/aesgcm/aesni_gcm-x64-osx.s \
crypto/aesgcm/aesni-x64-osx.s \
crypto/aesgcm/ghash-x64-osx.s \
chacha20-x64-osx.o \
poly1305-x64-osx.o

cp tunsafe tunsafe.unstripped
strip tunsafe
rm -f tunsafe_osx.zip
zip tunsafe_osx.zip tunsafe readme_osx.txt

