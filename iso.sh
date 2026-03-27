#!/bin/sh
set -eu
make all
. ./config.sh

ISO_ROOT="$BUILDDIR/isodir"
ISO_IMAGE="$BUILDDIR/nebula.iso"
KERNEL_IMAGE="$SYSROOT$BOOTDIR/nebula.kernel"
GRUB_DIR="$ISO_ROOT/boot/grub"
GRUB_CFG="$GRUB_DIR/grub.cfg"

echo "==> Preparing ISO directory"
rm -rf "$ISO_ROOT"
mkdir -p "$GRUB_DIR"

echo "==> Checking kernel"
if [ ! -f "$KERNEL_IMAGE" ]; then
	echo "Kernel not found: $KERNEL_IMAGE" >&2
	exit 1
fi

echo "==> Copying kernel"
cp "$KERNEL_IMAGE" "$ISO_ROOT/boot/nebula.kernel"

echo "==> Generating grub.cfg"
cat > "$GRUB_CFG" <<EOF
set timeout=0
set default=0

menuentry "Nebula OS" {
	multiboot /boot/nebula.kernel
	boot
}
EOF

echo "==> Creating ISO"
mkdir -p "$BUILDDIR"
grub-mkrescue -o "$ISO_IMAGE" "$ISO_ROOT"

echo "==> ISO created at: $ISO_IMAGE"
