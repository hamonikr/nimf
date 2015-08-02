# 후보창 때문에 gtk3을 추가하는데 나중에 모듈로 분리하면서 제거해야 합니다.

AM_CPPFLAGS = \
	-I$(top_srcdir)/libdasom \
	-DG_LOG_DOMAIN=\"dasom\" \
	$(GLIB_CFLAGS)

AM_CFLAGS = -Wall -Werror

moduledir = $(libdir)/dasom/modules
module_LTLIBRARIES = libdasom-english.la

icondir = $(datadir)/icons/hicolor/22x22/status
icon_DATA = dasom-en.png

libdasom_english_la_SOURCES  = dasom-english.c dasom-english.h
libdasom_english_la_CFLAGS   = $(AM_CFLAGS) $(GTK3_CFLAGS)
libdasom_english_la_LDFLAGS  = -avoid-version $(GTK3_LIBS)
libdasom_english_la_LIBADD   = $(top_builddir)/libdasom/libdasom.la

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/libdasom-english.so
	rm $(DESTDIR)$(moduledir)/libdasom-english.la

uninstall-hook:
	rm -f $(DESTDIR)$(moduledir)/libdasom-english.so
