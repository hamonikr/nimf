gsettings_SCHEMAS = org.nimf.clients.qt5.gschema.xml
@GSETTINGS_RULES@

qt5_im_moduledir = $(QT5_IM_MODULE_DIR)
qt5_im_module_LTLIBRARIES = libqt5im-nimf.la

BUILT_SOURCES = im-nimf-qt5.moc
libqt5im_nimf_la_SOURCES = im-nimf-qt5.cpp $(BUILT_SOURCES)

libqt5im_nimf_la_CXXFLAGS = \
	$(EXTRA_CFLAGS) \
	$(LIBNIMF_DEPS_CFLAGS) \
	-I$(top_srcdir)/libnimf \
	-DG_LOG_DOMAIN=\"nimf\" \
	-DNIMF_COMPILATION \
	$(IM_NIMF_QT5_DEPS_CFLAGS) \
	-I$(QT5_CORE_PRIVATE_INCLUDE_PATH) \
	-I$(QT5_GUI_PRIVATE_INCLUDE_PATH) \
	-DQT_NO_KEYWORDS \
	-fPIC \
	-std=c++11

libqt5im_nimf_la_LDFLAGS = -avoid-version -module \
	                         $(IM_NIMF_QT5_DEPS_LIBS) \
	                         $(LIBNIMF_DEPS_LIBS)
libqt5im_nimf_la_LIBADD  = $(top_builddir)/libnimf/libnimf.la

im-nimf-qt5.moc: im-nimf-qt5.cpp
	$(AM_V_GEN) $(QT5_MOC) -I $(QT5_CORE_PRIVATE_INCLUDE_PATH) \
	                       -I $(QT5_GUI_PRIVATE_INCLUDE_PATH) im-nimf-qt5.cpp \
	                       -o im-nimf-qt5.moc

install-data-hook:
	chmod -x $(DESTDIR)$(qt5_im_moduledir)/libqt5im-nimf.so
	rm -f    $(DESTDIR)$(qt5_im_moduledir)/libqt5im-nimf.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(qt5_im_moduledir)/libqt5im-nimf.so
	-rmdir -p $(DESTDIR)$(qt5_im_moduledir)

DISTCLEANFILES = Makefile.in $(BUILT_SOURCES)
