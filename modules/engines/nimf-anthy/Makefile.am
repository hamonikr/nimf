moduledir = $(libdir)/nimf/modules
module_LTLIBRARIES = libnimf-anthy.la

gsettings_SCHEMAS = org.nimf.engines.nimf-anthy.gschema.xml
@GSETTINGS_RULES@

libnimf_anthy_la_SOURCES = nimf-anthy.c
libnimf_anthy_la_CFLAGS  = \
	$(EXTRA_CFLAGS) \
	-I$(top_srcdir)/libnimf \
	-DG_LOG_DOMAIN=\"nimf\" \
	$(NIMF_ANTHY_DEPS_CFLAGS)

libnimf_anthy_la_LDFLAGS = -avoid-version -module $(NIMF_ANTHY_DEPS_LIBS)
libnimf_anthy_la_LIBADD  = $(top_builddir)/libnimf/libnimf.la

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/libnimf-anthy.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-anthy.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-anthy.so
	-rmdir -p $(DESTDIR)$(moduledir)

DISTCLEANFILES = Makefile.in
