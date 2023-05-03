include $(ROOTDIR)/.config
include $(ROOTDIR)/BaseVar.mk
.PHONY: all clean 

all: build install
build:
	$(CC) -I $(BUILDDIR) -o nbnslisten -Os $(CFLAGS) nbns_listen.c

clean:
install:
	cp $(ROOTDIR)/src/opensource/nbns/nbnslisten $(FSROOT)/sbin/nbns 
	$(STRIP) $(FSROOT)/sbin/nbns 