cmd_scripts/kconfig/mconf.o := gcc -Wp,-MD,scripts/kconfig/.mconf.o.d -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer   -I/usr/include/ncurses -DCURSES_LOC="<ncurses.h>" -DLOCALE   -c -o scripts/kconfig/mconf.o scripts/kconfig/mconf.c

deps_scripts/kconfig/mconf.o := \
  scripts/kconfig/mconf.c \
    $(wildcard include/config/mode.h) \
    $(wildcard include/config/color.h) \
    $(wildcard include/config/.h) \
  /usr/include/ctype.h \
  /usr/local/lib/gcc/i686-pc-linux-gnu/4.9.0/include-fixed/features.h \
  /usr/include/sys/cdefs.h \
  /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h \
  /usr/include/gnu/stubs-32.h \
  /usr/include/bits/types.h \
  /usr/local/lib/gcc/i686-pc-linux-gnu/4.9.0/include/stddef.h \
  /usr/include/bits/typesizes.h \
  /usr/include/endian.h \
  /usr/include/bits/endian.h \
  /usr/include/errno.h \
  /usr/include/bits/errno.h \
  /usr/include/linux/errno.h \
  /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h \
  /usr/include/asm-generic/errno-base.h \
  /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h \
  /usr/include/sys/types.h \
  /usr/include/time.h \
  /usr/include/sys/select.h \
  /usr/include/bits/select.h \
  /usr/include/bits/sigset.h \
  /usr/include/bits/time.h \
  /usr/local/lib/gcc/i686-pc-linux-gnu/4.9.0/include-fixed/sys/sysmacros.h \
  /usr/include/bits/pthreadtypes.h \
  /usr/local/lib/gcc/i686-pc-linux-gnu/4.9.0/include-fixed/limits.h \
  /usr/local/lib/gcc/i686-pc-linux-gnu/4.9.0/include-fixed/syslimits.h \
  /usr/include/limits.h \
  /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h \
  /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h \
  /usr/local/lib/gcc/i686-pc-linux-gnu/4.9.0/include/stdarg.h \
  /usr/include/stdlib.h \
  /usr/include/alloca.h \
  /usr/include/string.h \
  /usr/include/bits/string.h \
  /usr/local/lib/gcc/i686-pc-linux-gnu/4.9.0/include-fixed/bits/string2.h \
  /usr/include/unistd.h \
  /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h \
  /usr/include/getopt.h \
  /usr/include/locale.h \
  /usr/include/bits/locale.h \
  scripts/kconfig/lkc.h \
    $(wildcard include/config/list.h) \
  scripts/kconfig/expr.h \
  /usr/include/stdio.h \
  /usr/include/libio.h \
  /usr/include/_G_config.h \
  /usr/local/lib/gcc/i686-pc-linux-gnu/4.9.0/include-fixed/wchar.h \
  /usr/include/bits/wchar.h \
  /usr/include/gconv.h \
  /usr/include/bits/stdio_lim.h \
  /usr/include/bits/sys_errlist.h \
  /usr/include/bits/stdio.h \
  /usr/local/lib/gcc/i686-pc-linux-gnu/4.9.0/include/stdbool.h \
  /usr/include/libintl.h \
  scripts/kconfig/lkc_proto.h \
  scripts/kconfig/lxdialog/dialog.h \
  /usr/include/ncurses/ncurses.h \
  /usr/include/ncurses/ncurses_dll.h \
  /usr/include/ncurses/unctrl.h \
  /usr/include/ncurses/curses.h \

scripts/kconfig/mconf.o: $(deps_scripts/kconfig/mconf.o)

$(deps_scripts/kconfig/mconf.o):
