QT5_GUI_VERSION      = `pkg-config --modversion          Qt5Gui`
QT5_GUI_INCLUDE_DIR  = `pkg-config --variable=includedir Qt5Gui`
QT5_GUI_INCLUDE_PATH = $(QT5_GUI_INCLUDE_DIR)/QtGui/$(QT5_GUI_VERSION)/QtGui

qt5_im_moduledir = $(QT5_IM_MODULE_DIR)
qt5_im_module_LTLIBRARIES = libqt5im-nimf.la

BUILT_SOURCES = im-nimf-qt5.moc
libqt5im_nimf_la_SOURCES = im-nimf-qt5.cpp $(BUILT_SOURCES)

libqt5im_nimf_la_CXXFLAGS = \
	-Wall \
	-Werror \
	$(LIBNIMF_DEPS_CFLAGS) \
	-I$(top_srcdir)/libnimf \
	-DG_LOG_DOMAIN=\"nimf\" \
	$(QT5_PLATFORM_SUPPORT_CFLAGS) \
	-I$(QT5_GUI_INCLUDE_PATH) \
	-DQT_NO_KEYWORDS

libqt5im_nimf_la_LDFLAGS = -avoid-version -module \
	                         $(QT5_WIDGETS_LIBS) \
	                         $(QT5_PLATFORM_SUPPORT_LIBS) \
	                         $(LIBNIMF_DEPS_LIBS)
libqt5im_nimf_la_LIBADD  = $(top_builddir)/libnimf/libnimf.la

im-nimf-qt5.moc: im-nimf-qt5.cpp
	$(AM_V_GEN) $(QT5_MOC) -I $(QT5_GUI_INCLUDE_PATH) im-nimf-qt5.cpp \
	                       -o im-nimf-qt5.moc

install-data-hook:
	chmod -x $(DESTDIR)$(qt5_im_moduledir)/libqt5im-nimf.so
	rm -f    $(DESTDIR)$(qt5_im_moduledir)/libqt5im-nimf.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(qt5_im_moduledir)/libqt5im-nimf.so
	-rmdir -p $(DESTDIR)$(qt5_im_moduledir)

DISTCLEANFILES = Makefile.in $(BUILT_SOURCES)
