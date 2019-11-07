#!/bin/sh
perl poly1305-x64.pl gas    > poly1305-x64-linux.s
perl poly1305-x64.pl macosx > poly1305-x64-osx.s
perl poly1305-arm64.pl ios  > poly1305-arm64-ios.S


