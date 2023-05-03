include $(ROOTDIR)/.config

EXTRA_CFLAGS = -I$(BUILDDIR) -I$(BUILDDIR)/include -I$(KERNEL_DIR)/include -I$(BUILDDIR)/apps/include -I$(TMPDISTDIR)/include
EXTRA_LDFLAGS = -L$(TMPDISTDIR)/lib -lssl -lcrypto -lz

CERT_PEM = public.pem
obj-y = crypt.o main.o
E_TARGET = imgdecrypt imgencrypt

include $(ROOTDIR)/Rules.make

installme::
	$(Q)cp imgdecrypt $(FSROOT)/usr/bin/
	$(Q)cp $(CERT_PEM) $(FSROOT)/etc/
	$(Q)$(STRIP) $(FSROOT)/usr/bin/imgdecrypt