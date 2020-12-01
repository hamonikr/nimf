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

# for gtk2

gtk2_im_moduledir = $(GTK2_LIBDIR)/gtk-2.0/$(GTK2_BINARY_VERSION)/immodules
gtk2_im_module_LTLIBRARIES = im-nimf-gtk2.la

im_nimf_gtk2_la_SOURCES  = im-nimf.c
im_nimf_gtk2_la_CFLAGS   = $(IM_NIMF_GTK2_DEPS_CFLAGS) $(AM_CFLAGS) \
                           -Wno-error=deprecated-declarations
im_nimf_gtk2_la_LDFLAGS  = $(IM_NIMF_GTK2_DEPS_LIBS) -avoid-version -module
im_nimf_gtk2_la_LIBADD   = $(top_builddir)/libnimf/libnimf.la

DISTCLEANFILES = Makefile.in

install-data-hook:
	chmod -x $(DESTDIR)$(gtk3_im_moduledir)/im-nimf-gtk3.so
	chmod -x $(DESTDIR)$(gtk2_im_moduledir)/im-nimf-gtk2.so
	rm -f $(DESTDIR)$(gtk3_im_moduledir)/im-nimf-gtk3.la
	rm -f $(DESTDIR)$(gtk2_im_moduledir)/im-nimf-gtk2.la

uninstall-hook:
	rm -f $(DESTDIR)$(gtk3_im_moduledir)/im-nimf-gtk3.so
	rm -f $(DESTDIR)$(gtk2_im_moduledir)/im-nimf-gtk2.so
	$(GTK_QUERY_IMMODULES2) --update-cache
	$(GTK_QUERY_IMMODULES3) --update-cache
