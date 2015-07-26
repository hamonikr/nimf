# 후보창 때문에 gtk3을 추가하는데 나중에 모듈로 분리하면서 제거해야 합니다.

AM_CPPFLAGS = \
	-I$(top_srcdir)/libdasom \
	-DG_LOG_DOMAIN=\"dasom\" \
	$(GLIB_CFLAGS)

AM_CFLAGS = -Wall -Werror

moduledir = $(libdir)/dasom/modules
module_LTLIBRARIES = dasom-english.la

icondir = $(datadir)/icons/hicolor/22x22/status
icon_DATA = dasom-en.png

dasom_english_la_SOURCES  = dasom-english.c dasom-english.h
dasom_english_la_CFLAGS   = $(AM_CFLAGS) $(GTK3_CFLAGS)
dasom_english_la_LDFLAGS  = -avoid-version -module $(GTK3_LIBS)
dasom_english_la_LIBADD   = $(top_builddir)/libdasom/libdasom.la

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/dasom-english.so
	rm $(DESTDIR)$(moduledir)/dasom-english.la

uninstall-hook:
	rm -f $(DESTDIR)$(moduledir)/dasom-english.so
