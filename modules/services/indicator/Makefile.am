moduledir = $(libdir)/nimf/modules/services
module_LTLIBRARIES = libnimf-indicator.la

libnimf_indicator_la_SOURCES = \
	nimf-indicator.c \
	$(NULL)

libnimf_indicator_la_CFLAGS = \
	$(EXTRA_CFLAGS) \
	-I$(top_srcdir)/libnimf \
	-DG_LOG_DOMAIN=\"nimf\" \
	-DNIMF_MODULE_DIR=\"$(libdir)/nimf/modules\" \
	$(NIMF_INDICATOR_DEPS_CFLAGS)

libnimf_indicator_la_LDFLAGS = -avoid-version -module $(NIMF_INDICATOR_DEPS_LIBS)
libnimf_indicator_la_LIBADD  = $(top_builddir)/libnimf/libnimf.la

DISTCLEANFILES = Makefile.in

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/libnimf-indicator.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-indicator.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-indicator.so
	-rmdir -p $(DESTDIR)$(moduledir)
