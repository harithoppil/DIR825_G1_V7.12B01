EXTRA_CFLAGS=  -I../lib -I${BUILDDIR}/apps/include -I../../../ssap/msg \
              -I../../../ssap/lib -I${BUILDDIR}/include/ -I$(TMPDISTDIR)/include -I${BUILDDIR}/ -Wall -Os
              
EXTRA_LDFLAGS=-lutil -lssap -L$(BUILDDIR)/apps/ssap/lib -L$(TMPDISTDIR)/lib -lscew -lexpat -lz

ifeq (${PRODUCT}, UML)
	EXTRA_CFLAGS += -DLOGGER_UML
endif

E_TARGET = hotplug 

obj-y = hotplug.o hotplug_storage.o
obj-y += hotplug_usb_lp.o
obj-y += hotplug_usb_modem.o

include $(ROOTDIR)/Rules.make

installme::
	$(Q)cp $(E_TARGET) $(FSROOT)/sbin/
	$(Q)$(STRIP) $(FSROOT)/sbin/$(E_TARGET)
	#cp $(E_TARGET) $(SRCDIR)/filesystem/basefilesystem/root/var/
	#ln -sf ../var/hotplug $(FSROOT)/sbin/hotplug 

