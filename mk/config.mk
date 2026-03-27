DEFAULT_HOST := $(shell $(ROOTDIR)/default-host.sh)
HOST ?= $(DEFAULT_HOST)
HOSTARCH := $(shell $(ROOTDIR)/target-triplet-to-arch.sh $(HOST))

AR := $(HOST)-ar
AS := $(HOST)-as
CC := $(HOST)-gcc

SYSROOT ?= $(ROOTDIR)/sysroot
SYSROOT_CPPFLAGS := --sysroot=$(SYSROOT) -isystem $(SYSROOT)/usr/include
SYSROOT_LDFLAGS := --sysroot=$(SYSROOT) -isystem $(SYSROOT)/usr/lib

DESTDIR ?=
PREFIX ?= /usr
EXEC_PREFIX ?= $(PREFIX)
BOOTDIR ?= /boot
INCLUDEDIR ?= $(PREFIX)/include
LIBDIR ?= $(EXEC_PREFIX)/lib

CFLAGS ?= -O2 -g
CPPFLAGS ?=
LDFLAGS ?=
LIBS ?=
