bin_PROGRAMS = nimf-indicator

nimf_indicator_SOURCES = nimf-indicator.c

nimf_indicator_CFLAGS = \
	-Wall \
	-Werror \
	-I$(top_srcdir)/libnimf \
	-DNIMF_COMPILATION \
	-DG_LOG_DOMAIN=\"nimf\" \
	$(NIMF_INDICATOR_DEPS_CFLAGS)

nimf_indicator_LDADD   = $(top_builddir)/libnimf/libnimf.la
nimf_indicator_LDFLAGS = $(NIMF_INDICATOR_DEPS_LIBS)

DISTCLEANFILES = Makefile.in
