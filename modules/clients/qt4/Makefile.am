gsettings_SCHEMAS = org.nimf.clients.qt4.gschema.xml
@GSETTINGS_RULES@

QT4_LIB_DIR = `pkg-config --variable=libdir       QtCore`
QT4_MOC     = `pkg-config --variable=moc_location QtCore`

qt4_im_moduledir = $(QT4_LIB_DIR)/qt4/plugins/inputmethods
qt4_im_module_LTLIBRARIES = libqt4im-nimf.la

BUILT_SOURCES = im-nimf-qt4.moc
libqt4im_nimf_la_SOURCES = im-nimf-qt4.cpp $(BUILT_SOURCES)

libqt4im_nimf_la_CXXFLAGS = \
	$(EXTRA_CFLAGS) \
	$(LIBNIMF_DEPS_CFLAGS) \
	-I$(top_srcdir)/libnimf \
	-DG_LOG_DOMAIN=\"nimf\" \
	$(IM_NIMF_QT4_DEPS_CFLAGS) \
	-DQT_NO_KEYWORDS

libqt4im_nimf_la_LDFLAGS = -avoid-version -module $(IM_NIMF_QT4_DEPS_LIBS) \
	                         $(LIBNIMF_DEPS_LIBS)
libqt4im_nimf_la_LIBADD  = $(top_builddir)/libnimf/libnimf.la

im-nimf-qt4.moc: im-nimf-qt4.cpp
	$(AM_V_GEN) $(QT4_MOC) im-nimf-qt4.cpp -o im-nimf-qt4.moc

install-data-hook:
	chmod -x $(DESTDIR)$(qt4_im_moduledir)/libqt4im-nimf.so
	rm -f    $(DESTDIR)$(qt4_im_moduledir)/libqt4im-nimf.la

uninstall-hook:
	rm -f    $(DESTDIR)$(qt4_im_moduledir)/libqt4im-nimf.so

DISTCLEANFILES = Makefile.in $(BUILT_SOURCES)
