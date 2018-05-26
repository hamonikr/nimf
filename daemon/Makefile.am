bin_PROGRAMS = nimf

nimf_SOURCES = nimf.c

nimf_CFLAGS = \
	-Wall \
	-Werror \
	-I$(top_srcdir)/libnimf \
	-DNIMF_COMPILATION \
	-DG_LOG_DOMAIN=\"nimf\" \
	-DNIMF_MODULE_DIR=\"$(libdir)/nimf/modules\" \
	$(LIBNIMF_DEPS_CFLAGS) \
	$(NIMF_DEPS_CFLAGS)

nimf_LDFLAGS = $(NIMF_DEPS_LIBS)
nimf_LDADD   = $(top_builddir)/libnimf/libnimf.la

DISTCLEANFILES = Makefile.in
