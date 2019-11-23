g++7 -I . -O2 -DNDEBUG -DWITH_NETWORK_BSD=1 -static -mssse3 -o tunsafe \
tunsafe_amalgam.cpp \
crypto/chacha20/chacha20-x64-linux.s \
crypto/poly1305/poly1305-x64-linux.s \
-lrt -pthread
