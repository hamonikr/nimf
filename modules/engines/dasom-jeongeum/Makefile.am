AM_CPPFLAGS = \
	-I$(top_srcdir)/libdasom \
	-DG_LOG_DOMAIN=\"dasom\" \
	$(GLIB_CFLAGS)

AM_CFLAGS = -Wall -Werror

moduledir = $(libdir)/dasom/modules
module_LTLIBRARIES = dasom-jeongeum.la

icondir = $(datadir)/icons/hicolor/22x22/status
icon_DATA = dasom-ko.png

# gtk3 부분 나중에 없애자

dasom_jeongeum_la_SOURCES = dasom-jeongeum.c dasom-jeongeum.h
dasom_jeongeum_la_CFLAGS  = $(AM_CFLAGS) $(LIBHANGUL_CFLAGS) $(GTK3_CFLAGS)
dasom_jeongeum_la_LDFLAGS = -avoid-version -module $(GTK3_LIBS)
dasom_jeongeum_la_LIBADD  = \
	$(top_builddir)/libdasom/libdasom.la \
	$(LIBHANGUL_LIBS)

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/dasom-jeongeum.so
	rm $(DESTDIR)$(moduledir)/dasom-jeongeum.la

uninstall-hook:
	rm -f $(DESTDIR)$(moduledir)/dasom-jeongeum.so
