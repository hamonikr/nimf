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
	@debian/rules clean || true
	rm -rf $(STAMP_DIR) $(SOURCE_DIR) debian/tmp config.log config.status

distclean-local:
	rm -rf autom4te.cache build-aux m4 .anjuta .anjuta_sym_db.db

update-gtk-im-cache:
	$(GTK_QUERY_IMMODULES2) --update-cache
	$(GTK_QUERY_IMMODULES3) --update-cache

update-gtk-icon-cache:
	(cd data/icons; make update-gtk-icon-cache)

ChangeLog:

.PHONY: ChangeLog
