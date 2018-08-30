moduledir = $(libdir)/nimf/modules
module_LTLIBRARIES = libnimf-chewing.la

gsettings_SCHEMAS = org.nimf.engines.nimf-chewing.gschema.xml
@GSETTINGS_RULES@

libnimf_chewing_la_SOURCES = nimf-chewing.c
libnimf_chewing_la_CFLAGS  = \
	$(EXTRA_CFLAGS) \
	-I$(top_srcdir)/libnimf \
	-I$(top_srcdir)/libnimf/client \
	-I$(top_srcdir)/libnimf/server \
	-DG_LOG_DOMAIN=\"nimf\" \
	$(NIMF_CHEWING_DEPS_CFLAGS)

libnimf_chewing_la_LDFLAGS = -avoid-version -module $(NIMF_CHEWING_DEPS_LIBS)
libnimf_chewing_la_LIBADD  = $(top_builddir)/libnimf/libnimf.la

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/libnimf-chewing.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-chewing.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-chewing.so
	-rmdir -p $(DESTDIR)$(moduledir)

DISTCLEANFILES = Makefile.in
