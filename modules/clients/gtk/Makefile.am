gsettings_SCHEMAS = org.nimf.clients.gtk.gschema.xml
@GSETTINGS_RULES@

AM_CFLAGS = \
	$(EXTRA_CFLAGS) \
	-I$(top_srcdir)/libnimf \
	-DG_LOG_DOMAIN=\"nimf\"

# for gtk3

gtk3_im_moduledir = $(GTK3_LIBDIR)/gtk-3.0/$(GTK3_BINARY_VERSION)/immodules
gtk3_im_module_LTLIBRARIES = im-nimf-gtk3.la

im_nimf_gtk3_la_SOURCES  = im-nimf.c
im_nimf_gtk3_la_CFLAGS   = $(IM_NIMF_GTK3_DEPS_CFLAGS) $(AM_CFLAGS)
im_nimf_gtk3_la_LDFLAGS  = $(IM_NIMF_GTK3_DEPS_LIBS) -avoid-version -module
im_nimf_gtk3_la_LIBADD   = $(top_builddir)/libnimf/libnimf.la

# for gtk4

if HAVE_GTK4
gtk4_im_moduledir = $(GTK4_LIBDIR)/gtk-4.0/$(GTK4_BINARY_VERSION)/immodules
gtk4_im_module_LTLIBRARIES = libim-nimf-gtk4.la

libim_nimf_gtk4_la_SOURCES  = im-nimf-gtk4.c
libim_nimf_gtk4_la_CFLAGS   = $(IM_NIMF_GTK4_DEPS_CFLAGS) $(AM_CFLAGS)
libim_nimf_gtk4_la_LDFLAGS  = $(IM_NIMF_GTK4_DEPS_LIBS) -avoid-version -module
libim_nimf_gtk4_la_LIBADD   = $(top_builddir)/libnimf/libnimf.la
endif

# for gtk2

if HAVE_GTK2
gtk2_im_moduledir = $(GTK2_LIBDIR)/gtk-2.0/$(GTK2_BINARY_VERSION)/immodules
gtk2_im_module_LTLIBRARIES = im-nimf-gtk2.la

im_nimf_gtk2_la_SOURCES  = im-nimf.c
im_nimf_gtk2_la_CFLAGS   = $(IM_NIMF_GTK2_DEPS_CFLAGS) $(AM_CFLAGS) \
                           -Wno-error=deprecated-declarations
im_nimf_gtk2_la_LDFLAGS  = $(IM_NIMF_GTK2_DEPS_LIBS) -avoid-version -module
im_nimf_gtk2_la_LIBADD   = $(top_builddir)/libnimf/libnimf.la
endif

DISTCLEANFILES = Makefile.in

install-data-hook:
	chmod -x $(DESTDIR)$(gtk3_im_moduledir)/im-nimf-gtk3.so
	rm -f $(DESTDIR)$(gtk3_im_moduledir)/im-nimf-gtk3.la
if HAVE_GTK2
	chmod -x $(DESTDIR)$(gtk2_im_moduledir)/im-nimf-gtk2.so
	rm -f $(DESTDIR)$(gtk2_im_moduledir)/im-nimf-gtk2.la
endif
if HAVE_GTK4
	chmod -x $(DESTDIR)$(gtk4_im_moduledir)/libim-nimf-gtk4.so
	rm -f $(DESTDIR)$(gtk4_im_moduledir)/libim-nimf-gtk4.la
	if test "x$(GTK_QUERY_IMMODULES4)" != "xno"; then \
		$(GTK_QUERY_IMMODULES4) --update-cache; \
	fi
endif

uninstall-hook:
	rm -f $(DESTDIR)$(gtk3_im_moduledir)/im-nimf-gtk3.so
if HAVE_GTK2
	rm -f $(DESTDIR)$(gtk2_im_moduledir)/im-nimf-gtk2.so
endif
if HAVE_GTK4
	rm -f $(DESTDIR)$(gtk4_im_moduledir)/libim-nimf-gtk4.so
endif
	$(GTK_QUERY_IMMODULES2) --update-cache
	$(GTK_QUERY_IMMODULES3) --update-cache
if HAVE_GTK4
	if test "x$(GTK_QUERY_IMMODULES4)" != "xno"; then \
		$(GTK_QUERY_IMMODULES4) --update-cache; \
	fi
endif
