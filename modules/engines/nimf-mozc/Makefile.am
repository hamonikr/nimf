moduledir = $(libdir)/nimf/modules
module_LTLIBRARIES = libnimf-mozc.la

gsettings_SCHEMAS = org.nimf.engines.nimf-mozc.gschema.xml
@GSETTINGS_RULES@

libnimf_mozc_la_SOURCES = nimf-mozc.c
libnimf_mozc_la_CFLAGS  = \
	$(EXTRA_CFLAGS) \
	-I$(top_srcdir)/libnimf \
	-DG_LOG_DOMAIN=\"nimf\" \
	$(NIMF_MOZC_DEPS_CFLAGS)

libnimf_mozc_la_LDFLAGS = -avoid-version -module $(NIMF_MOZC_DEPS_LIBS)
libnimf_mozc_la_LIBADD  = $(top_builddir)/libnimf/libnimf.la

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/libnimf-mozc.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-mozc.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-mozc.so
	-rmdir -p $(DESTDIR)$(moduledir)

DISTCLEANFILES = Makefile.in