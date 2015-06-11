AM_CPPFLAGS = \
	-I$(top_srcdir)/libdasom \
	-DG_LOG_DOMAIN=\"dasom\" \
	$(GLIB_CFLAGS)

AM_CFLAGS = -Wall -Werror

# for gtk3

GTK3_LIBDIR = `pkg-config --variable=libdir gtk+-3.0`
GTK3_BINARY_VERSION = `pkg-config --variable=gtk_binary_version gtk+-3.0`

gtk3_im_moduledir = $(GTK3_LIBDIR)/gtk-3.0/$(GTK3_BINARY_VERSION)/immodules
gtk3_im_module_LTLIBRARIES = im-dasom-gtk3.la

im_dasom_gtk3_la_SOURCES  = im-dasom.c
im_dasom_gtk3_la_CFLAGS   = $(AM_CFLAGS) $(GTK3_CFLAGS)
im_dasom_gtk3_la_LDFLAGS  = $(GTK3_LIBS) -avoid-version -module
im_dasom_gtk3_la_LIBADD   = $(top_builddir)/libdasom/libdasom.la

# for gtk2

GTK2_LIBDIR = `pkg-config --variable=libdir gtk+-2.0`
GTK2_BINARY_VERSION = `pkg-config --variable=gtk_binary_version gtk+-2.0`

gtk2_im_moduledir = $(GTK2_LIBDIR)/gtk-2.0/$(GTK2_BINARY_VERSION)/immodules
gtk2_im_module_LTLIBRARIES = im-dasom-gtk2.la

im_dasom_gtk2_la_SOURCES  = im-dasom.c
im_dasom_gtk2_la_CFLAGS   = $(AM_CFLAGS) $(GTK2_CFLAGS)
im_dasom_gtk2_la_LDFLAGS  = $(GTK2_LIBS) -avoid-version -module
im_dasom_gtk2_la_LIBADD   = $(top_builddir)/libdasom/libdasom.la

install-data-hook:
	chmod -x $(DESTDIR)$(gtk3_im_moduledir)/im-dasom-gtk3.so
	chmod -x $(DESTDIR)$(gtk2_im_moduledir)/im-dasom-gtk2.so
	rm $(DESTDIR)$(gtk3_im_moduledir)/im-dasom-gtk3.la
	rm $(DESTDIR)$(gtk2_im_moduledir)/im-dasom-gtk2.la

uninstall-hook:
	rm -f $(DESTDIR)$(gtk3_im_moduledir)/im-dasom-gtk3.so
	rm -f $(DESTDIR)$(gtk2_im_moduledir)/im-dasom-gtk2.so
