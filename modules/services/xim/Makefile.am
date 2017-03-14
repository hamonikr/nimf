moduledir = $(libdir)/nimf/modules/services
module_LTLIBRARIES = libnimf-xim.la

libnimf_xim_la_SOURCES = \
	nimf-xim.c \
	$(NULL)

libnimf_xim_la_CFLAGS = \
	-Wall -Werror \
	-I$(top_srcdir)/libnimf \
	-DG_LOG_DOMAIN=\"nimf\" \
	$(NIMF_XIM_DEPS_CFLAGS)

libnimf_xim_la_LDFLAGS = -avoid-version -module $(NIMF_XIM_DEPS_LIBS)
libnimf_xim_la_LIBADD  = $(top_builddir)/libnimf/libnimf.la

DISTCLEANFILES = Makefile.in

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/libnimf-xim.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-xim.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-xim.so
	-rmdir -p $(DESTDIR)$(moduledir)
