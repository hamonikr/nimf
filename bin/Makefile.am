bin_PROGRAMS = nimf nimf-settings

man_MANS = nimf.1 nimf-settings.1

AM_CFLAGS = \
	$(EXTRA_CFLAGS) \
	-I$(top_srcdir)/libnimf \
	-I$(top_srcdir)/libnimf/client \
	-I$(top_srcdir)/libnimf/server \
	-DNIMF_COMPILATION \
	-DG_LOG_DOMAIN=\"nimf\"

nimf_SOURCES = nimf.c
nimf_CFLAGS = \
	$(LIBNIMF_DEPS_CFLAGS) \
	$(NIMF_DEPS_CFLAGS) \
	-DNIMF_MODULE_DIR=\"$(libdir)/nimf/modules\" \
	-DNIMF_SERVICE_MODULE_DIR=\"$(libdir)/nimf/modules/services\" \
	$(AM_CFLAGS)

nimf_LDFLAGS = $(NIMF_DEPS_LIBS)
nimf_LDADD   = $(top_builddir)/libnimf/libnimf.la

nimf_settings_SOURCES = nimf-settings.c
nimf_settings_CFLAGS = \
	$(NIMF_SETTINGS_DEPS_CFLAGS) \
	$(AM_CFLAGS)

nimf_settings_LDFLAGS = $(NIMF_SETTINGS_DEPS_LIBS)
nimf_settings_LDADD   = $(top_builddir)/libnimf/libnimf.la

DISTCLEANFILES = Makefile.in
