moduledir = $(libdir)/nimf/modules
noinst_LTLIBRARIES = libnimf-libhangul.la

gsettings_SCHEMAS = org.nimf.engines.nimf-libhangul.gschema.xml
@GSETTINGS_RULES@

mssymboldir = $(libdir)/nimf
mssymbol_DATA = mssymbol.txt

libnimf_libhangul_la_SOURCES = nimf-libhangul.c
libnimf_libhangul_la_CFLAGS  = \
	$(EXTRA_CFLAGS) \
	-I$(top_srcdir)/libnimf \
	-DG_LOG_DOMAIN=\"nimf\" \
	-DMSSYMBOL_PATH=\"$(mssymboldir)/$(mssymbol_DATA)\" \
	$(NIMF_LIBHANGUL_DEPS_CFLAGS)

libnimf_libhangul_la_LDFLAGS = -avoid-version -module $(NIMF_LIBHANGUL_DEPS_LIBS)
libnimf_libhangul_la_LIBADD  = $(top_builddir)/libnimf/libnimf.la

DISTCLEANFILES = Makefile.in

install-exec-hook:
	$(MKDIR_P) $(DESTDIR)$(moduledir)
	$(INSTALL) -m 755 .libs/libnimf-libhangul.so $(DESTDIR)$(moduledir)/libnimf-libhangul.so
	chmod -x $(DESTDIR)$(moduledir)/libnimf-libhangul.so

install-data-hook:
	rm -f $(DESTDIR)$(moduledir)/libnimf-libhangul.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-libhangul.so
	-rmdir -p $(DESTDIR)$(moduledir)
