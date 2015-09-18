bin_PROGRAMS = dasom-indicator

dasom_indicator_SOURCES = dasom-indicator.c

dasom_indicator_CFLAGS = \
	-Wall \
	-Werror \
	-I$(top_srcdir)/libdasom \
	-DDASOM_COMPILATION \
	-DG_LOG_DOMAIN=\"dasom\" \
	$(GLIB_CFLAGS) \
	$(GIO_CFLAGS) \
	$(GIO_UNIX_CFLAGS) \
	$(X11_CFLAGS) \
	$(GTK3_CFLAGS) \
	$(APPINDICATOR3_CFLAGS)

dasom_indicator_LDADD = \
	$(top_builddir)/libdasom/libdasom.la

dasom_indicator_LDFLAGS = \
	$(GLIB_LIBS) \
	$(GIO_LIBS) \
	$(GIO_UNIX_LIBS) \
	$(X11_LIBS) \
	$(GTK3_LIBS) \
	$(APPINDICATOR3_LIBS)

DISTCLEANFILES = Makefile.in
