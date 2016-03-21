moduledir = $(libdir)/nimf/modules
module_LTLIBRARIES = nimf-sunpinyin.la

gsettings_SCHEMAS = org.nimf.engines.sunpinyin.gschema.xml
@GSETTINGS_RULES@

nimf_sunpinyin_la_SOURCES = nimf-sunpinyin.cpp
nimf_sunpinyin_la_CXXFLAGS  = \
	-Wall -Werror \
	-I$(top_srcdir)/libnimf \
	-DG_LOG_DOMAIN=\"nimf\" \
	$(NIMF_SUNPINYIN_DEPS_CFLAGS)

nimf_sunpinyin_la_LDFLAGS = -avoid-version -module $(NIMF_SUNPINYIN_DEPS_LIBS)
nimf_sunpinyin_la_LIBADD  = $(top_builddir)/libnimf/libnimf.la

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/nimf-sunpinyin.so
	rm    -f $(DESTDIR)$(moduledir)/nimf-sunpinyin.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(moduledir)/nimf-sunpinyin.so
	-rmdir -p $(DESTDIR)$(moduledir)

DISTCLEANFILES = Makefile.in
