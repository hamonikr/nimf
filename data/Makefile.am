SUBDIRS = icons apparmor-abstractions

if WITH_IM_CONFIG_DATA
  SUBDIRS += im-config
endif

if WITH_IMSETTINGS_DATA
  SUBDIRS += imsettings
endif

configdir = /etc/input.d

config_DATA = nimf.conf

DISTCLEANFILES = Makefile.in
