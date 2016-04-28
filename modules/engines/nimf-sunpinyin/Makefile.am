moduledir = $(libdir)/nimf/modules
module_LTLIBRARIES = libnimf-sunpinyin.la

gsettings_SCHEMAS = org.nimf.engines.sunpinyin.gschema.xml
@GSETTINGS_RULES@

libnimf_sunpinyin_la_SOURCES = nimf-sunpinyin.cpp
libnimf_sunpinyin_la_CXXFLAGS  = \
	-Wall -Werror \
	-I$(top_srcdir)/libnimf \
	-DG_LOG_DOMAIN=\"nimf\" \
	$(NIMF_SUNPINYIN_DEPS_CFLAGS)

libnimf_sunpinyin_la_LDFLAGS = -avoid-version -module $(NIMF_SUNPINYIN_DEPS_LIBS)
libnimf_sunpinyin_la_LIBADD  = $(top_builddir)/libnimf/libnimf.la

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/libnimf-sunpinyin.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-sunpinyin.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-sunpinyin.so
	-rmdir -p $(DESTDIR)$(moduledir)

DISTCLEANFILES = Makefile.in
