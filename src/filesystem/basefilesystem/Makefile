include $(ROOTDIR)/.config
include $(ROOTDIR)/BaseVar.mk
EXTRA_CFLAGS=
-include $(ROOTDIR)/build/arch.mk
ifeq (${ENDIANNESS}, big)
	EXTRA_CFLAGS =-BYTE_NEED_CVT
else
	EXTRA_CFLAGS =-BYTE_NEED_CVT_err
endif

.PHONY: all install distclean clean
CONFIG_OPENSOURCE_SAMBA=y

PUDT_INIT = $(ROOTDIR)/product/$(PRODUCT)/init.d
all:

install:
	$(Q)cp -rf $(ROOTDIR)/src/filesystem/basefilesystem/root/* $(FSROOT)/
	$(Q)find $(FSROOT) -name .svn | xargs rm -rf
	$(Q)rm -f $(FSROOT)/etc/init.d/README
	$(Q)cp -f $(BUILDDIR)/product/config.xml $(FSROOT)/etc/config.xml
	$(Q)cp -f $(BUILDDIR)/product/config_flash.xml $(FSROOT)/etc/config_flash.xml
	if [ -f $(ROOTDIR)/product/$(PRODUCT)/config_flash.xml ]; then \
		cp -f $(ROOTDIR)/product/$(PRODUCT)/config_flash.xml $(FSROOT)/../config_flash.xml;\
	fi;

	if [ -f $(ROOTDIR)/product/$(PRODUCT)/config_flash.xml ]; then \
		cp $(ROOTDIR)/tools/bin/xml_flash_tool ./ ; \
		./xml_flash_tool $(FSROOT)/../config_flash.xml $(FSROOT)/../tbs_app_cfg $(EXTRA_CFLAGS) ; \
		rm -f ./xml_flash_tool ; \
	else \
		echo "xxxxxxxxxxxxx" > $(FSROOT)/../tbs_app_cfg ; \
	fi;
	if [ "${ARCH}" = "um" ]; then  \
		sed -e "s/ttyS0/tty0/g" root/etc/inittab > $(FSROOT)/etc/inittab; \
	fi;
	if [ "${CPU}" = "rt3052" ]; then  \
		sed -e "s/ttyS0/ttyS1/g" root/etc/inittab > $(FSROOT)/etc/inittab; \
	fi;
	if [ "${CONFIG_OPENSOURCE_SAMBA}" = "y" ]; then \
	  echo "export SAMBA=yes" >> $(FSROOT)/etc/init.d/env.rc; \
	fi;
	if [ "${CONFIG_OPENSOURCE_HTTPD_SSL}" = "y" ]; then \
	  echo "export HTTPS=yes" >> $(FSROOT)/etc/init.d/env.rc; \
	fi;
	if [ "${CONFIG_APPS_LOGIC_USB_MASS}" = "y"]; then \
	  echo "export USB=yes" >> $(FSROOT)/etc/init.d/env.rc; \
	fi; 
	if [ "${CONFIG_TBS_SUPPORT_IPV6}" = "y" ]; then \
	  sh ${ROOTDIR}/product/${PRODUCT}/ip6conf.sh; \
	fi;
	-install -c -m 755 $(PUDT_INIT)/* $(FSROOT)/etc/init.d;
	if [ "${CONFIG_MT7530_GE_SWITCH_CHIP}" != "y" ] && [ "${CONFIG_SITECOM_WLR_AC750}" = "y" ]; then \
		sed -e 's/config-vlan.sh 4 LLLLW/config-vlan.sh 3 LLLLW/g' $(PUDT_INIT)/mtk_vlan.rc > $(FSROOT)/etc/init.d/mtk_vlan.rc; \
	fi;
	-install -c -m 755 $(PUDT_INIT)/modules.d/* $(FSROOT)/etc/init.d/modules.d/;
	-dos2unix $(FSROOT)/etc/init.d/* 2>/dev/null 1>&2
	./mkdev.sh
	$(Q)echo "install base file system ok!"

distclean:

clean:
