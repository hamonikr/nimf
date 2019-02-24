bin_PROGRAMS = nimf

man_MANS = nimf.1

AM_CFLAGS = \
	$(EXTRA_CFLAGS) \
	-I$(top_srcdir)/libnimf \
	-I$(top_srcdir)/libnimf/client \
	-I$(top_srcdir)/libnimf/server \
	-DNIMF_COMPILATION \
	-DNIMF_MODULE_DIR=\"$(libdir)/nimf/modules\" \
	-DG_LOG_DOMAIN=\"nimf\"

nimf_SOURCES = nimf.c
nimf_CFLAGS = \
	$(LIBNIMF_DEPS_CFLAGS) \
	$(NIMF_DEPS_CFLAGS) \
	-DNIMF_SERVICE_MODULE_DIR=\"$(libdir)/nimf/modules/services\" \
	$(AM_CFLAGS)

nimf_LDFLAGS = $(NIMF_DEPS_LIBS)
nimf_LDADD   = $(top_builddir)/libnimf/libnimf.la

DISTCLEANFILES = Makefile.in
