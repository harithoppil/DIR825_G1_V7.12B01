include $(ROOTDIR)/.config
export MODULEPATH:=lib/modules
#export MODULEPATH:=lib/modules/${KERNELRELEASE}
#export PATH_ROOTFS_LIB:=$(FSROOT)/$(LIBDIR)
export PATH_ROOTFS_MOD:=$(FSROOT)/$(MODULEPATH)

ifeq ($(strip $(CONFIG_SITECOM_AUTOUPGRADE)), y)
SITECOM_TARGET +=autoFWupgrade
endif
ifeq ($(strip $(CONFIG_SITECOM_UTPROXY)), y)
SITECOM_TARGET +=utmproxy
SITECOM_TARGET_MODULE += utm_nat_nf_module.ko
endif
ifeq ($(strip $(CONFIG_SITECOM_WLR_AC750)), y)
SITECOM_TARGET_KO_MOD +=GPL_NetUSB.ko
SITECOM_TARGET_KO_MOD +=NetUSB.ko
endif

all:
install:
	mkdir -p $(PATH_ROOTFS_MOD)/driver/
	cp ${SITECOM_TARGET} $(FSROOT)/sbin/
ifeq ($(strip $(CONFIG_SITECOM_UTPROXY)), y)
	${Q}cp -f -L $(SITECOM_TARGET_MODULE)      \
  $(PATH_ROOTFS_MOD)/driver/$(SITECOM_TARGET_MODULE)
endif
ifeq ($(strip $(CONFIG_SITECOM_WLR_AC750)), y)
	${Q}cp -f -L $(SITECOM_TARGET_KO_MOD)      \
  $(PATH_ROOTFS_MOD)/driver/
endif
clean:
	
