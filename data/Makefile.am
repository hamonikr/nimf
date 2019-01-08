SUBDIRS = icons apparmor-abstractions

if WITH_IM_CONFIG_DATA
  SUBDIRS += im-config
endif

if WITH_IMSETTINGS_DATA
  SUBDIRS += imsettings
endif

desktopdir = $(datadir)/applications
desktop_in_files = nimf-settings.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

DISTCLEANFILES = Makefile.in nimf-settings.desktop
