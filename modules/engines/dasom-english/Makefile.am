moduledir = $(libdir)/dasom/modules
module_LTLIBRARIES = libdasom-english.la

libdasom_english_la_SOURCES = dasom-english.c dasom-english.h
libdasom_english_la_CFLAGS  = \
	-Wall -Werror \
	-I$(top_srcdir)/libdasom \
	-DG_LOG_DOMAIN=\"dasom\" \
	$(LIBDASOM_DEPS_CFLAGS)

libdasom_english_la_LDFLAGS = -avoid-version
libdasom_english_la_LIBADD  = $(top_builddir)/libdasom/libdasom.la

DISTCLEANFILES = Makefile.in

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/libdasom-english.so
	rm -f $(DESTDIR)$(moduledir)/libdasom-english.la

uninstall-hook:
	rm -f $(DESTDIR)$(moduledir)/libdasom-english.so
	-rmdir $(DESTDIR)$(moduledir)
