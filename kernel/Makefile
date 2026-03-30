# --------------------------------------------------
# Shared configuration
# --------------------------------------------------
ROOTDIR := $(abspath ..)
include $(ROOTDIR)/mk/config.mk

# --------------------------------------------------
# Kernel-specific directories
# --------------------------------------------------
ARCHDIR := arch/$(HOSTARCH)
BUILDDIR := $(ROOTDIR)/build
TARGET := $(ROOTDIR)/build/nebula.kernel

# --------------------------------------------------
# Kernel-specific flags
# --------------------------------------------------
CFLAGS := $(CFLAGS) -ffreestanding -Wall -Wextra
CPPFLAGS := $(CPPFLAGS) $(SYSROOT_CPPFLAGS) -D__is_kernel -Iinclude -Iarch 
LDFLAGS := $(LDFLAGS) $(SYSROOT_LDFLAGS)
LIBS := $(LIBS) -nostdlib -lk -lgcc

# Pull in arch-specific settings
include $(ARCHDIR)/make.config

CFLAGS := $(CFLAGS) $(KERNEL_ARCH_CFLAGS)
CPPFLAGS := $(CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)
LDFLAGS := $(LDFLAGS) $(KERNEL_ARCH_LDFLAGS)
LIBS := $(LIBS) $(KERNEL_ARCH_LIBS)

# --------------------------------------------------
# Source discovery
# --------------------------------------------------
KERNEL_C_SRCS := $(shell find kernel -type f -name '*.c')
KERNEL_S_SRCS := $(shell find kernel -type f -name '*.S')

ARCH_C_SRCS := $(shell find $(ARCHDIR) -type f -name '*.c')
ARCH_S_SRCS := $(shell find $(ARCHDIR) -type f -name '*.S')

ARCH_SPECIAL_OBJS := \
	$(BUILDDIR)/$(ARCHDIR)/crti.o \
	$(BUILDDIR)/$(ARCHDIR)/crtbegin.o \
	$(BUILDDIR)/$(ARCHDIR)/crtend.o \
	$(BUILDDIR)/$(ARCHDIR)/crtn.o

ALL_C_SRCS := $(KERNEL_C_SRCS) $(ARCH_C_SRCS)
ALL_S_SRCS := $(KERNEL_S_SRCS) $(ARCH_S_SRCS)

C_OBJS := $(patsubst %.c,$(BUILDDIR)/%.o,$(ALL_C_SRCS))
S_OBJS := $(patsubst %.S,$(BUILDDIR)/%.o,$(ALL_S_SRCS))

OBJS := $(filter-out $(ARCH_SPECIAL_OBJS),$(C_OBJS) $(S_OBJS))

LINK_OBJS := \
	$(BUILDDIR)/$(ARCHDIR)/crti.o \
	$(BUILDDIR)/$(ARCHDIR)/crtbegin.o \
	$(OBJS) \
	$(BUILDDIR)/$(ARCHDIR)/crtend.o \
	$(BUILDDIR)/$(ARCHDIR)/crtn.o

DEPS := $(OBJS:.o=.d)

# --------------------------------------------------
# Rules
# --------------------------------------------------
.PHONY: all clean install install-headers install-kernel

all: $(TARGET)

$(TARGET): $(LINK_OBJS) $(ARCHDIR)/linker.ld
	$(CC) -T $(ARCHDIR)/linker.ld -o $@ $(CFLAGS) $(LINK_OBJS) $(LDFLAGS) $(LIBS)
	grub-file --is-x86-multiboot $@

$(BUILDDIR)/$(ARCHDIR)/crtbegin.o:
	@mkdir -p $(dir $@)
	cp "$$($(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=crtbegin.o)" $@

$(BUILDDIR)/$(ARCHDIR)/crtend.o:
	@mkdir -p $(dir $@)
	cp "$$($(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=crtend.o)" $@

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -MMD -MP -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)

$(BUILDDIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(CC) -MMD -MP -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

clean:
	rm -rf $(BUILDDIR) $(TARGET)

install: install-headers install-kernel

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/

install-kernel: $(TARGET)
	mkdir -p $(DESTDIR)$(BOOTDIR)
	cp $(TARGET) $(DESTDIR)$(BOOTDIR)/

-include $(DEPS)
