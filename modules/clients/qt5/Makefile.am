QT5_LIB_DIR          = `pkg-config --variable=libdir     Qt5PlatformSupport`
QT5_GUI_VERSION      = `pkg-config --modversion          Qt5Gui`
QT5_GUI_INCLUDE_DIR  = `pkg-config --variable=includedir Qt5Gui`
QT5_GUI_INCLUDE_PATH = $(QT5_GUI_INCLUDE_DIR)/QtGui/$(QT5_GUI_VERSION)/QtGui
QT5_MOC              = `pkg-config --variable=host_bins Qt5Core`/moc

qt5_im_moduledir = $(QT5_LIB_DIR)/qt5/plugins/platforminputcontexts
qt5_im_module_LTLIBRARIES = libqt5im-dasom.la

BUILT_SOURCES = im-dasom-qt5.moc
libqt5im_dasom_la_SOURCES = im-dasom-qt5.cpp $(BUILT_SOURCES)

libqt5im_dasom_la_CXXFLAGS = \
	-Wall \
	-Werror \
	$(LIBDASOM_DEPS_CFLAGS) \
	-I$(top_srcdir)/libdasom \
	-DG_LOG_DOMAIN=\"dasom\" \
	-DDASOM_COMPILATION \
	$(QT5_PLATFORM_SUPPORT_CFLAGS) \
	-I$(QT5_GUI_INCLUDE_PATH) \
	-DQT_NO_KEYWORDS

libqt5im_dasom_la_LDFLAGS = -avoid-version -module \
	                          $(QT5_WIDGETS_LIBS) \
	                          $(QT5_PLATFORM_SUPPORT_LIBS) \
	                          $(GLIB_LIBS) $(GOBJECT_LIBS)
libqt5im_dasom_la_LIBADD  = $(top_builddir)/libdasom/libdasom.la

im-dasom-qt5.moc: im-dasom-qt5.cpp
	$(AM_V_GEN) $(QT5_MOC) im-dasom-qt5.cpp -o im-dasom-qt5.moc

install-data-hook:
	chmod -x $(DESTDIR)$(qt5_im_moduledir)/libqt5im-dasom.so
	rm -f    $(DESTDIR)$(qt5_im_moduledir)/libqt5im-dasom.la

uninstall-hook:
	rm -f    $(DESTDIR)$(qt5_im_moduledir)/libqt5im-dasom.so

DISTCLEANFILES = Makefile.in $(BUILT_SOURCES)
