moduledir = $(libdir)/nimf/modules
module_LTLIBRARIES = nimf-system-keyboard.la

nimf_system_keyboard_la_SOURCES = nimf-system-keyboard.c
nimf_system_keyboard_la_CFLAGS  = \
	-Wall -Werror \
	-I$(top_srcdir)/libnimf \
	-DG_LOG_DOMAIN=\"nimf\" \
	$(NIMF_SYSTEM_KEYBOARD_DEPS_CFLAGS)

nimf_system_keyboard_la_LDFLAGS = -avoid-version -module \
	                                $(NIMF_SYSTEM_KEYBOARD_DEPS_LIBS)
nimf_system_keyboard_la_LIBADD  = $(top_builddir)/libnimf/libnimf.la

DISTCLEANFILES = Makefile.in

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/nimf-system-keyboard.so
	rm    -f $(DESTDIR)$(moduledir)/nimf-system-keyboard.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(moduledir)/nimf-system-keyboard.so
	-rmdir -p $(DESTDIR)$(moduledir)
