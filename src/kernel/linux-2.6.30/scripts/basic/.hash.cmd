cmd_scripts/basic/hash := gcc -Wp,-MD,scripts/basic/.hash.d -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer     -o scripts/basic/hash scripts/basic/hash.c  

deps_scripts/basic/hash := \
  scripts/basic/hash.c \
  /usr/include/stdio.h \
  /usr/local/lib/gcc/i686-pc-linux-gnu/4.9.0/include-fixed/features.h \
  /usr/include/sys/cdefs.h \
  /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h \
  /usr/include/gnu/stubs-32.h \
  /usr/local/lib/gcc/i686-pc-linux-gnu/4.9.0/include/stddef.h \
  /usr/include/bits/types.h \
  /usr/include/bits/typesizes.h \
  /usr/include/libio.h \
  /usr/include/_G_config.h \
  /usr/local/lib/gcc/i686-pc-linux-gnu/4.9.0/include-fixed/wchar.h \
  /usr/include/bits/wchar.h \
  /usr/include/gconv.h \
  /usr/local/lib/gcc/i686-pc-linux-gnu/4.9.0/include/stdarg.h \
  /usr/include/bits/stdio_lim.h \
  /usr/include/bits/sys_errlist.h \
  /usr/include/bits/stdio.h \
  /usr/include/stdlib.h \
  /usr/include/sys/types.h \
  /usr/include/time.h \
  /usr/include/endian.h \
  /usr/include/bits/endian.h \
  /usr/include/sys/select.h \
  /usr/include/bits/select.h \
  /usr/include/bits/sigset.h \
  /usr/include/bits/time.h \
  /usr/local/lib/gcc/i686-pc-linux-gnu/4.9.0/include-fixed/sys/sysmacros.h \
  /usr/include/bits/pthreadtypes.h \
  /usr/include/alloca.h \
  /usr/include/string.h \
  /usr/include/bits/string.h \
  /usr/local/lib/gcc/i686-pc-linux-gnu/4.9.0/include-fixed/bits/string2.h \

scripts/basic/hash: $(deps_scripts/basic/hash)

$(deps_scripts/basic/hash):
