moduledir = $(libdir)/dasom/modules
module_LTLIBRARIES = dasom-jeongeum.la

icondir = $(datadir)/icons/hicolor/22x22/status
icon_DATA = dasom-ko.png

gsettings_SCHEMAS = org.freedesktop.Dasom.engines.jeongeum.gschema.xml
@GSETTINGS_RULES@

# gtk3 부분 나중에 없애자

dasom_jeongeum_la_SOURCES = dasom-jeongeum.c dasom-jeongeum.h
dasom_jeongeum_la_CFLAGS  = \
	-Wall -Werror \
	-I$(top_srcdir)/libdasom \
	-DG_LOG_DOMAIN=\"dasom\" \
	$(LIBDASOM_DEPS_CFLAGS) \
	$(LIBHANGUL_CFLAGS)

dasom_jeongeum_la_LDFLAGS = -avoid-version -module
dasom_jeongeum_la_LIBADD  = \
	$(top_builddir)/libdasom/libdasom.la \
	$(top_builddir)/modules/engines/dasom-english/libdasom-english.la \
	$(LIBHANGUL_LIBS)

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/dasom-jeongeum.so
	rm $(DESTDIR)$(moduledir)/dasom-jeongeum.la

uninstall-hook:
	rm -f $(DESTDIR)$(moduledir)/dasom-jeongeum.so
