moduledir = $(libdir)/nimf/modules/services
module_LTLIBRARIES = libnimf-preedit-window.la

libnimf_preedit_window_la_SOURCES = \
	nimf-preedit-window.c \
	$(NULL)

libnimf_preedit_window_la_CFLAGS = \
	$(EXTRA_CFLAGS) \
	-I$(top_srcdir)/libnimf \
	-DG_LOG_DOMAIN=\"nimf\" \
	$(NIMF_PREEDIT_WINDOW_DEPS_CFLAGS)

libnimf_preedit_window_la_LDFLAGS = -avoid-version -module $(NIMF_PREEDIT_WINDOW_DEPS_LIBS)
libnimf_preedit_window_la_LIBADD  = $(top_builddir)/libnimf/libnimf.la

DISTCLEANFILES = Makefile.in

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/libnimf-preedit-window.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-preedit-window.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-preedit-window.so
	-rmdir -p $(DESTDIR)$(moduledir)
