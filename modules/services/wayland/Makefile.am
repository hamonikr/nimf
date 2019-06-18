moduledir = $(libdir)/nimf/modules/services
module_LTLIBRARIES = libnimf-wayland.la

WAYLAND_IM_XML_PATH = `pkg-config --variable=pkgdatadir wayland-protocols`/unstable/input-method/input-method-unstable-v1.xml

BUILT_SOURCES = \
	input-method-unstable-v1-client-protocol.h \
	input-method-unstable-v1-protocol.c \
	$(NULL)

libnimf_wayland_la_SOURCES = \
	nimf-wayland.c \
	nimf-wayland-ic.c \
	nimf-wayland-ic.h \
	$(BUILT_SOURCES) \
	$(NULL)

libnimf_wayland_la_CFLAGS = \
	$(EXTRA_CFLAGS) \
	-I$(top_srcdir)/libnimf \
	-DG_LOG_DOMAIN=\"nimf\" \
	$(NIMF_WAYLAND_DEPS_CFLAGS)

libnimf_wayland_la_LDFLAGS = -avoid-version -module $(NIMF_WAYLAND_DEPS_LIBS)
libnimf_wayland_la_LIBADD  = $(top_builddir)/libnimf/libnimf.la

input-method-unstable-v1-client-protocol.h:
	$(AM_V_GEN) wayland-scanner client-header < $(WAYLAND_IM_XML_PATH) \
	                            > input-method-unstable-v1-client-protocol.h

input-method-unstable-v1-protocol.c:
	$(AM_V_GEN) wayland-scanner code < $(WAYLAND_IM_XML_PATH) \
	                            > input-method-unstable-v1-protocol.c

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/libnimf-wayland.so
	rm    -f $(DESTDIR)$(moduledir)/libnimf-wayland.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(moduledir)/libnimf-wayland.so
	-rmdir -p $(DESTDIR)$(moduledir)

CLEANFILES = $(BUILT_SOURCES)

DISTCLEANFILES = Makefile.in
