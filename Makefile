ROOTDIR := $(abspath .)
include mk/config.mk

export ROOTDIR HOST HOSTARCH AR AS CC SYSROOT SYSROOT_CPPFLAGS SYSROOT_LDFLAGS
export DESTDIR PREFIX EXEC_PREFIX BOOTDIR INCLUDEDIR LIBDIR
export CFLAGS CPPFLAGS LDFLAGS LIBS

.PHONY: all headers libc kernel clean

all: headers libc kernel

headers:
	$(MAKE) -C libc install-headers DESTDIR=$(SYSROOT)
	$(MAKE) -C kernel install-headers DESTDIR=$(SYSROOT)

libc: headers
	$(MAKE) -C libc install DESTDIR=$(SYSROOT)

kernel: headers libc
	$(MAKE) -C kernel install DESTDIR=$(SYSROOT)

clean:
	$(MAKE) -C libc clean
	$(MAKE) -C kernel clean
	rm -rf $(SYSROOT)
