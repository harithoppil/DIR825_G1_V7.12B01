#1209 ���vendor���ٱ���ѡ��
QuickTarget=$(OpenSourceTarget) $(AutoTarget) filesystem $(VendorTarget)
QuickCleanTarget=$(patsubst %,%_clean,$(QuickTarget))
QuickDistCleanTarget=$(patsubst %,%_distclean,$(QuickTarget))
QuickInstallTarget=$(patsubst %,%_install,$(QuickTarget))

AutoTarget=
#�Զ�����Ŀ�����ӹ���
#�Զ�����Ŀ����Ϊ�˷�����ӱ���Ŀ��
#��ӹ���
#1.ȷ��Ŀ������XXX��
#2.��ӵ�AutoTarget.Ex:AutoTarget+=XXX
#3.ָ��Ŀ��·������ֵ��XXX_path.Ex:XXX_path=$(BUILDDIR)/apps/XXX 

AutoTarget+=scripts boot opensource 
-include $(ROOTDIR)/tbs_sdk_config
ifeq ($(TBS_SDK),y)		
TbsSdkTarget += apps ssap logic web html tr069fe tr064fe
ifdef  CONFIG_APPS_LOGIC_NEW3G
TbsSdkTarget += new3g
endif

else
AutoTarget += apps ssap logic web html tr069fe tr064fe
ifdef  CONFIG_APPS_LOGIC_NEW3G
AutoTarget+= new3g
endif

endif


scripts_path=./scripts
boot_path=$(BUILDDIR)/boot
opensource_path=$(BUILDDIR)/opensource
apps_path=$(BUILDDIR)/apps
ssap_path=$(BUILDDIR)/apps/ssap
logic_path=$(BUILDDIR)/apps/logic
web_path=$(BUILDDIR)/apps/web
html_path=$(BUILDDIR)/apps/html
tr069fe_path=$(BUILDDIR)/apps/tr069fe
tr064fe_path=$(BUILDDIR)/apps/tr064fe

#scb+ 2011-12-27
ifdef  CONFIG_APPS_LOGIC_NEW3G
new3g_path=$(BUILDDIR)/apps/logic/new3g
endif


AutoTarget+=kernel driver dsl wireless vendor
kernel_path=$(BUILDDIR)/kernel
driver_path=$(BUILDDIR)/driver
dsl_path=$(BUILDDIR)/driver/dsl
wireless_path=$(BUILDDIR)/driver/wireless
vendor_path=$(BUILDDIR)/vendor

############## Open Source Quick Targets #############################`########
OpenSourcePath=build/opensource/
OpenSourceTarget=$(patsubst $(OpenSourcePath)%,%,$(shell find $(OpenSourcePath) -maxdepth 1 -type d 2>/dev/null))

$(OpenSourceTarget):
	make -C $(OpenSourcePath)$@ all 
$(patsubst %,%_distclean,$(OpenSourceTarget)):
	make -C $(OpenSourcePath)$(patsubst %_distclean,%,$@) distclean
$(patsubst %,%_clean,$(OpenSourceTarget)):
	make -C $(OpenSourcePath)$(patsubst %_clean,%,$@) clean
$(patsubst %,%_install,$(OpenSourceTarget)):
	make -C $(OpenSourcePath)$(patsubst %_install,%,$@) install

############## Vendor Quick Targets #############################`########
VendorPath=build/vendor/
VendorTarget=$(patsubst $(VendorPath)%,%,$(shell find $(VendorPath) -maxdepth 1 -type d 2>/dev/null))

$(VendorTarget):
	make -C $(VendorPath)$@ 
$(patsubst %,%_distclean,$(VendorTarget)):
	make -C $(VendorPath)$(patsubst %_distclean,%,$@) distclean
$(patsubst %,%_clean,$(VendorTarget)):
	make -C $(VendorPath)$(patsubst %_clean,%,$@) clean
$(patsubst %,%_install,$(VendorTarget)):
	make -C $(VendorPath)$(patsubst %_install,%,$@) install

############## AutoTarget ####################################
$(AutoTarget):
	make -C $($@_path) 
$(patsubst %,%_distclean,$(AutoTarget)):
	make -C $($(patsubst %_distclean,%_path,$@)) distclean 
$(patsubst %,%_clean,$(AutoTarget)):
	make -C $($(patsubst %_clean,%_path,$@)) clean 
$(patsubst %,%_install,$(AutoTarget)):
	make -C $($(patsubst %_install,%_path,$@)) install 
$(patsubst %,%_jsupdate,$(AutoTarget)):
	make -C $($(patsubst %_jsupdate,%_path,$@)) jsupdate 

############## TbsSdkTarget ####################################
ifeq ($(TBS_SDK),y)
$(TbsSdkTarget):
	make -C $($@_path) 
$(patsubst %,%_distclean,$(TbsSdkTarget)):
	@echo "APPS DISTCLEAN OK!"
$(patsubst %,%_clean,$(TbsSdkTarget)):
	@echo "APPS CLEAN OK!"
$(patsubst %,%_install,$(TbsSdkTarget)):
	make -C $($(patsubst %_install,%_path,$@)) install 
$(patsubst %,%_jsupdate,$(TbsSdkTarget)):
	make -C $($(patsubst %_jsupdate,%_path,$@)) jsupdate 
endif

############## Filesystem Source Quick Targets #####################################
FilesystemSourcePath=build/filesystem
filesystem:
filesystem_distclean:
filesystem_clean:
filesystem_install:
	make -C $(FilesystemSourcePath) install




