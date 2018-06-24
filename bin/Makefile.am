bin_PROGRAMS = nimf nimf-settings

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

nimf_settings_SOURCES = nimf-settings.c
nimf_settings_CFLAGS = \
	-Wall -Werror \
	-DG_LOG_DOMAIN=\"nimf\" \
	-DNIMF_MODULE_DIR=\"$(libdir)/nimf/modules\" \
	-I$(top_srcdir)/libnimf \
	$(NIMF_SETTINGS_DEPS_CFLAGS)

nimf_settings_LDFLAGS = $(NIMF_SETTINGS_DEPS_LIBS)
nimf_settings_LDADD   = $(top_builddir)/libnimf/libnimf.la

DISTCLEANFILES = Makefile.in
