#!/bin/sh
perl chacha20-x64.pl gas    > chacha20-x64-linux.s
perl chacha20-x64.pl macosx > chacha20-x64-osx.s
perl chacha20-arm64.pl ios  > chacha20-arm64-ios.S