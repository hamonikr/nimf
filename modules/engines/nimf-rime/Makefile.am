moduledir = $(libdir)/nimf/modules
module_LTLIBRARIES = libnimf-rime.la

gsettings_SCHEMAS = org.nimf.engines.nimf-rime.gschema.xml
@GSETTINGS_RULES@

libnimf_rime_la_SOURCES = nimf-rime.c
libnimf_rime_la_CFLAGS  = \
	$(EXTRA_CFLAGS) \
	-I$(top_srcdir)/libnimf \
	-DG_LOG_DOMAIN=\"nimf\" \
	$(NIMF_RIME_DEPS_CFLAGS)

libnimf_rime_la_LDFLAGS = -avoid-version -module $(NIMF_RIME_DEPS_LIBS)
libnimf_rime_la_LIBADD  = $(top_builddir)/libnimf/libnimf.la

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/libnimf-rime.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-rime.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-rime.so
	-rmdir -p $(DESTDIR)$(moduledir)

DISTCLEANFILES = Makefile.in
