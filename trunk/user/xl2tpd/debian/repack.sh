#!/bin/sh
# Repackage upstream source to exclude non-distributable files
# should be called as "repack sh --upstream-source <ver> <downloaded file>
# (for example, via uscan)

set -e
set -u

FILE=$3
PKG=`dpkg-parsechangelog|grep ^Source:|sed 's/^Source: //'`
VER="$2+dfsg"

printf "\nRepackaging $FILE\n"

DIR=`mktemp -d ./tmpRepackXXXXXX`
trap "rm -rf $DIR" QUIT INT EXIT

tar xzf $FILE -C $DIR

TARGET=`echo $FILE |sed 's/_\(.*\)\.orig/_\1+dfsg.orig/'`
REPACK=`basename $TARGET`

UP_DIR=`ls -1 $DIR`

(
    set -e
    set -u

    cd $DIR

    rm -rv $UP_DIR/debian/

    REPACK_DIR="$PKG-$VER.orig"
    mv $UP_DIR $REPACK_DIR
    tar -c $REPACK_DIR | gzip -9 > $REPACK
)

rm -v $FILE
mv $DIR/$REPACK $TARGET

echo "*** $FILE repackaged as $TARGET"
