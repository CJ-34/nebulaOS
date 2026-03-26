#!/bin/sh
set -e 
. ./headers.sh

for PROJECT in $PROJECT; do
	(cd $PROJECT && DESTDIR="$SYSROOT" $MAKE install)
done
