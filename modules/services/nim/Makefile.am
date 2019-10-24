moduledir = $(libdir)/nimf/modules/services
module_LTLIBRARIES = libnimf-nim.la

libnimf_nim_la_SOURCES = \
	nimf-nim.c \
	nimf-nim.h \
	nimf-nim-ic.c \
	nimf-nim-ic.h \
	nimf-connection.c \
	nimf-connection.h \
	$(NULL)

libnimf_nim_la_CFLAGS = \
	$(EXTRA_CFLAGS) \
	-I$(top_srcdir)/libnimf \
	-DG_LOG_DOMAIN=\"nimf\" \
	$(NIMF_NIM_DEPS_CFLAGS)

libnimf_nim_la_LDFLAGS = -avoid-version -module $(NIMF_NIM_DEPS_LIBS)
libnimf_nim_la_LIBADD  = $(top_builddir)/libnimf/libnimf.la

DISTCLEANFILES = Makefile.in

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/libnimf-nim.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-nim.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-nim.so
	-rmdir -p $(DESTDIR)$(moduledir)
