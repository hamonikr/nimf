# gtk3 부분 나중에 없애자

SUBDIRS = IMdkit

bin_PROGRAMS = dasom-daemon

dasom_daemon_SOURCES = \
	dasom-daemon.c \
	dasom-daemon.h \
	dasom-context.c \
	dasom-context.h

dasom_daemon_CFLAGS = \
	-Wall \
	-Werror \
	-I$(top_srcdir)/daemon/IMdkit \
	-I$(top_srcdir)/libdasom \
	-DDASOM_COMPILATION \
	-DG_LOG_DOMAIN=\"dasom\" \
	-DDASOM_MODULE_DIR=\"$(libdir)/dasom/modules\" \
	$(GLIB_CFLAGS) \
	$(GIO_CFLAGS) \
	$(GIO_UNIX_CFLAGS) \
	$(X11_CFLAGS) \
	$(GTK3_CFLAGS)

dasom_daemon_LDADD = \
	$(top_builddir)/libdasom/libdasom.la \
	$(top_builddir)/daemon/IMdkit/libIMdkit.la \
	$(GLIB_LIBS) \
	$(GIO_LIBS) \
	$(GIO_UNIX_LIBS) \
	$(X11_LIBS) \
	$(GTK3_LIBS)
