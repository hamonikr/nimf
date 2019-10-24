bin_PROGRAMS = nimf-settings

man_MANS = nimf-settings.1

applicationsdir   = $(datadir)/applications
applications_DATA = nimf-settings.desktop

autostartdir   = /etc/xdg/autostart
autostart_DATA = nimf-settings-autostart.desktop

@INTLTOOL_DESKTOP_RULE@

gsettings_SCHEMAS = org.nimf.settings.gschema.xml
@GSETTINGS_RULES@

AM_CFLAGS = \
	$(EXTRA_CFLAGS) \
	-I$(top_srcdir)/libnimf \
	-DNIMF_COMPILATION \
	-DNIMF_MODULE_DIR=\"$(libdir)/nimf/modules\" \
	-DG_LOG_DOMAIN=\"nimf\"

nimf_settings_SOURCES = nimf-settings.c
nimf_settings_CFLAGS = \
	$(NIMF_SETTINGS_DEPS_CFLAGS) \
	$(AM_CFLAGS)

nimf_settings_LDFLAGS = $(NIMF_SETTINGS_DEPS_LIBS)
nimf_settings_LDADD   = $(top_builddir)/libnimf/libnimf.la

DISTCLEANFILES = \
		Makefile.in \
		nimf-settings.desktop \
		nimf-settings-autostart.desktop
