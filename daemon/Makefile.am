bin_PROGRAMS = nimf-daemon

nimf_daemon_SOURCES = nimf-daemon.c

nimf_daemon_CFLAGS = \
	-Wall \
	-Werror \
	-I$(top_srcdir)/libnimf \
	-DNIMF_COMPILATION \
	-DG_LOG_DOMAIN=\"nimf\" \
	-DNIMF_MODULE_DIR=\"$(libdir)/nimf/modules\" \
	$(LIBNIMF_DEPS_CFLAGS)

nimf_daemon_LDFLAGS = $(GOBJECT_LIBS)
nimf_daemon_LDADD   = $(top_builddir)/libnimf/libnimf.la

DISTCLEANFILES = Makefile.in
