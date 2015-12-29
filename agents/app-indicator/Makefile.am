bin_PROGRAMS = nimf-indicator

nimf_indicator_SOURCES = nimf-indicator.c

nimf_indicator_CFLAGS = \
	-Wall \
	-Werror \
	-I$(top_srcdir)/libnimf \
	-DNIMF_COMPILATION \
	-DG_LOG_DOMAIN=\"nimf\" \
	$(GLIB_CFLAGS) \
	$(GIO_CFLAGS) \
	$(GIO_UNIX_CFLAGS) \
	$(X11_CFLAGS) \
	$(GTK3_CFLAGS) \
	$(APPINDICATOR3_CFLAGS)

nimf_indicator_LDADD = $(top_builddir)/libnimf/libnimf.la

nimf_indicator_LDFLAGS = \
	$(GLIB_LIBS) \
	$(GIO_LIBS) \
	$(GIO_UNIX_LIBS) \
	$(X11_LIBS) \
	$(GTK3_LIBS) \
	$(APPINDICATOR3_LIBS)

DISTCLEANFILES = Makefile.in
