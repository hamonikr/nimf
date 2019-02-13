moduledir = $(libdir)/nimf/modules
module_LTLIBRARIES = libnimf-m17n.la

gsettings_SCHEMAS = org.nimf.engines.nimf-m17n.gschema.xml
@GSETTINGS_RULES@

libnimf_m17n_la_SOURCES = nimf-m17n.c
libnimf_m17n_la_CFLAGS  = \
	$(EXTRA_CFLAGS) \
	-I$(top_srcdir)/libnimf \
	-I$(top_srcdir)/libnimf/client \
	-I$(top_srcdir)/libnimf/server \
	-DG_LOG_DOMAIN=\"nimf\" \
	$(NIMF_M17N_DEPS_CFLAGS)

libnimf_m17n_la_LDFLAGS = -avoid-version -module $(NIMF_M17N_DEPS_LIBS)
libnimf_m17n_la_LIBADD  = $(top_builddir)/libnimf/libnimf.la

DISTCLEANFILES = Makefile.in

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/libnimf-m17n.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-m17n.so
	-rmdir -p $(DESTDIR)$(moduledir)
