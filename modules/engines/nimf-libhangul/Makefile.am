moduledir = $(libdir)/nimf/modules
module_LTLIBRARIES = nimf-libhangul.la

gsettings_SCHEMAS = org.nimf.engines.libhangul.gschema.xml
@GSETTINGS_RULES@

nimf_libhangul_la_SOURCES = nimf-libhangul.c
nimf_libhangul_la_CFLAGS  = \
	-Wall -Werror \
	-I$(top_srcdir)/libnimf \
	-DG_LOG_DOMAIN=\"nimf\" \
	$(NIMF_LIBHANGUL_DEPS_CFLAGS)

nimf_libhangul_la_LDFLAGS = -avoid-version -module $(NIMF_LIBHANGUL_DEPS_LIBS)
nimf_libhangul_la_LIBADD  = $(top_builddir)/libnimf/libnimf.la

DISTCLEANFILES = Makefile.in

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/nimf-libhangul.so
	rm    -f $(DESTDIR)$(moduledir)/nimf-libhangul.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(moduledir)/nimf-libhangul.so
	-rmdir -p $(DESTDIR)$(moduledir)
