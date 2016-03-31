moduledir = $(libdir)/nimf/modules
module_LTLIBRARIES = nimf-anthy.la

gsettings_SCHEMAS = org.nimf.engines.anthy.gschema.xml
@GSETTINGS_RULES@

nimf_anthy_la_SOURCES = nimf-anthy.c
nimf_anthy_la_CFLAGS  = \
	-Wall -Werror \
	-I$(top_srcdir)/libnimf \
	-DG_LOG_DOMAIN=\"nimf\" \
	$(NIMF_ANTHY_DEPS_CFLAGS)

nimf_anthy_la_LDFLAGS = -avoid-version -module $(NIMF_ANTHY_DEPS_LIBS)
nimf_anthy_la_LIBADD  = $(top_builddir)/libnimf/libnimf.la

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/nimf-anthy.so
	rm    -f $(DESTDIR)$(moduledir)/nimf-anthy.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(moduledir)/nimf-anthy.so
	-rmdir -p $(DESTDIR)$(moduledir)

DISTCLEANFILES = Makefile.in
