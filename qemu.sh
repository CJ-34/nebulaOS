#!/bin/sh
set -e
. ./iso.sh
. ./config.sh
HOST=$(./default-host.sh)
qemu-system-$(./target-triplet-to-arch.sh $HOST) -cdrom $BUILDDIR/nebula.iso -serial stdio
