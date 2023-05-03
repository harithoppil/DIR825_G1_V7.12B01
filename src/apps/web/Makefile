ifeq "$(ROOTDIR)" ""
export ROOTDIR=$(shell while true; do if [ -f BaseVar.mk ]; then pwd;exit; else cd ..;fi;done;)
endif

include $(ROOTDIR)/.config
obj-y=web_proc.o web_outpage.o web_cgi.o web_cmd.o web_msg.o web_utils.o web_mark.o web_stub.o ../omlib/om_nameval.o ../omlib/om_buffer.o
E_TARGET=webproc
EXTRA_CFLAGS=-I$(BUILDDIR) -I$(BUILDDIR)/apps/include -I$(BUILDDIR)/apps/ssap/msg -Wall  
EXTRA_LDFLAGS=-lssap -L$(BUILDDIR)/apps/ssap/lib -L$(TMPDISTDIR)/lib -lscew -lexpat -lz
include $(ROOTDIR)/Rules.make

installme::
	$(Q)cp webproc $(FSROOT)/usr/www/cgi-bin
	$(Q)$(STRIP) $(FSROOT)/usr/www/cgi-bin/webproc

web_cgi.o: $(ROOTDIR)/build/modlist.h $(ROOTDIR)/build/langlist.h $(ROOTDIR)/build/lang.h

$(ROOTDIR)/build/modlist.h $(ROOTDIR)/build/langlist.h $(ROOTDIR)/build/lang.h:$(ROOTDIR)/build/autoconf.h
	$(Q)echo "Create modlist.h..."
	$(Q)grep "CONFIG_APPS_LOGIC_" $(ROOTDIR)/build/autoconf.h|awk '{print $$2}'|sed -n 's/\(CONFIG_APPS_LOGIC_\)\([^ ]\+\)/ADD_Mod(\1\2, "\2");/p' >$(ROOTDIR)/build/modlist.h
	$(Q)echo "Create langlist.h..."
	$(Q)grep "CONFIG_APPS_HTML_LANG_SUPPORT_" $(ROOTDIR)/build/autoconf.h|awk '{print $$2}'|sed -n 's/\(CONFIG_APPS_HTML_LANG_SUPPORT_\)\([^ ]\+\)/ADD_LANG(\1\2, "\2");/p' >$(ROOTDIR)/build/langlist.h
	$(Q)echo "Create lang.h..."
	$(Q)grep "CONFIG_APPS_HTML_LANG_DEFAULT_" $(ROOTDIR)/build/autoconf.h|awk '{print $$2}'|sed -n 's/\(CONFIG_APPS_HTML_LANG_DEFAULT_\)\([^ ]\+\)/char *g_szDefaultLang = "\2";/p' >$(ROOTDIR)/build/lang.h

