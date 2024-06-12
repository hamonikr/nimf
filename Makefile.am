SUBDIRS = libnimf modules bin po data docs

ACLOCAL_AMFLAGS = -I m4

DISTCLEANFILES = \
	Makefile.in \
	po/Makefile.in.in \
	INSTALL \
	aclocal.m4 \
	config.h.in \
	config.h.in~ \
	configure \
	gtk-doc.make

clean-local:
	-debian/rules clean

distclean-local:
	rm -rf autom4te.cache build-aux m4 .anjuta .anjuta_sym_db.db

update-gtk-im-cache:
	$(GTK_QUERY_IMMODULES2) --update-cache
	$(GTK_QUERY_IMMODULES3) --update-cache

update-gtk-icon-cache:
	(cd data/icons; make update-gtk-icon-cache)

install-exec-hook:
	$(LIBTOOL) --finish $(DESTDIR)/usr/lib/x86_64-linux-gnu/qt5/plugins/platforminputcontexts
	$(LIBTOOL) --finish $(DESTDIR)/usr/lib/x86_64-linux-gnu/nimf/modules/services
	$(LIBTOOL) --finish $(DESTDIR)/usr/lib/x86_64-linux-gnu/gtk-3.0/3.0.0/immodules
	$(LIBTOOL) --finish $(DESTDIR)/usr/lib/x86_64-linux-gnu/nimf/modules
	$(LIBTOOL) --finish $(DESTDIR)/usr/lib/x86_64-linux-gnu

ChangeLog:

.PHONY: ChangeLog
