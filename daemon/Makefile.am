bin_PROGRAMS = dasom-daemon

dasom_daemon_SOURCES = dasom-daemon.c

dasom_daemon_CFLAGS = \
	-Wall \
	-Werror \
	-I$(top_srcdir)/libdasom \
	-DDASOM_COMPILATION \
	-DG_LOG_DOMAIN=\"dasom\" \
	-DDASOM_MODULE_DIR=\"$(libdir)/dasom/modules\" \
	$(LIBDASOM_DEPS_CFLAGS)

dasom_daemon_LDFLAGS = $(GOBJECT_LIBS)
dasom_daemon_LDADD = \
	$(top_builddir)/libdasom/libdasom.la

DISTCLEANFILES = Makefile.in
