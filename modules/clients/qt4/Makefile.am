QT4_LIB_DIR = `pkg-config --variable=libdir       QtCore`
QT4_MOC     = `pkg-config --variable=moc_location QtCore`

qt4_im_moduledir = $(QT4_LIB_DIR)/qt4/plugins/inputmethods
qt4_im_module_LTLIBRARIES = libqt4im-dasom.la

BUILT_SOURCES = im-dasom-qt4.moc
libqt4im_dasom_la_SOURCES = im-dasom-qt4.cpp $(BUILT_SOURCES)

libqt4im_dasom_la_CXXFLAGS = \
	-Wall \
	-Werror \
	$(LIBDASOM_DEPS_CFLAGS) \
	-I$(top_srcdir)/libdasom \
	-DG_LOG_DOMAIN=\"dasom\" \
	-DDASOM_COMPILATION \
	$(QT4_GUI_CFLAGS) \
	-DQT_NO_KEYWORDS

libqt4im_dasom_la_LDFLAGS = -avoid-version -module $(QT4_GUI_LIBS)
libqt4im_dasom_la_LIBADD  = $(top_builddir)/libdasom/libdasom.la

im-dasom-qt4.moc: im-dasom-qt4.cpp
	$(AM_V_GEN) $(QT4_MOC) im-dasom-qt4.cpp -o im-dasom-qt4.moc

install-data-hook:
	chmod -x $(DESTDIR)$(qt4_im_moduledir)/libqt4im-dasom.so
	rm       $(DESTDIR)$(qt4_im_moduledir)/libqt4im-dasom.la

uninstall-hook:
	rm -f    $(DESTDIR)$(qt4_im_moduledir)/libqt4im-dasom.so

DISTCLEANFILES = Makefile.in $(BUILT_SOURCES)
