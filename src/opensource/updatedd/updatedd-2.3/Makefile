# -- updatedd: Makefile.in --
#
# Copyright (C) 2002, 2003, 2004 Philipp Benner
#
# This file is part of UpdateDD - http://updatedd.philipp-benner.de.
#
# UpdateDD is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# any later version.
#
# UpdateDD is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with UpdateDD; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

SUBDIRS = libexception_handle src src/plugins scripts Documentation

define recursive 
	@for i in $(SUBDIRS); do		\
		$(MAKE) -C $$i $@;		\
		if test $$? -ne 0; then		\
			break;			\
		fi;				\
	done
endef

all: build-stamp
build-stamp:
	@for i in $(SUBDIRS); do		\
		$(MAKE) -C $$i all;		\
		if test $$? -ne 0; then		\
			break;			\
		fi;				\
	done
	touch build-stamp

install: all
	$(recursive)

uninstall:
	$(recursive)

clean:
	$(recursive)
	$(RM) build-stamp

distclean: clean
	$(recursive)
	$(RM) -f Makefile
	$(RM) -f build
	$(RM) -f config.status
	$(RM) -f config.log
	$(RM) -f include/config.h
	$(RM) -f include/version.h

.PHONY: all install uninstall distclean build-stamp
