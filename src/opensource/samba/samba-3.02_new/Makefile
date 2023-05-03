include $(ROOTDIR)/.config
include $(ROOTDIR)/BaseVar.mk

ROMFSINST=install

EXTCFLAGS=-I$(TOOLCHAINS_SYSINCLUDE)
EXTLDFLAGS=-L$(TOOLCHAINS_SYSROOT)lib
export EXTCFLAGS EXTLDFLAGS

all:
	$(MAKE) -C source

install: all
	$(ROMFSINST) source/bin/CP850.so $(FSROOT)/lib/CP850.so
	#$(STRIP) $(FSROOT)/lib/CP850.so
	$(ROMFSINST) source/bin/CP437.so $(FSROOT)/lib/CP437.so
	#$(STRIP) $(FSROOT)/lib/CP437.so
	$(ROMFSINST) source/bin/libbigballofmud.so $(FSROOT)/lib/libbigballofmud.so	
	#$(STRIP) $(FSROOT)/lib/libbigballofmud.so
	cd $(FSROOT)/lib/; ln -snf libbigballofmud.so libbigballofmud.so.0
	$(ROMFSINST) source/bin/nmbd $(FSROOT)/sbin/nmbd
	#$(STRIP) $(FSROOT)/sbin/nmbd
	$(ROMFSINST) source/bin/smbd $(FSROOT)/sbin/smbd
	#$(STRIP) $(FSROOT)/sbin/smbd
	$(ROMFSINST) source/bin/smbpasswd $(FSROOT)/sbin/smbpasswd	
	#$(STRIP) $(FSROOT)/sbin/smbpasswd
#	mkdir -p $(FSROOT)/etc/samba
	$(ROMFSINST) packaging/SGI/smbpasswd $(FSROOT)/var/smbpasswd
	$(ROMFSINST) packaging/Mandrake/smbusers $(FSROOT)/var/smbusers
#	$(ROMFSINST) source/smbadduser $(FSROOT)/bin
#	chmod 777 $(FSROOT)/bin/smbadduser
#	$(ROMFSINST) source/bin/smbpasswd $(FSROOT)/sbin/smbpasswd
#	$(ROMFSINST) samba.sh $(FSROOT)/bin
#	$(ROMFSINST) samba_add_dir.sh $(FSROOT)/bin
#	cd $(FSROOT)/etc; ln -snf ../var/smb.conf smb.conf
#	$(ROMFSINST) -D source/smb.conf $(FSROOT)/etc/samba/smb.conf
clean:
	$(MAKE) -C source clean
