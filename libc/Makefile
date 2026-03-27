# --------------------------------------------------
# Shared configuration
# --------------------------------------------------
ROOTDIR := $(abspath ..)
include $(ROOTDIR)/mk/config.mk

# --------------------------------------------------
# libc-specific directories
# --------------------------------------------------
ARCHDIR := arch/$(HOSTARCH)
BUILDDIR := $(ROOTDIR)/build

# --------------------------------------------------
# libc-specific flags
# --------------------------------------------------
CFLAGS := $(CFLAGS) -ffreestanding -Wall -Wextra
CPPFLAGS := $(CPPFLAGS) $(SYSROOT_CPPFLAGS) -D__is_libc -Iinclude

LIBK_CFLAGS := $(CFLAGS)
LIBK_CPPFLAGS := $(CPPFLAGS) -D__is_libk

include $(ARCHDIR)/make.config

CFLAGS := $(CFLAGS) $(ARCH_CFLAGS)
CPPFLAGS := $(CPPFLAGS) $(ARCH_CPPFLAGS)

LIBK_CFLAGS := $(LIBK_CFLAGS) $(KERNEL_ARCH_CFLAGS)
LIBK_CPPFLAGS := $(LIBK_CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)

# --------------------------------------------------
# Source discovery
# --------------------------------------------------
FREE_C_SRCS := $(shell find stdio stdlib string -type f -name '*.c')
FREE_S_SRCS := $(shell find stdio stdlib string -type f -name '*.S' 2>/dev/null)

FREE_C_SRCS += $(ARCH_FREE_C_SRCS)
FREE_S_SRCS += $(ARCH_FREE_S_SRCS)

HOSTED_C_SRCS := $(ARCH_HOSTED_C_SRCS)
HOSTED_S_SRCS := $(ARCH_HOSTED_S_SRCS)

ALL_C_SRCS := $(FREE_C_SRCS) $(HOSTED_C_SRCS)
ALL_S_SRCS := $(FREE_S_SRCS) $(HOSTED_S_SRCS)

# --------------------------------------------------
# Object mapping
# --------------------------------------------------
FREE_OBJS := $(patsubst %.c,$(BUILDDIR)/%.o,$(FREE_C_SRCS))
FREE_OBJS += $(patsubst %.S,$(BUILDDIR)/%.o,$(FREE_S_SRCS))

HOSTED_OBJS := $(patsubst %.c,$(BUILDDIR)/%.o,$(HOSTED_C_SRCS))
HOSTED_OBJS += $(patsubst %.S,$(BUILDDIR)/%.o,$(HOSTED_S_SRCS))

OBJS := $(FREE_OBJS) $(HOSTED_OBJS)

LIBK_OBJS := $(patsubst $(BUILDDIR)/%.o,$(BUILDDIR)/%.libk.o,$(FREE_OBJS))

DEPS := $(OBJS:.o=.d) $(LIBK_OBJS:.o=.d)

BINARIES := libk.a
# BINARIES := libc.a libk.a

.PHONY: all clean install install-headers install-libs

all: $(BINARIES)

libc.a: $(OBJS)
	$(AR) rcs $@ $^

libk.a: $(LIBK_OBJS)
	$(AR) rcs $@ $^

# --------------------------------------------------
# Normal libc objects
# --------------------------------------------------
$(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -MMD -MP -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)

$(BUILDDIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(CC) -MMD -MP -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

# --------------------------------------------------
# libk objects
# --------------------------------------------------
$(BUILDDIR)/%.libk.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -MMD -MP -c $< -o $@ -std=gnu11 $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

$(BUILDDIR)/%.libk.o: %.S
	@mkdir -p $(dir $@)
	$(CC) -MMD -MP -c $< -o $@ $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

clean:
	rm -rf $(BUILDDIR) libc.a libk.a *.a

install: install-headers install-libs

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/

install-libs: $(BINARIES)
	mkdir -p $(DESTDIR)$(LIBDIR)
	cp $(BINARIES) $(DESTDIR)$(LIBDIR)

-include $(DEPS)
