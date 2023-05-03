
TARGET_CPPFLAGS:=-I$(TMPDISTDIR)/include
TARGET_LDFLAGS:=-L$(TMPDISTDIR)/lib
TARGET_CFLAGS=-Os -pipe -funit-at-a-time

EXTRA_CPPFLAGS := -I$(TMPDISTDIR)/include
EXTRA_CFLAGS := $(EXTRA_CPPFLAGS)
EXTRA_LDFLAGS := -L$(TMPDISTDIR)/lib

GNU_TARGET_NAME:=$(TARGET)
GNU_HOST_NAME:=i386-redhat-linux
TARGET_CROSS:=$(CROSS_COMPILE)

ifeq ($(CONFIG_ENABLE_LOCALE),true)
  DISABLE_NLS:=
else
  DISABLE_NLS:=--disable-nls
endif

#ifneq ($(CONFIG_LARGEFILE),y)
#  DISABLE_LARGEFILE= --disable-largefile
#endif

CONFIGURE_ARGS = \
		--target=$(GNU_TARGET_NAME) \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--program-prefix="" \
		--program-suffix="" \
		--prefix=/usr \
		--exec-prefix=/usr \
		--bindir=/usr/bin \
		--sbindir=/usr/sbin \
		--libexecdir=/usr/lib \
		--sysconfdir=/etc \
		--datadir=/usr/share \
		--localstatedir=/var \
		--mandir=/usr/man \
		--infodir=/usr/info \
		$(DISABLE_NLS)

TARGET_CONFIGURE_OPTS:= \
  AR=$(CROSS_COMPILE)ar \
  AS="$(CC) -c $(TARGET_CFLAGS)" \
  LD=$(CROSS_COMPILE)ld \
  NM=$(CROSS_COMPILE)nm \
  CC="$(CC)" \
  GCC="$(CC)" \
  CXX=$(CROSS_COMPILE)g++ \
  RANLIB=$(CROSS_COMPILE)ranlib \
  STRIP=$(CROSS_COMPILE)strip \
  OBJCOPY=$(CROSS_COMPILE)objcopy \
  OBJDUMP=$(CROSS_COMPILE)objdump \
  SIZE=$(CROSS_COMPILE)size


CONFIGURE_VARS = \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS)" \
		CXXFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS)" \
		CPPFLAGS="$(TARGET_CPPFLAGS) $(EXTRA_CPPFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		PKG_CONFIG_PATH="$(TMPDISTDIR)/lib/pkgconfig" \
		PKG_CONFIG_LIBDIR="$(TMPDISTDIR)/lib/pkgconfig"

CONFIGURE_PATH = .
CONFIGURE_CMD = ./configure

define Build/Configure/Default
	(cd $(PKG_BUILD_DIR)/$(CONFIGURE_PATH)/$(strip $(3)); \
	if [ -x $(CONFIGURE_CMD) ]; then \
		$(CONFIGURE_VARS) \
		$(2) \
		$(CONFIGURE_CMD) \
		$(CONFIGURE_ARGS) \
		$(1); \
	fi; \
	)
endef

MAKE_VARS = \
	CFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS)" \
	CXXFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS)" \
	LDFLAGS="$(EXTRA_LDFLAGS) "

MAKE_FLAGS = \
	$(TARGET_CONFIGURE_OPTS) \
	CROSS="$(TARGET_CROSS)" \
	ARCH="$(ARCH)"

MAKE_PATH = .

define Build/Compile/Default
	$(MAKE_VARS) \
	$(MAKE) -C $(PKG_BUILD_DIR)/$(MAKE_PATH) \
		$(MAKE_FLAGS) \
		$(1);
endef

##################################################
# For install
##################################################
ifndef KERNELRELEASE
VERSION := $(strip $(shell head -n 1 ${KERNEL_DIR}/Makefile | cut -f 2 -d'='))
PATCHLEVEL := $(strip $(shell head -n 2 ${KERNEL_DIR}/Makefile | tail -1 | cut -f 2 -d'='))
SUBVERSION := $(strip $(shell head -n 3 ${KERNEL_DIR}/Makefile | tail -1 | cut -f 2 -d'='))
EXTRAVERSION ?= $(strip $(shell head -n 4 ${KERNEL_DIR}/Makefile | tail -1 | cut -f 2 -d'='))
KERNELRELEASE := $(strip ${VERSION}.${PATCHLEVEL}.${SUBVERSION}${EXTRAVERSION})
endif

export LIBDIR:=lib
export BINDIR:=usr/sbin
export MODULEPATH:=lib/modules/${KERNELRELEASE}
export PATH_ROOTFS_LIB:=$(FSROOT)/$(LIBDIR)
export PATH_ROOTFS_MOD:=$(FSROOT)/$(MODULEPATH)
export PATH_ROOTFS_RC:=$(FSROOT)/etc/init.d
export PATH_ROOTFS_BIN:=$(FSROOT)/$(BINDIR)

export INSTALL_BIN:=install -m0755
export INSTALL_DIR:=install -d -m0755
export INSTALL_DATA:=install -m0644
export INSTALL_CONF:=install -m0600