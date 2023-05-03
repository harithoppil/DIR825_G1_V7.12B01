
EXTRA_CFLAGS =-I$(BUILDDIR)/include 
EXTRA_LDFLAGS =-lutil -lssap -L$(BUILDDIR)/apps/ssap/lib -L$(TMPDISTDIR)/lib -lscew -lexpat -lz

E_TARGET=flash

include $(ROOTDIR)/Rules.make

installme::
	mkdir -p $(FSROOT)/usr/bin/
	install -m 755 $(ROOTDIR)/src/vendor/realtek/flash/flash $(FSROOT)/usr/bin/
	$(STRIP) $(FSROOT)/usr/bin/flash

clean:
	-rm -f flash *.elf *.o

