moduledir = $(libdir)/nimf/modules
module_LTLIBRARIES = nimf-jeongeum.la

gsettings_SCHEMAS = org.nimf.engines.jeongeum.gschema.xml
@GSETTINGS_RULES@

nimf_jeongeum_la_SOURCES = nimf-jeongeum.c
nimf_jeongeum_la_CFLAGS  = \
	-Wall -Werror \
	-I$(top_srcdir)/libnimf \
	-DG_LOG_DOMAIN=\"nimf\" \
	$(LIBNIMF_DEPS_CFLAGS) \
	$(LIBHANGUL_CFLAGS)

nimf_jeongeum_la_LDFLAGS = -avoid-version -module \
	$(LIBNIMF_DEPS_LIBS) $(LIBHANGUL_LIBS)
nimf_jeongeum_la_LIBADD  = $(top_builddir)/libnimf/libnimf.la

DISTCLEANFILES = Makefile.in

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/nimf-jeongeum.so
	rm    -f $(DESTDIR)$(moduledir)/nimf-jeongeum.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(moduledir)/nimf-jeongeum.so
	-rmdir -p $(DESTDIR)$(moduledir)
