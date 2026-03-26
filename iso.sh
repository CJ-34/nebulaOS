#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/nebula.kernel isodir/boot/nebula.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "nebula" {
	multiboot /boot/nebula.kernel
}
EOF

grub-mkrescue -o nebula.iso isodir
