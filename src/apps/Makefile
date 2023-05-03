ifeq "$(ROOTDIR)" ""
export ROOTDIR=$(shell while true; do if [ -f BaseVar.mk ]; then pwd;exit; else cd ..;fi;done;)
endif

include $(ROOTDIR)/.config

#subdir-y += ssap
#subdir-y += logic
#subdir-${CONFIG_APPS_CLI} += cli
#subdir-${CONFIG_APPS_LIB} += omlib
#subdir-${CONFIG_APPS_TR069} += tr069fe
#subdir-${CONFIG_APPS_WEB} += web
subdir-${CONFIG_APPS_HTML} += html
#subdir-${CONFIG_APPS_TM} += tm
#subdir-${CONFIG_APPS_SUPP} += supp
#subdir-${CONFIG_APPS_IPV6} += ipv6
#subdir-${CONFIG_APPS_CTC_UPnP_DM_FE} += CTC_UPnP_DM_FE
#subdir-y += upgcgi

include $(ROOTDIR)/Rules.make

installme::
	
#ssap
	$(Q)cp $(ROOTDIR)/src/apps/ssap/lib/libssap.so $(FSROOT)/lib/

	$(Q)cp $(ROOTDIR)/src/apps/ssap/pc/pc $(FSROOT)/usr/bin/
	$(Q)$(STRIP) $(FSROOT)/usr/bin/pc
	
	$(Q)cp $(ROOTDIR)/src/apps/ipv6/ramon $(FSROOT)/usr/bin/
	$(Q)$(STRIP) $(FSROOT)/usr/bin/ramon
	
	$(Q)cp $(ROOTDIR)/src/apps/ipv6/ip6mon $(FSROOT)/usr/bin/
	$(Q)$(STRIP) $(FSROOT)/usr/bin/ip6mon
	
	$(Q)cp $(ROOTDIR)/src/apps/ipv6/ip6aac $(FSROOT)/usr/bin/
	$(Q)$(STRIP) $(FSROOT)/usr/bin/ip6aac

	$(Q)cp $(ROOTDIR)/src/apps/ssap/flash_test/flash_test $(FSROOT)/usr/bin/
	$(Q)$(STRIP) $(FSROOT)/usr/bin/flash_test


ifeq ("$(CONFIG_APPS_SSAP_UPGRADE)","y")
	$(Q)cp $(ROOTDIR)/src/apps/ssap/upg/upgrader $(FSROOT)/usr/bin/
	$(Q)$(STRIP) $(FSROOT)/usr/bin/upgrader
endif

ifeq ("$(CONFIG_APPS_SSAP_TFTPUPG)","y")
	$(Q)cp $(ROOTDIR)/src/apps/ssap/tftpu/tftpd $(FSROOT)/usr/bin/
	$(Q)$(STRIP) $(FSROOT)/usr/bin/tftpd
endif


ifeq ("$(CONFIG_APPS_SSAP_PROTEST)","y")
	$(Q)cp $(ROOTDIR)/src/apps/ssap/pti/protest $(FSROOT)/usr/bin/
	$(Q)$(STRIP) $(FSROOT)/usr/bin/protest
endif

ifeq ("$(CONFIG_APPS_SSAP_GETSMAPS)","y")
	$(Q)cp $(ROOTDIR)/src/apps/ssap/getsmaps/getsmaps $(FSROOT)/usr/bin/
	$(Q)$(STRIP) $(FSROOT)/usr/bin/getsmaps
endif

#logic
	$(Q)cp $(ROOTDIR)/src/apps/logic/logic $(FSROOT)/usr/bin/
	$(Q)$(STRIP) $(FSROOT)/usr/bin/logic
	$(Q)cp $(ROOTDIR)/src/apps/logic/config_full.xml $(FSROOT)/etc/ -f

#upgcgi
	$(Q)cp $(ROOTDIR)/src/apps/upgcgi/webupg $(FSROOT)/usr/www/cgi-bin -rf
	$(Q)$(STRIP) $(FSROOT)/usr/www/cgi-bin/webupg

#cli
ifeq ("$(CONFIG_APPS_CLI)","y")
	$(Q)cp $(ROOTDIR)/src/apps/cli/cmd $(FSROOT)/usr/bin
	$(Q)$(STRIP) $(FSROOT)/usr/bin/cmd
endif


#omlib
ifeq ("$(CONFIG_APPS_LIB)","y")
	echo "enter omlib and do nothing!"
endif

#tr069fe
ifeq ("$(CONFIG_APPS_TR069)","y")
    ifeq ("$(CONFIG_TR069_SSL)","y")
		$(Q)cp $(ROOTDIR)/src/apps/tr069fe/tb_tr069 $(FSROOT)/sbin
		$(Q)$(STRIP) $(FSROOT)/sbin/tb_tr069
		$(Q)cp $(ROOTDIR)/src/apps/tr069fe/verify $(FSROOT)/sbin
		$(Q)$(STRIP) $(FSROOT)/sbin/verify
    else
		$(Q)cp $(ROOTDIR)/src/apps/tr069fe/tb_tr069 $(FSROOT)/sbin
		$(Q)$(STRIP) $(FSROOT)/sbin/tb_tr069
    endif

endif

#web
ifeq ("$(CONFIG_APPS_WEB)","y")
	$(Q)cp $(ROOTDIR)/src/apps/web/webproc $(FSROOT)/usr/www/cgi-bin -rf
	$(Q)$(STRIP) $(FSROOT)/usr/www/cgi-bin/webproc

	$(Q)cp $(ROOTDIR)/src/apps/web/modlist $(ROOTDIR)/build/modlist.h -f
	$(Q)cp $(ROOTDIR)/src/apps/web/langlist $(ROOTDIR)/build/langlist.h -f
	$(Q)cp $(ROOTDIR)/src/apps/web/lang $(ROOTDIR)/build/lang.h -f
endif

#html

#tm
ifeq ("$(CONFIG_APPS_TM)","y")
	$(Q)cp $(ROOTDIR)/src/apps/tm/tm $(FSROOT)/usr/bin
	$(Q)$(STRIP) $(FSROOT)/usr/bin/tm
endif

#CONFIG_APPS_CTC_UPnP_DM_FE
ifeq ("$(CONFIG_APPS_CTC_UPnP_DM_FE)","y")
		$(Q)cp $(ROOTDIR)/src/apps/CTC_UPnP_DM_FE/upnpdm_fe $(FSROOT)/sbin
		$(Q)$(STRIP) $(FSROOT)/sbin/upnpdm_fe
endif
	
#supp
ifeq ("$(CONFIG_APPS_SUPP)","y")
	$(Q)cp $(ROOTDIR)/src/apps/supp/supp $(FSROOT)/sbin
	$(Q)$(STRIP) $(FSROOT)/sbin/supp
endif

	
ifdef CONFIG_TBS_GDB_DEBUG
	#$(Q)cp -rf $(ROOTDIR)/src/apps $(FSROOT)/usr/apps
	#$(Q)find $(FSROOT)/usr/apps -name .svn | xargs rm -rf
endif
ifeq ($(CONFIG_APPS_LOGIC_DIAG),y)
	cp $(ROOTDIR)/src/apps/logic/diagnostics/diag $(FSROOT)/sbin/diag
	cp $(ROOTDIR)/src/apps/logic/diagnostics/ralink/diagtest $(FSROOT)/sbin/diagtest
	$(STRIP) $(FSROOT)/sbin/diagtest
endif
ifeq ($(CONFIG_APPS_LOGIC_MULTILANG),y)
	cp $(ROOTDIR)/src/apps/logic/multilanguage/httpget/httpget $(FSROOT)/sbin/httpget
	$(STRIP) $(FSROOT)/sbin/httpget
endif
	cp $(ROOTDIR)/src/apps/logic/usb/hotplug/hotplug $(FSROOT)/sbin/hotplug
	$(STRIP) $(FSROOT)/sbin/hotplug
ifeq ($(CONFIG_APPS_LOGIC_LOGGER),y)
	cp $(ROOTDIR)/src/apps/logic/logger/logmonitor $(FSROOT)/usr/bin/logmonitor
	$(STRIP) $(FSROOT)/usr/bin/logmonitor
endif
ifeq ($(CONFIG_APPS_IPV6_IFIPV6),y)
	$(Q)cp $(ROOTDIR)/src/apps/ipv6/ifip6 $(FSROOT)/usr/bin/ 
	$(STRIP) $(FSROOT)/usr/bin/ifip6
endif

