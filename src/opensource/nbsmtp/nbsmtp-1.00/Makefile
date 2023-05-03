srcdir=.
prefix=/usr/local
sysconfdir=${prefix}/etc
exec_prefix=${prefix}
libexecdir=${exec_prefix}/libexec
doxygen=/usr/bin/doxygen
bindir=$(prefix)/bin
mandir=$(prefix)/man
iconpath=$(prefix)/nbsmtp-1.00
INSTALL=/usr/bin/install -c

SRCS=nbsmtp.c base64.c util.c smtp.c fdutil.c original.c fileconfig.c main.c hmac_md5.c

OBJS=$(SRCS:.c=.o)

CFLAGS=-Wall -pedantic -DPACKAGE_NAME=\"nbsmtp\" -DPACKAGE_TARNAME=\"nbsmtp\" -DPACKAGE_VERSION=\"1.00\" -DPACKAGE_STRING=\"nbsmtp\ 1.00\" -DPACKAGE_BUGREPORT=\"nbsmtp@ferdyx.org\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_STDIO_H=1 -DHAVE_UNISTD_H=1 -DHAVE_STDLIB_H=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_SOCKET_H=1 -DHAVE_SYS_PARAM_H=1 -DHAVE_NETDB_H=1 -DHAVE_NETINET_IN_H=1 -DHAVE_FCNTL_H=1 -DHAVE_CTYPE_H=1 -DHAVE_SYSLOG_H=1 -DHAVE_STRING_H=1 -DHAVE_PWD_H=1 -D_GNU_SOURCE=1 -DHAVE_STDLIB_H=1 -DHAVE_MALLOC=1 -DHAVE_STDLIB_H=1 -DHAVE_REALLOC=1 -DHAVE_STRFTIME=1 -DHAVE_VPRINTF=1 -DHAVE_GETHOSTBYNAME=1 -DHAVE_GETHOSTNAME=1 -DHAVE_SOCKET=1 -DHAVE_STRDUP=1 -DHAVE_STRCASESTR=1 -DHAVE_ASPRINTF=1 -DHAVE_MEMSET=1 -DHAVE_GETOPT=1 -DHAVE_STRSEP=1  -DSYSCONFDIR=\"$(sysconfdir)\" $(SSL_DEFS)  -g -O2

.PHONY: all clean distclean install doc uninstall changelog

all: nbsmtp

nbsmtp: $(OBJS)
	$(CC) -o nbsmtp $(OBJS)
clean:
	$(RM) nbsmtp *.o core

distclean: clean
	$(RM) config.* Makefile

install: nbsmtp
	[ "x" = "xyes" ] \
	&& ( \
		$(INSTALL) -d $(iconpath); \
		$(INSTALL) nbsmtp-growl.png $(iconpath)/nbsmtp-growl.png; ) \
	|| ( true )
	$(INSTALL) -d $(bindir)
	$(INSTALL) nbsmtp $(bindir)/nbsmtp
	$(INSTALL) -d $(mandir)/man8
	$(INSTALL) nbsmtp.8 $(mandir)/man8/nbsmtp.8
	$(INSTALL) -d $(mandir)/man5
	$(INSTALL) nbsmtprc.5 $(mandir)/man5/nbsmtprc.5

doc:
	@( [ ! -z $(doxygen) ] && $(doxygen) Doxyfile ) || echo "Doxygen wasn't found"

uninstall:
	$(RM) $(iconpath)/nbsmtp-growl.png
	$(RM) $(bindir)/nbsmtp
	$(RM) $(mandir)/man8/nbsmtp.8
	$(RM) $(mandir)/man5/nbsmtprc.5

changelog:
	perl cvs2cl.pl -U AUTHORS -r -T
	${EDITOR} ChangeLog
