include $(ROOTDIR)/.config
include $(ROOTDIR)/BaseVar.mk
.PHONY: all build clean

CC=$(CROSS_COMPILE)gcc

SRCDIR=$(CURDIR)

EXEC = autoFWupgrade

#OBJS = autoFWupgrade.o htmlget.o serverFWupgrade.o

CFLAGS +=-fPIC -I$(BUILDDIR)/kernel/${TBS_KERNEL_VERSION}/include/ -I../msg -I../../include -I../../../include -I$(TMPDISTDIR)/include -I../syslog -Wall -O3 -D_GNU_SOURCE -I$(BUILDDIR)
LDFLAGS +=-lutil -lssap -L$(BUILDDIR)/apps/ssap/lib -L$(TMPDISTDIR)/lib -lscew -lexpat -lz

#all: $(EXEC) build
all:  build

#$(EXEC): $(OBJS)
#	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS$(LDLIBS-$(@)))

build:
	mkdir -p  $(FSROOT)/sbin $(TMPDISTDIR)/sbin
	install -m 0755 $(SRCDIR)/autoFWupgrade $(TMPDISTDIR)/sbin/autoFWupgrade
	#make -C $(SRCDIR) all

clean:
	cd $(SRCDIR); \
	#rm -f $(EXEC) *.o *~

install:
	cp $(TMPDISTDIR)/sbin/autoFWupgrade $(FSROOT)/sbin/autoFWupgrade
	$(STRIP) $(FSROOT)/sbin/autoFWupgrade
