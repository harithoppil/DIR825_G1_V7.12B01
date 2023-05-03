include $(ROOTDIR)/.config
include $(BUILDDIR)/arch.mk


target-y = cmddaemon 
all: $(target-y)

cmddaemon: cmddaemon.o

#CFLAGS = -Dlinux -D__linux__ -Dunix  -D_LINUX_2_6_ -I. -I$(TOOLCHAINS_SYSROOT)/include -I$(KERNEL_INCLUDE_DIR) \
			    -I$(TOOLCHAINS_SYSROOT)/../lib/gcc-lib/mips-linux-uclibc/3.3.5/include
CFLAGS += -Os -Wall -Werror -pipe -I../ssap/msg -I$(BUILDDIR) -I$(BUILDDIR)/apps/include  -I$(TMPDISTDIR)/include
LDFLAGS += -lutil -lssap -L$(BUILDDIR)/apps/ssap/lib -L$(TMPDISTDIR)/lib -lscew -lexpat -lz

ifdef CONFIG_TBS_APPS_DEBUG_CMDDAEMON
CFLAGS += -DCMDDAEMON_DEBUG
endif

%.o : %.c
	$(CC) $(CFLAGS) -c $^ -o $@

install:
	install -c $(target-y) $(FSROOT)/usr/bin/
	-cd $(FSROOT)/usr/bin/; $(STRIP) $(target-y)

clean:
	-rm -f *.o *.d $(target-y)
