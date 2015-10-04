SUBDIRS = libdasom modules daemon agents po data

ACLOCAL_AMFLAGS = -I m4

DISTCLEANFILES = \
	Makefile.in \
	po/Makefile.in.in \
	INSTALL \
	aclocal.m4 \
	config.h.in \
	config.h.in~ \
	configure

clean-local:
	-debian/rules clean

distclean-local:
	rm -rf autom4te.cache build-aux m4

update-gtk-im-cache:
	$(GTK_QUERY_IMMODULES2) --update-cache
	$(GTK_QUERY_IMMODULES3) --update-cache

ChangeLog:

.PHONY: ChangeLog
